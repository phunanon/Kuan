#pragma once

typedef uint16_t onum;
typedef uint16_t inum;
typedef uint16_t argnum;
typedef uint32_t fid;

#define NUM_OBJ 255

enum ObjType : uint8_t {
  String
};
enum SimpleValType : uint8_t {
  N, T, F, Character
};
enum OpType : uint8_t {
  NONE_0, INC_1, DEC_1, ADD_2, ADD_V, SUB_2, SUB_V, GTHAN_2,
  STR_V, GET_STR_0, PRINTLN_1
};
enum InsType : uint8_t {
  RETURN, PUSH_NUM, PUSH_CHAR, PUSH_STR, PUSH_PARA,
  CALL, EXECUTE, SKIP, IF
};

//These tables describe the symbols and their arities
static const char* opSymbols[] = {
  "", "inc", "dec", "+", "+", "-", "-",
  "<", "str", "get-str", "println",
  0
};
static const uint8_t opArities[] = {
  1, 1, 2, 255, 2, 255,
  2, 255, 0, 1
};