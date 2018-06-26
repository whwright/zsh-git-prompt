#include <cstdint>
#include <cstdio>
#include <fstream>
#include <iostream>

#include <gtest/gtest.h>

#include "src/lib.h"

// Capture stdout
// testing::internal::CaptureStdout();
// std::cout << "My test"
// std::string output = testing::internal::GetCapturedStdout();
// std::cout << testing::internal::GetCapturedStdout() << std::endl;
//
// TODO: Test coverage, I'm lazy

/**
 * A simple tempfile generate. Write the text
 * passed in to a new file selected.
 */
class TFile {
 public:
    explicit TFile(const std::string &text): fname(select_temp()) {
        std::ofstream fout(fname.c_str());
        fout << text;
    }
    explicit TFile(const std::string &text, const std::string &_fname): fname(_fname) {
        std::ofstream fout(fname.c_str());
        fout << text;
    }
    ~TFile() {
        std::remove(this->fname.c_str());
    }

    std::string select_temp() {
        std::string prefix = "/tmp/tempGT";
        std::uint_fast16_t count = 0;
        std::string temp = prefix + std::to_string(count);

        while (gstat::file_exists(temp)) {
            ++count;
            temp = prefix + std::to_string(count);
        }

        return temp;
    }

    std::string fname;
};


TEST(GStatParseBranch, BranchLocal) {
    std::string temp_dummy;
    std::string input("## master");
    gstat::GBranch result = gstat::parse_branch(input, temp_dummy);
    EXPECT_EQ(result.branch, "master");
}

TEST(GStatParseBranch, BranchUpstream) {
    std::string temp_dummy;
    std::string input("## master...up/master");
    gstat::GBranch result = gstat::parse_branch(input, temp_dummy);
    EXPECT_EQ(result.branch, "master");
    EXPECT_EQ(result.upstream, "up/master");
}

TEST(GStatParseBranch, BranchOnHash) {
    TFile temp(std::string("54321"));
    std::string input("## HEAD (no branch)");
    gstat::GBranch result = gstat::parse_branch(input, temp.fname);
    EXPECT_EQ(result.branch, "54321");
    EXPECT_EQ(result.upstream, "..");
}

TEST(GStatParseBranch, BranchInitGitLess217) {
    std::string temp_dummy;
    std::string input("## Initial commit on master");
    gstat::GBranch result = gstat::parse_branch(input, temp_dummy);
    EXPECT_EQ(result.branch, "master");
    EXPECT_EQ(result.upstream, "..");
}

TEST(GStatParseBranch, BranchInitGitGreater217) {
    std::string temp_dummy;
    std::string input("## No commits yet on master");
    gstat::GBranch result = gstat::parse_branch(input, temp_dummy);
    EXPECT_EQ(result.branch, "master");
    EXPECT_EQ(result.upstream, "..");
}

TEST(GStatParseRemote, AheadTwo) {
    std::string input("## master...up/master [ahead 2]");
    gstat::GRemote result = gstat::parse_remote(input);
    EXPECT_EQ(result.ahead, 2);
    EXPECT_EQ(result.behind, 0);
}

TEST(GStatParseRemote, BehindOne) {
    std::string input("## master...up/master [behind 1]");
    gstat::GRemote result = gstat::parse_remote(input);
    EXPECT_EQ(result.ahead, 0);
    EXPECT_EQ(result.behind, 1);
}

TEST(GStatParseRemote, AheadTwoBehindOne) {
    std::string input("## master...up/master [ahead 2, behind 1]");
    gstat::GRemote result = gstat::parse_remote(input);
    EXPECT_EQ(result.ahead, 2);
    EXPECT_EQ(result.behind, 1);
}

TEST(GStatParseRemote, RemoteBranchGone) {
    std::string input("## master...up/master [gone]");
    gstat::GRemote result = gstat::parse_remote(input);
    EXPECT_EQ(result.ahead, 0);
    EXPECT_EQ(result.behind, 0);
}

