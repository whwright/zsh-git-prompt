/**
 * Generate a simple header with predefined hashes for later case.
 */
#include <sys/stat.h>

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <list>
#include <string>
#include <vector>

#include "lib.h"

/**
 * Apply standard conversions to make suitable guard name.
 *
 * Returns: std::string - The guard name
 */
std::string guard_name(const std::string &fname) {
    std::string guard = std::string(fname);
    std::string::iterator itr = guard.begin();
    for (std::string::const_iterator i = guard.begin(); i != guard.end(); ++i) {
        *itr = ::toupper(*i);
        if (std::isalnum(*itr) == 0) {
            *itr = '_';
        }

        ++itr;
    }

    while (guard.length() != 0 && guard.at(0) == '_') {
        guard = guard.substr(1);
    }

    return guard;
}

/**
 * Just a simple mkdir clone, recurse until exists.
 *
 * Raises: std::runtime_error - Unable to complete mkdir
 */
void mkdir_recurse(const std::string &path) {
    if (path.length() == 0 || path == "/" || gstat::file_is_dir(path)) {
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
        std::cout << "Useage: " << argv[0] << " filename" << std::endl;
        return 1;
    }
    std::string fname(argv[1]);
    std::string guard = guard_name(fname);

    mkdir_recurse(gstat::dirname(fname));
    std::ofstream fout(fname.c_str());
    if (!fout.good()) {
        std::cout << "CRITIAL ERROR: Cannot open file!" << std::endl;
        return 1;
    }

    fout << "#ifndef " << guard << std::endl
        << "#define " << guard << std::endl <<std::endl;

    std::list<std::string> vals({ "AA", "AU", "DD", "DU", "UA", "UD", "UU" });
    for (std::list<std::string>::const_iterator i = vals.begin(); i != vals.end(); ++i) {
        fout << "#define HASH_CASE_" << *i << " "
            << std::to_string(gstat::hash_two_places(*i)) << std::endl;
    }
    fout << std::endl << "#endif  // " << guard;
    fout.close();

    return 0;
}
