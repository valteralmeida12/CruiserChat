#include "chatBot.h"
#include <iostream>

int main(int argc, char** argv) {
    // Override with first argument if provided
    if (argc < 1) {
        std::cerr << "Model path is incorrect!";
        return 1;
    }

    std::string model_path = argv[1];

    try {
        chatbot bot(model_path);

        while (true) {
            std::cout << "You: ";
            std::string input;
            std::string line;
            
            // Get first line
            if (!std::getline(std::cin, line)) break;
            
            if (line == ">>exit") break;
            
            input = line;
            
            // Check if more input is available (like pasted content)
            while (std::cin.peek() != EOF && std::cin.peek() != '\n') {
                if (std::getline(std::cin, line)) {
                    input += "\n" + line;
                } else {
                    break;
                }
            }
            
            // Clear any remaining newline
            if (std::cin.peek() == '\n') {
                std::cin.ignore();
            }

            std::cout << "Bot: ";
            bot.get_response(input);
            std::cout << "\n\n";
        }
    } catch (const std::exception& e) {
        std::cerr << "\nFatal error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
