#include <iostream>

int main() {
  // Flush after every std::cout / std:cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;
  bool _exit = false;
  while (1){
  // Uncomment this block to pass the first stage
  std::cout << "$ ";

  std::string input;
  std::getline(std::cin, input);
  if(input == "exit 0") {
    _exit = true;
    break;
  }
  std::cout << input << ": command not found" << std::endl;
  }
}
