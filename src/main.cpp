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
// tokenize: Enhanced tokenizer that splits the input line into tokens while
// preserving text within single or double quotes.
// 
// For single quotes, the text is taken literally.
// For double quotes, the backslash (\) escapes only these characters: \, $,
// " or newline.
// For example:
//   Input:  echo "hello  world" 'and goodbye'
//   Output: ["echo", "hello  world", "and goodbye"]
// ----------------------------------------------------------------------------
std::vector<std::string> tokenize(const std::string &input) {
  enum State { UNQUOTED, SINGLE_QUOTED, DOUBLE_QUOTED };
  State state = UNQUOTED;
  std::vector<std::string> tokens;
  std::string token;

  for (size_t i = 0; i < input.size(); i++) {
    char c = input[i];
    switch(state) {
      case UNQUOTED:
        if (isspace(c)) {
          if (!token.empty()) {
            tokens.push_back(token);
            token.clear();
          }
        } else if (c == '\'') {
          state = SINGLE_QUOTED;
        } else if (c == '"') {
          state = DOUBLE_QUOTED;
        } else {
          token.push_back(c);
        }
        break;

      case SINGLE_QUOTED:
        if (c == '\'') {
          state = UNQUOTED;
        } else {
          token.push_back(c);
        }
        break;

      case DOUBLE_QUOTED:
        if (c == '"') {
          state = UNQUOTED;
        } else if (c == '\\') {
          // In double quotes, backslash escapes \, $, " or newline.
          if (i + 1 < input.size()) {
            char next = input[i+1];
            if (next == '\\' || next == '$' || next == '"' || next == '\n') {
              token.push_back(next);
              i++; // Skip the escaped character.
            } else {
              // Backslash remains literal if not followed by special char.
              token.push_back(c);
            }
          } else {
            token.push_back(c);
          }
        } else {
          token.push_back(c);
        }
        break;
    }
  }
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

    // Tokenize the input with support for single and double quotes.
    std::vector<std::string> args = tokenize(input);
    if (args.empty())
     
