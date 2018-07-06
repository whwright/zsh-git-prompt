/**
 *     The MIT License
 *
 *     Copyright (c) 2018 Jeremy Pallats/starcraft.man
 *
 * A simple reimplementation in c++ for gitstatus.
 * Main just handles the input and delegates to library.
 */
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "src/gstat.h"

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
        while (std::getline(std::cin, part)) {
            lines.push_back(part);
        }
    } else {
        std::stringstream ssin(gstat::run_cmd("git status --porcelain --branch 2>&1"));
        while (std::getline(ssin, part)) {
            lines.push_back(part);
        }
    }

    if (lines.front().find("fatal: ") == 0 &&
            lines.front().find("ot a git repository") != std::string::npos) {
        return 0;
    }

    // Must be in a git repository past here
    try {
        std::cout << gstat::current_gitstatus(lines);
    } catch (const std::runtime_error &e) {
        // pass
    }
}
