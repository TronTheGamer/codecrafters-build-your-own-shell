#include <iostream>
#include <sstream>
#include <vector>
#include <algorithm> // For std::find

// List of built-in commands
std::vector<std::string> commands = {"echo", "exit", "type"};

int main() {
    // Flush after every std::cout / std::cerr
    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;

    while (true) {
        std::cout << "$ "; // Shell prompt

        std::string input;
        std::getline(std::cin, input); // Read user input

        std::stringstream _str(input);
        std::string _cmd, _arg;

        _str >> _cmd; // Extract the command

        if (_cmd == "exit") {
            break; // Exit the shell
        } 
        else if (_cmd == "echo") {
            // Echo the arguments
            while (_str >> _arg) {
                std::cout << _arg << " ";
            }
            std::cout << std::endl;
        } 
        else if (_cmd == "type") {
            // Check each argument after 'type'
            while (_str >> _arg) {
                // Check if the argument is a known built-in command
                if (std::find(commands.begin(), commands.end(), _arg) != commands.end()) {
                    std::cout << _arg << " is a shell builtin" << std::endl;
                } else {
                    std::cout << _arg << ": not found" << std::endl;
                }
            }
        } 
        else if (!_cmd.empty()) {
            // Handle unrecognized commands
            std::cout << input << ": command not found" << std::endl;
        }
    }

    return 0;
}

