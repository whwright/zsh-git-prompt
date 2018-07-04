/**
 * Generate a simple header with predefined hashes for later case.
 */
#include <sys/stat.h>

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <fstream>
#include <initializer_list>
#include <iostream>
#include <string>

#include "src/lib.h"

/**
 * Apply standard conversions to make suitable guard name.
 *
 * Returns: std::string - The guard name
 */
std::string guard_name(const std::string &fname) {
    std::string guard = std::string(fname);
    for (std::string::iterator itr = guard.begin(); itr != guard.end(); ++itr) {
        *itr = ::toupper(*itr);
        if (std::isalnum(*itr) == 0) {
            *itr = '_';
        }
    }

    uint_fast16_t cnt = 0;
    while (guard.length() != 0 && guard.at(cnt) == '_') {
        cnt++;
    }

    return guard.substr(cnt);
}

/**
 * Just a simple mkdir clone, recurse until exists.
 *
 * Raises: std::runtime_error - Unable to complete mkdir
 */
void mkdir_recurse(const std::string &path) {
    if (path.length() == 0 || path == ROOT_DIR || gstat::file_is_dir(path)) {
        return;
    }

    mkdir_recurse(gstat::dirname(path));

    uint_fast8_t err = mkdir(path.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    if (err != 0) {
        throw std::runtime_error("Unable to create dir: " + path);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " filename" << std::endl;
        return 1;
    }

    std::string fname(argv[1]);
    mkdir_recurse(gstat::dirname(fname));

    std::ofstream fout(fname.c_str());
    if (!fout.good()) {
        std::cout << "CRITIAL ERROR: Cannot open file!" << std::endl;
        return 1;
    }

    std::string guard = guard_name(fname);
    fout << "#ifndef " << guard << std::endl
        << "#define " << guard << std::endl <<std::endl;

    std::initializer_list<std::string> vals({ "AA", "AU", "DD", "DU", "UA", "UD", "UU" });
    for (std::initializer_list<std::string>::const_iterator itr = vals.begin(); itr != vals.end(); ++itr) {
        fout << "#define HASH_CASE_" << *itr << " "
            << gstat::hash_two_places(*itr) << std::endl;
    }

    fout << std::endl << "#endif  // " << guard;
    fout.close();

    return 0;
}
