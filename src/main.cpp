#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <cinttypes>
#include <string>
#include <vector>
#include "KuanVM.hpp"
#include "Parser.hpp"
using namespace std;

int main () {

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
  Parser::parse("(+ 2 2)");
  
  printf("\nDone.\n");
  if (auto leaks = Object::checkMemLeak())
    printf("Warning: %d ARC memory leak/s detected.\n", leaks);
}
