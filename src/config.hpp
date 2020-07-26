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
  NONE_0, EXE_V, INC_1, DEC_1,
  ADD_2, ADD_V, NEG_1, SUB_2, SUB_V,
  MUL_2, MUL_V, DIV_2, DIV_V,
  GTHAN_2, GTHAN_V, LTHAN_2, LTHAN_V,
  GET_STR_0, GET_STR_1, GET_NUM_0, GET_NUM_1,
  STR_V, PRINTLN_1
};
enum InsType : uint8_t {
  RETURN, PUSH_NUM, PUSH_CHAR, PUSH_STR, PUSH_PARA,
  CALL, EXECUTE, SKIP, IF
};

//These tables describe the symbols and their arities
//  where 255 arity is varadic
static const char* opSymbols[] = {
  "", "exe", "inc", "dec",
  "+", "+", "-", "-", "-",
  "*", "*", "/", "/",
  "<", "<", ">", ">",
  "<=", "<=", ">=", ">=",
  "get-str", "get-str", "get-num", "get-num",
  "str", "println",
  0
};
static const uint8_t opArities[] = {
  0, 1, 1,
  2, 255, 1, 2, 255,
  2, 255, 2, 255,
  2, 255, 2, 255,
  2, 255, 2, 255,
  0, 1, 0, 1,
  255, 1
};