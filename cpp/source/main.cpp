#include "chatBot.h"
#include <iostream>
#include <fstream>
#include <ctime>
#include <filesystem>

int main(int argc, char** argv) {
    // Override with first argument if provided
    if (argc < 1) {
        std::cerr << "Model path is incorrect!";
        return 1;
    }

    // Ensure logs directory exists
    std::filesystem::path logDir = "../Cruiser_Chat_Logs";
    if (!std::filesystem::exists(logDir)) {
        std::filesystem::create_directories(logDir);
    }

    // Create a timestamped log file
    std::time_t now;
    std::tm* timeinfo;
    char timeBuffer[80];

    std::time(&now);
    timeinfo = std::localtime(&now);
    std::strftime(timeBuffer, sizeof(timeBuffer), "%Y%m%d_%H%M%S", timeinfo);

    std::string logfile = (logDir / (std::string(timeBuffer) + ".txt")).string();
    std::ofstream log(logfile, std::ios::out);

    if (!log.is_open()) {
        std::cerr << "Failed to open log file: " << logfile << std::endl;
        return 1;
    }

    // Save model path from command line
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
            
            // Clear any remaining newline and check for additional input
            // This handles multi-line pasted content properly
            while (std::cin.rdbuf()->in_avail() > 0 && std::cin.peek() != EOF) {
                char nextChar = std::cin.peek();
                if (nextChar == '\n') {
                    std::cin.ignore(1); // consume the newline
                    break; // stop here for single Enter press
                } else {
                    // There's more content, read the next line
                    if (std::getline(std::cin, line)) {
                        input += "\n" + line;
                    } else {
                        break;
                    }
                }
            }
            
            // Save user input to log file
            log << "You: " << input << "\n";

            std::cout << "Bot: ";
            std::string response = bot.get_response(input);
            std::cout << response << "\n\n";

            // Log bot response
            log << "Bot: " << response << "\n\n";

            log.flush(); // ensure it's written immediately
        }
    } catch (const std::exception& e) {
        std::cerr << "\nFatal error: " << e.what() << std::endl;
        return 1;
    }
    log.close();
    std::cout << "Conversation saved to: " << logfile << "\n";
    return 0;
}
