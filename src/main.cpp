#include <iostream>
#include <sstream>
#include <vector>
#include <algorithm>  // For std::find
#include <cstdlib>    // For getenv
#include <unistd.h>   // For fork(), execvp(), access()
#include <sys/wait.h> // For waitpid()
#include <sys/stat.h> // For stat

// List of built-in commands
std::vector<std::string> commands = {"echo", "exit", "type", "pwd","cd"};

// Check if the file is executable
bool is_executable(const std::string &path)
{
  return access(path.c_str(), X_OK) == 0;
}

// Find an executable in PATH directories
std::string find_in_path(const std::string &command)
{
  char *path_env = getenv("PATH");
  if (!path_env)
    return "";

  std::stringstream ss(path_env);
  std::string dir;
  while (std::getline(ss, dir, ':'))
  {
    std::string full_path = dir + "/" + command;
    if (is_executable(full_path))
    {
      return full_path;
    }
  }
  return "";
}

// Execute external command with arguments
void execute_external_command(const std::vector<std::string> &args)
{
  // Convert std::vector<std::string> to char* array for execvp
  std::vector<char *> argv;
  for (const auto &arg : args)
  {
    argv.push_back(const_cast<char *>(arg.c_str()));
  }
  argv.push_back(nullptr); // Null-terminate the argument list

  pid_t pid = fork(); // Create a child process
  if (pid == 0)
  {
    // Child process
    execvp(argv[0], argv.data());
    // If execvp fails
    perror("execvp failed");
    exit(EXIT_FAILURE);
  }
  else if (pid > 0)
  {
    // Parent process waits for the child to finish
    int status;
    waitpid(pid, &status, 0);
  }
  else
  {
    // Fork failed
    perror("fork failed");
  }
}

int main()
{
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;

  while (true)
  {
    std::cout << "$ ";

    std::string input;
    std::getline(std::cin, input);

    std::stringstream _str(input);
    std::string _cmd, _arg;
    std::vector<std::string> args;

    // Extract command and arguments
    while (_str >> _arg)
    {
      args.push_back(_arg);
    }

    if (args.empty())
      continue; // Ignore empty input

    _cmd = args[0]; // First token is the command

    if (_cmd == "exit")
    {
      break;
    }
    else if (_cmd == "echo")
    {
      for (size_t i = 1; i < args.size(); ++i)
      {
        std::cout << args[i] << " ";
      }
      std::cout << std::endl;
    }
    else if (_cmd == "type")
    {
      for (size_t i = 1; i < args.size(); ++i)
      {
        std::string &arg = args[i];
        if (std::find(commands.begin(), commands.end(), arg) != commands.end())
        {
          std::cout << arg << " is a shell builtin" << std::endl;
        }
        else
        {
          std::string path = find_in_path(arg);
          if (!path.empty())
          {
            std::cout << arg << " is " << path << std::endl;
          }
          else
          {
            std::cout << arg << ": not found" << std::endl;
          }
        }
      }
    }
    else if(_cmd == "pwd"){
      char cwd[1024];
      if (getcwd(cwd, sizeof(cwd)) != NULL)
        std::cout << cwd << std::endl;
      else
        perror("getcwd() error");
    }
    else if(_cmd == "cd"){
      if(args.size() == 1){
        chdir(getenv("HOME"));
      }
      else if(access(args[1].c_str(), F_OK) == -1){
        std::cout << "cd: " << args[1] << ": No such file or directory" << std::endl;
      }
      else{
        chdir(args[1].c_str());
      }
    }
    else
    {
      // Check if it's an external command
      std::string path = find_in_path(_cmd);
      if (!path.empty())
      {
        // Replace the command with its full path
        // args[0] = path;                        // Since the command is in the $PATH, no need to replace
        execute_external_command(args);
      }
      else
      {
        std::cout << _cmd << ": command not found" << std::endl;
      }
    }
  }

  return 0;
}
