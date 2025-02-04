#include <iostream>
#include <sstream>
#include <vector>
#include <algorithm>  // For std::find
#include <cstdlib>    // For getenv
#include <unistd.h>   // For access()
#include <sys/stat.h> // For stat
#include <cstring>    // For strtok

// List of built-in commands
std::vector<std::string> commands = {"echo", "exit", "type"};

// Function to check if a file is executable
bool is_executable(const std::string &path) {
    return access(path.c_str(), X_OK) == 0;
}

// Function to search for an executable in PATH directories
std::string find_in_path(const std::string &command) {
    char *path_env = getenv("PATH"); // Get the PATH environment variable
    if (!path_env) return "";        // If PATH is not set, return empty

    std::string path_str(path_env);
    std::stringstream ss(path_str);
    std::string dir;

    while (std::getline(ss, dir, ':')) { // Split PATH by ':'
        std::string full_path = dir + "/" + command;
        if (is_executable(full_path)) {
            return full_path; // Found the executable
        }
    }
    return ""; // Not found
}

int main() {
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
            while (_str >> _arg) {
                std::cout << _arg << " ";
            }
            std::cout << std::endl;
        } 
        else if (_cmd == "type") {
            while (_str >> _arg) {
                // 1. Check if it's a built-in command
                if (std::find(commands.begin(), commands.end(), _arg) != commands.end()) {
                    std::cout << _arg << " is a shell builtin" << std::endl;
                } 
                // 2. Search in PATH for executable
                else {
                    std::string path = find_in_path(_arg);
                    if (!path.empty()) {
                        std::cout << _arg << " is " << path << std::endl;
                    } 
                    // 3. If not found
                    else {
                        std::cout << _arg << ": not found" << std::endl;
                    }
                }
            }
        } 
        else if (!_cmd.empty()) {
            std::cout << input << ": command not found" << std::endl;
        }
    }

    return 0;
}