#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include "thompson.h"

using namespace regexnfa;

void interactive_mode() {
    std::cout << "Regex → NFA (Thompson) interactive mode\n";
    std::cout << "Enter a regex (supports: |, *, parentheses). Enter empty to quit.\n";
    while (true) {
        std::cout << "regex> ";
        std::string regex;
        if (!std::getline(std::cin, regex)) break;
        if (regex.empty()) break;
        try {
            NFA nfa = ThompsonBuilder::build_from_regex(regex);
            std::cout << "NFA built. Start= S" << nfa.start_state() << "\n";
            std::cout << nfa.debug_dump() << std::endl;
            std::cout << "DOT output (first 6 lines):\n";
            auto dot = nfa.to_dot();
            int count = 0;
            std::istringstream iss(dot);
            std::string line;
            while (count < 6 && std::getline(iss, line)) {
                std::cout << line << "\n";
                ++count;
            }
            std::cout << "Enter string to test (empty to skip): ";
            std::string s;
            std::getline(std::cin, s);
            if (!s.empty()) {
                bool ok = nfa.accepts(s);
                std::cout << (ok ? "ACCEPTED" : "REJECTED") << "\n";
            }
        } catch (const std::exception &e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
    }
}

void batch_mode_from_file(const std::string &path) {
    std::ifstream ifs(path);
    if (!ifs) { std::cerr << "Cannot open " << path << "\n"; return; }
    std::string line;
    while (std::getline(ifs, line)) {
        if (line.empty()) continue;
        if (line[0] == '#') continue;
        std::istringstream iss(line);
        std::string regex, test, expected;
        if (!std::getline(iss, regex, '\t')) continue;
        if (!std::getline(iss, test, '\t')) test = "";
        if (!std::getline(iss, expected, '\t')) expected = "";
        try {
            NFA nfa = ThompsonBuilder::build_from_regex(regex);
            bool res = nfa.accepts(test);
            std::cout << regex << "\t'" << test << "' => " << (res?"ACCEPT":"REJECT");
            if (!expected.empty()) std::cout << " (expected " << expected << ")";
            std::cout << "\n";
        } catch (const std::exception &e) {
            std::cerr << "Error building regex '" << regex << "': " << e.what() << "\n";
        }
    }
}

int main(int argc, char** argv) {
    if (argc == 1) {
        interactive_mode();
    } else if (argc == 2) {
        std::string arg = argv[1];
        if (arg == "--help" || arg == "-h") {
            std::cout << "Usage: regex_nfa_cli [tests_file.txt]\nIf no file provided, interactive mode runs." << std::endl;
            return 0;
        }
        batch_mode_from_file(arg);
    } else {
        std::cerr << "Too many arguments\n";
        return 2;
    }
    return 0;
}
