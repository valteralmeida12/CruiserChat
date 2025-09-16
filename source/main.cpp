#include "chatBot.h"
#include <iostream>
#include <fstream>
#include <ctime>
#include <filesystem>
#include <readline/readline.h>
#include <readline/history.h>

// Color codes for terminal output
const std::string RESET = "\033[0m";
const std::string CYAN = "\033[96m";
const std::string YELLOW = "\033[93m";
const std::string BLUE = "\033[94m";
const std::string RED = "\033[91m";
const std::string GREEN = "\033[92m";

void writeLogEntry(std::ofstream& log, const std::string& speaker, const std::string& content) {
    if (speaker == "You") {
        log << "USER:\n";
        log << "  " << content << "\n\n";
    } else {
        log << "BOT:\n";
        log << "  " << content << "\n\n";
    }
}

std::string get_user_input() {
    char* input_line = readline((CYAN + "You: " + RESET).c_str());
    if (!input_line){
        return ">>exit";
    }

    std::string input = input_line;
    free(input_line);
    
    if (!input.empty()) {
        add_history(input.c_str());
    }
    
    return input;
}


std::string get_multiline_input() {
    std::string input = get_user_input();
    
    if (input == ">>exit") return input;
    
    std::string line;
    while (true) {
        // Print the prompt
        printf("<press Enter to send>");
        
        char* line_ptr = readline("");
        if (!line_ptr) break;
        line = line_ptr;
        free(line_ptr);

        // Move cursor to beginning of line and clear it
        printf("\r\033[K");
        fflush(stdout);
        
        if (line.empty()) break;
        if (line == ">>exit") return line;
        
        input += "\n" + line;
    }
    
    return input;
}


void print_help() {
    std::cout << BLUE << "\n=== Available Commands ===\n" << RESET;
    std::cout << "--model (--m)   - Provide path to the model file (default: ../models/Llama-3.2-3B-Instruct-Q8_0.gguf)\n";
    std::cout << "--help          - Show this help message\n";
    std::cout << ">>exit          - Exit the application\n";
    std::cout << BLUE << "==========================\n\n" << RESET;
}

int main(int argc, char** argv) {
    // Override with first argument if provided
    std::string model_path = "../models/Llama-3.2-3B-Instruct-Q6_K.gguf";
    for(int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "--help") {
            print_help();
            return 0;
        }
        else if (std::string(argv[i]) == "--model" || std::string(argv[i]) == "--m") {
            if (i + 1 < argc) {
                model_path = argv[++i];  // Get the next argument as model path
            } else {
                std::cerr << "Error: --model requires a file path\n";
                print_help();
                return 1;
            }
        }
        else{
            std::cerr << RED << "Unknown argument: " << argv[i] << RESET << std::endl;
            return 1;
        }
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
        std::cerr << RED << "Failed to open log file: " << logfile << std::endl;
        return 1;
    }

    // Write log header
    log << "═══════════════════════════════════════════════════════════════════════════════\n";
    log << "                              CHAT SESSION LOG\n";
    log << "                         " << std::string(timeBuffer) << "\n";
    log << "═══════════════════════════════════════════════════════════════════════════════\n\n";

    try {
        chatbot bot(model_path, 0.8f, 0.9f);

        std::cout << GREEN << "Welcome to CruiserChat\n";
        std::cout << "Type '>>exit' to quit the application.\n\n" << RESET;

        int messageCount = 0;
        while (true) {
            // USE READLINE FUNCTION INSTEAD OF OLD INPUT HANDLING
            std::string input = get_multiline_input();
            if (input == ">>exit") break;
            
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
