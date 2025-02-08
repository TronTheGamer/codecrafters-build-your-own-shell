#include <algorithm> // For std::find
#include <cctype>    // For isspace()
#include <cstdlib>   // For getenv, exit
#include <iostream>
#include <sstream>
#include <sys/stat.h> // For stat()
#include <sys/wait.h> // For waitpid()
#include <unistd.h>   // For fork(), execvp(), access(), chdir(), getcwd()
#include <vector>

// List of built-in commands.
std::vector<std::string> commands = {"echo", "exit", "type", "cd", "pwd"};

// ----------------------------------------------------------------------------
// is_executable: Check if a file at 'path' exists and is executable.
// ----------------------------------------------------------------------------
bool is_executable(const std::string &path) {
  return access(path.c_str(), X_OK) == 0;
}

// ----------------------------------------------------------------------------
// find_in_path: Searches for the executable 'command' in the directories
// specified in the PATH environment variable. Returns the full path if found,
// otherwise returns an empty string.
// ----------------------------------------------------------------------------
std::string find_in_path(const std::string &command) {
  char *path_env = getenv("PATH");
  if (!path_env)
    return "";

  std::string path_str(path_env);
  std::stringstream ss(path_str);
  std::string dir;
  while (std::getline(ss, dir, ':')) { // Split PATH by ':'
    std::string full_path = dir + "/" + command;
    if (is_executable(full_path)) {
      return full_path;
    }
  }
  return "";
}

// ----------------------------------------------------------------------------
// execute_external_command: Executes an external command with arguments.
// It creates a child process using fork(), then replaces the child process
// image with the new program using execvp().
// ----------------------------------------------------------------------------
void execute_external_command(const std::vector<std::string> &args) {
  // Convert std::vector<std::string> to a null-terminated C-style array.
  std::vector<char *> argv;
  for (const auto &arg : args) {
    argv.push_back(const_cast<char *>(arg.c_str()));
  }
  argv.push_back(nullptr); // Terminate the argument list.

  pid_t pid = fork(); // Create a child process.
  if (pid == 0) {
    // Child process: execute the external command.
    execvp(argv[0], argv.data());
    // If execvp returns, an error occurred.
    perror("execvp failed");
    exit(EXIT_FAILURE);
  } else if (pid > 0) {
    // Parent process: wait for the child process to finish.
    int status;
    waitpid(pid, &status, 0);
  } else {
    // Fork failed.
    perror("fork failed");
  }
}

// ----------------------------------------------------------------------------
// tokenize: Custom tokenizer that splits the input line into tokens while
// preserving text within single quotes (removing the quotes).
//
// For example:
//   Input:  echo 'hello world'
//   Output: ["echo", "hello world"]
//
// It toggles an "in_quote" flag when a single quote is encountered.
// ----------------------------------------------------------------------------
std::vector<std::string> tokenize(const std::string &input) {
  std::vector<std::string> tokens;
  std::string token;
  bool in_quote = false;

  for (size_t i = 0; i < input.size(); ++i) {
    char c = input[i];
    if (c == '\'') {
      // Toggle the quoting state.
      in_quote = !in_quote;
    } else if (isspace(c) && !in_quote) {
      // Outside of quotes, treat whitespace as a delimiter.
      if (!token.empty()) {
        tokens.push_back(token);
        token.clear();
      }
    } else {
      // Add character to the current token.
      token.push_back(c);
    }
  }
  // Add any remaining token.
  if (!token.empty()) {
    tokens.push_back(token);
  }
  return tokens;
}

int main() {
  // Flush output streams immediately.
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;

  while (true) {
    std::cout << "$ "; // Print shell prompt.

    std::string input;
    std::getline(std::cin, input);

    // Tokenize the input with support for single quotes.
    std::vector<std::string> args = tokenize(input);
    if (args.empty())
      continue; // Ignore empty input lines.

    // The first token is the command.
    std::string _cmd = args[0];

    // Handle built-in commands.
    if (_cmd == "exit") {
      break;
    } else if (_cmd == "cd") {
      std::string target;
      if (args.size() < 2) {
        // If no argument is provided, change to HOME directory.
        char *home = getenv("HOME");
        if (home) {
          target = home;
        } else {
          std::cerr << "cd: HOME not set\n";
          continue;
        }
      } else if (args[1] == "~") {
        char *home = getenv("HOME");
        if (home) {
          target = home;
        } else {
          std::cerr << "cd: HOME not set\n";
          continue;
        }
      } else if (args[1] == "-") {
        char *oldpwd = getenv("OLDPWD");
        if (oldpwd) {
          target = oldpwd;
        } else {
          std::cerr << "cd: OLDPWD not set\n";
          continue;
        }
      } else {
        target = args[1];
      }
      if (chdir(target.c_str()) != 0) {
        std::cerr << "cd: " << target << ": No such file or directory\n";
      }
    } else if (_cmd == "echo") {
      // Print all arguments after "echo" with a space separator.
      for (size_t i = 1; i < args.size(); ++i) {
        std::cout << args[i] << " ";
      }
      std::cout << std::endl;
    } else if (_cmd == "pwd") {
      // Implement pwd: print the current working directory.
      char buffer[1024];
      if (getcwd(buffer, sizeof(buffer)) != nullptr) {
        std::cout << buffer << std::endl;
      } else {
        perror("pwd");
      }
    } else if (_cmd == "type") {
      // Process each argument after "type".
      for (size_t i = 1; i < args.size(); ++i) {
        const std::string &arg = args[i];
        // Check for built-in commands first.
        if (std::find(commands.begin(), commands.end(), arg) !=
            commands.end()) {
          std::cout << arg << " is a shell builtin" << std::endl;
        } else {
          std::string path = find_in_path(arg);
          if (!path.empty()) {
            std::cout << arg << " is " << path << std::endl;
          } else {
            std::cout << arg << ": not found" << std::endl;
          }
        }
      }
    } else if (_cmd == "pwd") {
      std::cout << getenv("PWD") << std::endl;
    } else {
      // Handle external commands.
      std::string path = find_in_path(_cmd);
      if (!path.empty()) {
        // Replace the command with its full path.
        args[0] = path;
        execute_external_command(args);
      } else {
        std::cout << _cmd << ": command not found" << std::endl;
      }
    }
  }

  return 0;
}
