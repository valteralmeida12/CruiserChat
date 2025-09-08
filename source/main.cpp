#include "chatBot.h"
#include <iostream>
#include <fstream>
#include <ctime>
#include <filesystem>

// Color codes for terminal output
const std::string RESET = "\033[0m";
const std::string CYAN = "\033[96m";
const std::string YELLOW = "\033[93m";
const std::string RED = "\033[91m";
const std::string GREEN = "\033[92m";

void writeLogEntry(std::ofstream& log, const std::string& speaker, const std::string& content) {
    if (speaker == "You") {
        log << "┌─ USER ─────────────────────────────────────────────────────────────────────\n";
        log << "│ " << content << "\n";
        log << "└────────────────────────────────────────────────────────────────────────────\n\n";
    } else {
        log << "╭─ BOT ──────────────────────────────────────────────────────────────────────\n";
        log << "│ " << content << "\n";
        log << "╰────────────────────────────────────────────────────────────────────────────\n\n";
    }
}

int main(int argc, char** argv) {
    // Override with first argument if provided
    std::string model_path;
    if (argc < 2) {
        model_path = "../models/Phi-3-mini-4k-instruct-q4.gguf";
    }
    else {
        model_path = argv[1];
    }
    // Save model path from command line

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
        std::cerr << RED << "Failed to open log file: " << logfile << std::endl;
        return 1;
    }

    // Write log header
    log << "═══════════════════════════════════════════════════════════════════════════════\n";
    log << "                              CHAT SESSION LOG\n";
    log << "                         " << std::string(timeBuffer) << "\n";
    log << "═══════════════════════════════════════════════════════════════════════════════\n\n";


    try {
        chatbot bot(model_path);

        std::cout << GREEN << "Welcome to CruiserChat\n";

        int messageCount = 0;
        while (true) {
            std::cout << CYAN << "You: ";
            std::string input;
            std::string line;
            
            // Get first line
            if (!std::getline(std::cin, line)) break;
            if (line == ">>exit") break;
            
            input = line;
            
            // Clear any remaining newline and check for additional input
            // This handles multi-line pasted content (snippets for example)
            while (std::cin.rdbuf()->in_avail() > 0 && std::cin.peek() != EOF) {
                char nextChar = std::cin.peek();
                if (nextChar == '\n') {
                    std::cin.ignore(1); // consume the newline
                    break; 
                } else {
                    // If there's more content, read the next line
                    if (std::getline(std::cin, line)) {
                        input += "\n" + line;
                    } else {
                        break;
                    }
                }
            }
            
            messageCount++;

            // Save user input to log file
            writeLogEntry(log, "You", input);

            std::cout << YELLOW << "Bot: ";
            std::string response = bot.get_response(input);
            std::cout << response << RESET << "\n\n";

            // Log bot response
            writeLogEntry(log, "Bot", response);

            log.flush(); // ensure it's written immediately
        }

        // Write log footer
        log << "═══════════════════════════════════════════════════════════════════════════════\n";
        log << "                           SESSION ENDED\n";
        log << "                      Total Messages: " << messageCount << "\n";
        log << "═══════════════════════════════════════════════════════════════════════════════\n";

    } catch (const std::exception& e) {
        std::cerr << RED << "\nFatal error: " << e.what() << std::endl;
        return 1;
    }

    log.close();
    std::cout << GREEN <<  "Conversation saved to: " << logfile << "\n";

    return 0;
}
