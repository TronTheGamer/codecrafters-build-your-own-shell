#include <iostream>
#include <sstream>

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
  std::stringstream _str(input);
  std::string _cmd;
  std::string _arg;

  _str >> _cmd;

  if(_cmd == "exit") {
    _exit = true;
    break;
  } else if(_cmd == "echo"){
    while (_str >> _arg){
      std::cout << _arg << " ";
    }
    std::cout << std::endl;
  }
  std::cout << input << ": command not found" << std::endl;
  }
}
