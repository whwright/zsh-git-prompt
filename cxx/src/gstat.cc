/**
 *     The MIT License
 *
 *     Copyright (c) 2018 Jeremy Pallats/starcraft.man
 *
 */
#include "src/gstat.h"

// Linux only
#include <sys/poll.h>
#include <sys/stat.h>
#include <unistd.h>

#include <array>
#include <cassert>
#include <cctype>
#include <climits>
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "src/const.h"

namespace gstat {

std::ostream & operator<<(std::ostream& os, const GBranch &info) {
    os  << info.branch << " "
        << info.upstream << " "
        << info.local << " ";

    return os;
}

std::ostream & operator<<(std::ostream& os, const GRemote &info) {
    os  << info.ahead << " "
        << info.behind << " ";

    return os;
}

std::ostream & operator<<(std::ostream& os, const GStats &info) {
    os  << info.staged << " "
        << info.conflicts << " "
        << info.changed << " "
        << info.untracked << " ";

    return os;
}

/**
 * This sets the tree_d when inside a git worktree.
 * In this case, the .git folder is instead a file with the path to the
 * worktree folder in the original repository.
 * Relative this new folder, scan up until we hit the root.
 *
 * Returns: std::string - The path to the tree directory.
 */
void GPaths::set_tree_d() {
    // Format of the file:
    //      gitdir: /tmp/g/.git/worktrees/wg
    std::ifstream fin(this->tree_d.c_str());
    if (!fin.good()) {
        throw std::runtime_error("Could not open wotkree file: " + this->tree_d);
    }
    fin >> this->tree_d >> this->tree_d;
    fin.close();

    this->git_root = this->tree_d;
    while (basename(this->git_root) != ".git") {
        this->git_root = dirname(this->git_root);
    }
}

std::string GPaths::head() {
    return join({this->tree_d, "HEAD"});
}

std::string GPaths::merge() {
    return join({this->tree_d, "MERGE_HEAD"});
}

std::string GPaths::rebase() {
    return join({this->tree_d, "rebase-apply"});
}

std::string GPaths::stash() {
    return join({this->git_root, "logs", "refs", "stash"});
}

/**
 * Does the current STDIN file have __ANY__ input.
 *
 * Returns: Bool
 */
bool stdin_has_input() {
    struct pollfd fds;
    fds.fd = 0;
    fds.events = POLLIN;

    return poll(&fds, 1, 0) == 1;
}

/**
 * Run a command on the system in the current directory.
 *
 * Return: std:string of output captured
 *
 * Raises: runtime_error : Popen failed, returned NULL.
 */
std::string run_cmd(const char *cmd) {
    FILE *pfile = popen(cmd, "r");
    if (pfile == nullptr) {
        throw std::runtime_error("Could not execute cmd: " + std::string(cmd));
    }

    std::array<char, 1000> buffer;
    std::string text;
    while (fgets(buffer.data(), 1000, pfile) != nullptr) {
        text.append(buffer.data());
    }
    pclose(pfile);

    return text;
}

/**
 * Get the current working directory.
 *
 * Returns: A string of the path.
 *
 * Raises: runtime_error: Failed to get CWD.
 */
std::string get_cwd() {
    std::array<char, PATH_MAX> buffer;

    if ((getcwd(buffer.data(), PATH_MAX)) == nullptr) {
        throw std::runtime_error("Unable to get CWD");
    }

    return std::string(buffer.data());
}

/**
 * Move upward from the current directory until a directory
 * with `.git` is found.
 *
 * Returns: std::string - The path with git.
 */
std::string find_git_root() {
    std::string cwd = get_cwd();
    std::string git_leaf = ".git";
    std::string git_root = join({cwd, git_leaf});

    while (cwd != ROOT_DIR) {
        if (file_exists(git_root)) {
            return git_root;
        }

        cwd = dirname(cwd);
        git_root = join({cwd, git_leaf});
    }

    throw std::runtime_error("Could not find a git directory!");
}

/**
 * Parse the branch of a string line.
 *
 * Returns: GBranch structure.
 */
GBranch parse_branch(const std::string &branch_line,
                     const std::string &head_file) {
    GBranch result;
    result.local = 1;

    std::string temp = branch_line.substr(3);
    std::size_t found = temp.rfind(" [");
    if (found != std::string::npos) {
        temp = temp.substr(0, found);
    }

    found = temp.find("...");
    if (found != std::string::npos) {
        result.branch = temp.substr(0, found);
        result.upstream = temp.substr(found + 3);
        result.local = 0;
    } else if (temp.find("(no branch)") != std::string::npos) {
        result.local = 0;
        std::ifstream fin(head_file.c_str());
        if (fin.good()) {
            fin >> result.branch;
        } else {
            throw std::runtime_error("Failed to get hash!");
        }
        fin.close();
    } else if (temp.find("Initial commit") != std::string::npos ||
               temp.find("No commits yet") != std::string::npos) {
        result.branch = temp.substr(temp.rfind(" ") + 1);
    } else {
        result.branch = temp;
    }

    return result;
}

/*
 * Parse the remote tracking portion of git status.
 *
 * Returns: GRemote structure.
 */
GRemote parse_remote(const std::string &branch_line) {
    GRemote remote;
    std::string temp = branch_line;

    std::size_t found = branch_line.find(" [");
    // Check for remote tracking (example: [ahead 2])
    if (found == std::string::npos ||
        branch_line.at(branch_line.length() - 1) != ']') {
        return remote;
    }

    // Only the remote tracking section remains
    temp = temp.substr(found + 2, temp.length() -1);

    if (temp.length() != 0 && temp.find("ahead") != std::string::npos) {
        temp = temp.replace(temp.begin(), temp.begin() + 6, "");
        std::string part;
        while (temp.length() != 0) {
            part += temp.at(0);
            temp = temp.substr(1);
            if (temp.length() == 0 || std::isdigit(temp.at(0)) == 0) {
                break;
            }
        }

        remote.ahead = std::stoi(part);
    }

    if (temp.length() != 0 && temp.at(0) == ',') {
        temp = temp.substr(1);
    }

    if (temp.length() != 0 && temp.find("behind") != std::string::npos) {
        temp = temp.replace(temp.begin(), temp.begin() + 7, "");
        remote.behind = std::stoi(temp);
    }

    return remote;
}

/**
 * Parses the status information from porcelain output.
 *
 * Returns: GStats structure.
 */
GStats parse_stats(const lines_iter_t &start, const lines_iter_t &end) {
    GStats stats;

    for (lines_iter_t itr = start; itr != end; ++itr) {
        if (itr->at(0) == '?') {
            stats.untracked++;
            continue;
        }

        switch (hash_two_places(*itr)) {
        case HASH_CASE_AA:
        case HASH_CASE_AU:
        case HASH_CASE_DD:
        case HASH_CASE_DU:
        case HASH_CASE_UA:
        case HASH_CASE_UD:
        case HASH_CASE_UU:
            stats.conflicts++;
            continue;
        }

        switch (itr->at(0)) {
        case 'A':
        case 'C':
        case 'D':
        case 'M':
        case 'R':
            stats.staged++;
        }

        switch (itr->at(1)) {
        case 'C':
        case 'D':
        case 'M':
        case 'R':
            stats.changed++;
        }
    }

    return stats;
}

/**
 * Returns: std:string - The # of stashes on repo
 */
std::string stash_count(const std::string &stash_file) {
    std::uint_fast32_t count = 0;

    std::ifstream fin(stash_file.c_str());
    while (fin.good()) {
        std::string buffer;
        std::getline(fin, buffer);
        if (buffer != "") {
            ++count;
        }
    }
    fin.close();

    return std::to_string(count);
}

/**
 * Returns:
 *  - "0": No active rebase
 *  - "1/4": Rebase in progress, commit 1 of 4
 */
std::string rebase_progress(const std::string &rebase_d) {
    std::string result = "0";
    std::string temp;

    std::ifstream next(join({rebase_d, "next"}).c_str());
    std::ifstream last(join({rebase_d, "last"}).c_str());
    if (next.good() && last.good()) {
        result.clear();
        last >> temp;
        next >> result;
        result += "/" + temp;
    }
    last.close();
    next.close();

    return result;
}

/**
 * Take input and produce the required output per specification.
 */
std::string current_gitstatus(const std::vector<std::string> &lines) {
    GPaths path(gstat::find_git_root());
    GBranch info = parse_branch(lines.front(), path.head());
    GRemote remote = parse_remote(lines.front());
    GStats stats = parse_stats(lines.begin() + 1, lines.end());
    std::string stashes = stash_count(path.stash());
    std::string merge = std::to_string(
                        static_cast<uint_fast8_t>(file_exists(path.merge())));
    std::string rebase = rebase_progress(path.rebase());

    std::ostringstream ss;
    ss << info.branch << " " << remote << stats << stashes << " "
        << std::to_string(info.local) << " " << info.upstream << " "
        << merge << " " << rebase;;

    return ss.str();
}

}  // namespace gstat
