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
    hasEntry |= !parsed[i]->id;
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
      if (!line) return;
      linenoiseHistoryAdd(line);
      input = string(line);
      if (!input.length()) continue;
      free(line);
      if (input == "q") break;
    }
    vm.functions.remove(0);
    if (parseAndLoad(vm, input)) {
      auto v = vm.executeFunction(0, previous);
      previous.tryDelete();
      previous = v;
      printf("%f\n", v.as.num); //TODO to string
    }
  }
  previous.tryDelete();
}

int main (int argc, char *argv[]) {
  kb_listen();
  if (argc > 1) {
    ifstream infile{string(argv[1])};
    auto vm = KuanVM();
    parseAndLoad(vm, {istreambuf_iterator<char>(infile), istreambuf_iterator<char>()});
    auto ret = vm.executeFunction(0, {});
    if (argc == 3 && string(argv[2]) == "-r")
      printf("%f\n", ret.as.num); //TODO to string
  } else repl();

  if (auto leaks = Object::checkMemLeak())
    printf("Warning: %d ARC memory leak/s detected.\n", leaks);
  
  printf("\nDone.\n");
  return 0;
}

  /*Instruction function[1024] {
    {PUSH_NUM, 123},
    {PUSH_NUM, 456},
    {EXECUTE, {.op = {ADD, 2}}},
    {EXECUTE, {.op = {PRINTLN, 1}}}
  };*/


/*
  //(println (fib 35))
  Instruction function0[] {
    {PUSH_NUM, 35},
    {CALL, {.call = {1, 1}}},
    {EXECUTE, {.op = {PRINTLN_1, 1}}}
  };
  //(fn fib n (if (< n 2) n (+ (fib (- n 1)) (fib (- n 2)))))
  Instruction function1[] {
    {PUSH_PARA, 0},
    {PUSH_NUM, 2},
    {EXECUTE, {.op = {GTHAN_2, 2}}},
    {IF, 2},
    {PUSH_PARA, 0},
    {SKIP, 9},
    {PUSH_PARA, 0},
    {PUSH_NUM, 1},
    {EXECUTE, {.op = {SUB_2, 2}}},
    {CALL, {.call = {1, 1}}},
    {PUSH_PARA, 0},
    {PUSH_NUM, 2},
    {EXECUTE, {.op = {SUB_2, 2}}},
    {CALL, {.call = {1, 1}}},
    {EXECUTE, {.op = {ADD_2, 2}}}
  };
*/
/*
  //(println "Hello, " (get-str) \.)
  auto hello = new Object(new string("Hello, "), true);
  Instruction function0[] {
    {PUSH_STR, {.obj = hello}},
    {EXECUTE, {.op = {GET_STR_0, 0}}},
    {PUSH_CHAR, '.'},
    {EXECUTE, {.op = {STR_V, 3}}},
    {EXECUTE, {.op = {PRINTLN_1, 1}}}
  };
  auto vm = KuanVM();
  vm.functions.add(0, function0, sizeof(function0)/sizeof(Instruction), {hello});
  //vm.addFunction(1, function1, sizeof(function1)/sizeof(Instruction));
  vm.executeFunction(0, 0, 0);
*/