
#include <iostream>
#include <fstream>
#include <cstdlib>
#include "nfa.h"
#include "thompson.h"

using namespace regexnfa;

// Helper function: write DOT file and call dot to generate PNG
void visualize_nfa(const NFA &nfa, const std::string &filename_base) {
    std::string dot_content = nfa.to_dot();
    std::string dot_filename = filename_base + ".dot";
    std::ofstream ofs(dot_filename);
    if (!ofs) {
        std::cerr << "Failed to write DOT file: " << dot_filename << std::endl;
        return;
    }
    ofs << dot_content;
    ofs.close();

    // Call Graphviz dot to generate PNG
    std::string cmd = "dot -Tpng " + dot_filename + " -o " + filename_base + ".png";
    int ret = std::system(cmd.c_str());
    if (ret != 0) {
        std::cerr << "Graphviz dot command failed: " << cmd << std::endl;
    } else {
        std::cout << "Visualization generated: " << filename_base << ".png" << std::endl;
    }
}

int main() {
    std::cout << "Regex NFA Test\n";
    std::cout << "Enter a regex (extended operators supported: +, *, ?, |, parentheses):\n";

    std::string regex;
    std::getline(std::cin, regex);

    try {
        NFA nfa = ThompsonBuilder::build_from_regex(regex);
        std::cout << "NFA built successfully!\n";

        std::cout << "Start state: S" << nfa.start_state() << "\nAccept states: ";
        for (int s : nfa.accept_states()) std::cout << "S" << s << " ";
        std::cout << "\n";

        std::cout << "\nDOT output preview (first 10 lines):\n";
        std::string dot = nfa.to_dot();
        size_t pos = 0, line_count = 0;
        while (line_count < 10) {
            size_t next = dot.find('\n', pos);
            if (next == std::string::npos) break;
            std::cout << dot.substr(pos, next - pos) << "\n";
            pos = next + 1;
            line_count++;
        }

        // Generate DOT and PNG automatically
        visualize_nfa(nfa, "nfa_visualization");

    } catch (const std::exception &e) {
        std::cerr << "Error building NFA: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
