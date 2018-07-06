/**
 *     The MIT License
 *
 *     Copyright (c) 2018 Jeremy Pallats/starcraft.man
 */
#ifndef CXX_SRC_GSTAT_H_
#define CXX_SRC_GSTAT_H_

#include <sys/stat.h>

#include <initializer_list>
#include <string>
#include <vector>

#define PATH_SEP "/"
#define ROOT_DIR "/"

namespace gstat {

typedef std::vector<std::string>::const_iterator lines_iter_t;

class GBranch {
 public:
    GBranch() noexcept: branch(""), upstream(".."), local(0) {}
    ~GBranch() noexcept {};  // Suppress warning

    friend std::ostream & operator<<(std::iostream& os, const GBranch &info);

    std::string branch;
    std::string upstream;
    std::uint_fast8_t local;
};

class GRemote {
 public:
    GRemote() noexcept: ahead(0), behind(0) {}

    friend std::ostream & operator<<(std::iostream& os, const GRemote &info);

    std::uint_fast32_t ahead;
    std::uint_fast32_t behind;
};

class GStats {
 public:
    GStats() noexcept: changed(0), conflicts(0), staged(0), untracked(0) {}

    friend std::ostream & operator<<(std::iostream& os, const GRemote &info);

    std::uint_fast32_t changed;
    std::uint_fast32_t conflicts;
    std::uint_fast32_t staged;
    std::uint_fast32_t untracked;
};

bool file_is_dir(const std::string &path);  // Inlined below
class GPaths {
 public:
    explicit GPaths(const std::string &_git_root): git_root(_git_root), tree_d(_git_root) {
        if (!file_is_dir(this->tree_d)) {
            this->set_tree_d();
        }
    }
    void set_tree_d();

    std::string head();
    std::string merge();
    std::string rebase();
    std::string stash();

    std::string git_root;
    std::string tree_d;
};

GBranch parse_branch(const std::string &branch_line,
                     const std::string &head_file);
GRemote parse_remote(const std::string &branch_line);
GStats parse_stats(const lines_iter_t &start, const lines_iter_t &end);
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
 * Simple copy of os.path.join, takes variable args as initializer_list
 * Condition: Must be called with at least 1 element.
 *
 * Returns: std::string - The new joined path
 */
inline std::string join(const std::initializer_list<std::string> &list) {
    std::initializer_list<std::string>::const_iterator itr = list.begin();
    std::string result(*itr);

    for (++itr; itr != list.end(); ++itr) {
        result += PATH_SEP + *itr;
    }

    return result;
}

/**
 * Simple copy of os.path.basename
 *
 * Returns: std::string - The last leaf of the path
 */
inline std::string basename(const std::string &path) {
    return path.substr(path.rfind(PATH_SEP) + 1);
}

/**
 * Simple copy of os.path.dirname
 *
 * Returns: std::string - The new path without last leaf
 */
inline std::string dirname(const std::string &path) {
    return path.substr(0, path.rfind(PATH_SEP));
}

/**
 * Simple hash, both characters are important.
 */
inline std::uint_fast32_t hash_two_places(const std::string &word) {
    return static_cast<std::uint_fast32_t>(word.at(0)) * 1000 +
           static_cast<std::uint_fast32_t>(word.at(1));
}

}  // namespace gstat

#endif  // CXX_SRC_GSTAT_H_
