#ifndef GSTATUS_LIB_H
#define GSTATUS_LIB_H

#include <sys/stat.h>

#include <string>
#include <vector>

namespace gstat {

class GBranch {
 public:
    GBranch(): branch(""), upstream(".."), local(0) {}

    friend std::ostream & operator<<(std::iostream& os, const GBranch &info);

    std::string branch;
    std::string upstream;
    std::uint_fast8_t local;
};

class GRemote {
 public:
    GRemote(): ahead(0), behind(0) {}

    friend std::ostream & operator<<(std::iostream& os, const GRemote &info);

    std::uint_fast32_t ahead;
    std::uint_fast32_t behind;
};

class GStats {
 public:
    GStats(): changed(0), conflicts(0), staged(0), untracked(0) {}

    friend std::ostream & operator<<(std::iostream& os, const GRemote &info);

    std::uint_fast32_t changed;
    std::uint_fast32_t conflicts;
    std::uint_fast32_t staged;
    std::uint_fast32_t untracked;
};

class GPaths {
 public:
    explicit GPaths(const std::string &_git_root): git_root(_git_root), tree_d("") {
        this->set_tree_d();
    }
    std::string set_tree_d();

    std::string head();
    std::string merge();
    std::string rebase();
    std::string stash();

    std::string git_root;
    std::string tree_d;
};

GBranch parse_branch(const std::string &branch_line, const std::string &head_file);
GRemote parse_remote(const std::string &branch_line);
GStats parse_stats(const std::vector<std::string> &lines);
std::string current_gitstatus(const std::vector<std::string> &lines);
std::string stash_count(const std::string &stash_file);
std::string rebase_progress(const std::string &rebase_d);
std::string run_cmd(const char *cmd);
std::string get_cwd();
std::string find_git_root();
bool stdin_has_input();

/**
 * Returns: bool - Does that file exist?
 */
inline bool file_exists(const std::string &path) {
    struct stat buffer;

    return (stat(path.c_str(), &buffer) == 0);
}

/**
 * Returns: bool - Is the file a directory? (implies exists)
 */
inline bool file_is_dir(const std::string &path) {
    struct stat buffer;

    return (stat(path.c_str(), &buffer) == 0) && S_ISDIR(buffer.st_mode);
}

/**
 * Simple copy of os.path.join
 *
 * Returns: std::string - The new joined path
 */
inline std::string join(const std::string &path, const std::string &leaf) {
    return path + "/" + leaf;
}

/**
 * Simple copy of os.path.basename
 *
 * Returns: std::string - The last leaf of the path
 */
inline std::string basename(const std::string &path) {
    return path.substr(path.rfind('/') + 1);
}

/**
 * Simple copy of os.path.dirname
 *
 * Returns: std::string - The new path without last leaf
 */
inline std::string dirname(const std::string &path) {
    return path.substr(0, path.rfind('/'));
}

/**
 * Simple hash, both characters are important.
 */
inline std::uint_fast32_t hash_two_places(const std::string &word) {
    return static_cast<std::uint_fast32_t>(word.at(0)) * 1000 + static_cast<std::uint_fast32_t>(word.at(1));
}

}  // namespace gstat

#endif  // GSTATUS_LIB_H
