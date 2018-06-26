/*
 * A simple reimplementation in c++ for gitstatus.
 * Main just handles the input and delegates to library.
 */
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "lib.h"

/*
 * Main entry, this program works in two modes:
 *      1) If STDIN data present, read that and parse it. Assume it is the
 *         output of `git status --branch --porcelain`.
 *
 *      2) Otherwise, run the command yourself and parse it internally.
 */
int main() {
    std::vector<std::string> lines;
    std::string part;

    if (gstat::stdin_has_input()) {
        while (!std::cin.eof()) {
            std::getline(std::cin, part);
            lines.push_back(part);
        }

        lines.pop_back();
    } else {
        std::string all_text = gstat::run_cmd("git status --porcelain --branch 2>&1");

        for (std::string::const_iterator i = all_text.begin();
             i != all_text.end(); ++i) {
            if (*i == '\n') {
                lines.push_back(part);
                part.clear();
            } else {
                part += *i;
            }
        }
    }

    if (lines[0].find("fatal: ") == 0 &&
            lines[0].find("ot a git repository") != std::string::npos) {
        return 0;
    }

    // Must be in a git repository past here
    try {
        std::cout << gstat::current_gitstatus(lines) << std::endl;
    } catch (const std::runtime_error &e) {
        // pass
    }
}