TEST(GStatParseStats, AllTestCases) {
    const char *possible_strings[] = {
        "?? untracked1",
        "?? untracked2",
        "?? untracked3",
        "AA conflicts1",
        "AU conflicts2",
        "DD conflicts3",
        "DU conflicts4",
        "UA conflicts5",
        "UD conflicts6",
        "UD conflicts7",
        "A_ staged1",
        "C_ staged2",
        "D_ staged3",
        "M_ staged4",
        "R_ staged5",
        "_C changed1",
        "_D changed2",
        "_M changed3",
        "_R changed4",
    };
    std::vector<std::string> lines(possible_strings, possible_strings + 19);

    gstat::GStats result = gstat::parse_stats(lines);
    EXPECT_EQ(result.changed, 4);
    EXPECT_EQ(result.conflicts, 7);
    EXPECT_EQ(result.staged, 5);
    EXPECT_EQ(result.untracked, 3);
}

TEST(GStatPath, Join) {
    std::string input("/usr/bin");
    std::string leaf("gcc");
    std::string result = gstat::join(input, leaf);
    EXPECT_EQ(result, "/usr/bin/gcc");
}

TEST(GStatPath, Basename) {
    std::string input("/usr/bin/gcc");
    std::string result = gstat::basename(input);
    EXPECT_EQ(result, "gcc");
}

TEST(GStatPath, Dirname) {
    std::string input("/usr/bin/gcc");
    std::string result = gstat::dirname(input);
    EXPECT_EQ(result, "/usr/bin");
}

TEST(GStatPath, FileExists) {
    std::string input = gstat::join(gstat::dirname(gstat::find_git_root()), std::string("CMakeLists.txt"));
    EXPECT_TRUE(gstat::file_exists(input));
    EXPECT_FALSE(gstat::file_is_dir(input));
}

TEST(GStatPath, FileIsDir) {
    std::string input = gstat::join(gstat::dirname(gstat::find_git_root()), std::string("src"));
    EXPECT_TRUE(gstat::file_exists(input));
    EXPECT_TRUE(gstat::file_is_dir(input));
}

TEST(GStatPath, GetCWD) {
    std::string result = gstat::get_cwd();
    EXPECT_EQ(gstat::basename(result), "build");
}

TEST(GStatPath, FindGitRoot) {
    std::string expect = gstat::get_cwd();
    while (!gstat::file_exists(gstat::join(expect, std::string(".git")))) {
        expect = gstat::dirname(expect);
    }
    expect = gstat::join(expect, std::string(".git"));

    std::string result = gstat::find_git_root();
    EXPECT_EQ(result, expect);
}

TEST(GStatHash, SimpleHash) {
    std::string input = "AA";
    int val = static_cast<int>('A');
    EXPECT_EQ(gstat::hash_two_places(input), val * 1000 + val);
}

TEST(GStatRunCmd, SimpleEcho) {
    std::string input = "echo \"Hello!\"";
    std::string result = gstat::run_cmd(input.c_str());
    EXPECT_EQ(result, "Hello!\n");
}

TEST(GStatStashCount, NoFile) {
    std::string temp = "tempzzzz";
    EXPECT_EQ(gstat::stash_count(temp), "0");
}

TEST(GStatStashCount, FileTwoEntries) {
    std::string text("This is a first stash\nSecond stash");
    TFile temp(text);
    EXPECT_EQ(gstat::stash_count(temp.fname), "2");
}

TEST(GStatRebaseProgress, NoFile) {
    std::string temp = "tempzzzz";
    EXPECT_EQ(gstat::rebase_progress(temp), "0");
}

TEST(GStatRebaseProgress, ShowProgress) {
    std::string cwd = gstat::get_cwd();
    TFile next(std::string("2"), gstat::join(cwd, std::string("next")));
    TFile last(std::string("5"), gstat::join(cwd, std::string("last")));
    EXPECT_EQ(gstat::rebase_progress(cwd), "2/5");
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
