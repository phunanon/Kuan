#include <string>
#include <vector>
#include <memory>
#include <fstream>
#include <iterator>
#include "linenoise/linenoise.h"
#include "keypresses.c"
#include "KuanVM.hpp"
#include "Parser.hpp"
using namespace std;

bool parseAndLoad (KuanVM &vm, string input) {
  bool hasEntry = false;
  auto parsed = Parser::parse(input);
  for (uint i = 0; i < parsed.size(); ++i) {
    if (parsed[i]->id && !parsed[i]->ins.size()) {
      printf("ERR: `%s` is malformed, so discarded.\n", parsed[i]->name.c_str());
      continue;
    }
    hasEntry |= !parsed[i]->id && parsed[i]->ins.size();
    vm.functions.add(move(parsed[i]));
  }
  return hasEntry;
}

void repl () {
  printf("Kuan REPL. %% gives previous result. Arrow keys navigate history/entry. q or ^C to quit.\n");
  auto vm = KuanVM();
  Value previous;
  while (true) {
    string input;
    {
      char* line = linenoise("> ");
      if (!line) break;
      linenoiseHistoryAdd(line);
      input = string(line);
      free(line);
      if (!input.length()) continue;
      if (input == "q") break;
    }
    vm.functions.remove(0);
    if (parseAndLoad(vm, input)) {
      auto v = vm.executeFunction(0, previous);
      previous.tryDelete();
      previous = v;
      printf("%s\n", vm.ValAsStr(v).c_str());
    }
  }
  previous.tryDelete();
}

int main (int argc, char *argv[]) {
  kb_listen();
  if (argc > 1) {
    auto vm = KuanVM();
    //Handle code as argument
    if (argv[1][0] == '(') {
      parseAndLoad(vm, string(argv[1]));
      auto ret = vm.executeFunction(0, {});
      printf("%s\n", vm.ValAsStr(ret).c_str());
      ret.tryDelete();
      return 0;
    }
    //Handle file as argument
    ifstream infile{string(argv[1])};
    parseAndLoad(vm, {istreambuf_iterator<char>(infile), istreambuf_iterator<char>()});
    auto ret = vm.executeFunction(0, {});
    if (argc == 3 && string(argv[2]) == "-r")
      printf("%s\n", vm.ValAsStr(ret).c_str());
    ret.tryDelete();
  } else repl();

  if (auto leaks = Object::checkMemLeak())
    printf("Warning: %d ARC memory leak/s detected.\n", leaks);
  return 0;
}