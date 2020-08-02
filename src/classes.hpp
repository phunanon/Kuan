#pragma once
#include <string>
#include <vector>
#include <memory>
#include "config.hpp"
using namespace std;


//ARC memory management
extern uint8_t rc[NUM_OBJ];
extern onum leftmostRef;
extern onum newRef ();


struct __attribute__((__packed__)) Object {
  ObjType type;
  union {
    string* str;
  } as;
  onum ref = 0; //Remains 0 for consts
  Object (string* str, bool isConst = false) {
    type = String;
    as.str = str;
    if (!isConst)
      ++rc[ref = newRef()];
  }
  Object (const Object& o) {
    type = o.type;
    as = o.as;
    if ((ref = o.ref))
      ++rc[ref];
  }
  Object& operator= (const Object& o) {
    type = o.type;
    as = o.as;
    if ((ref = o.ref))
      ++rc[ref];
    return *this;
  }
  ~Object () {
    if (ref && --rc[ref])
      return;
    switch (type) {
      case String: delete as.str; break;
    }
    if (ref && ref < leftmostRef)
      leftmostRef = ref;
  }
  static onum checkMemLeak ();
};

//A Value is either a valid double, or a NaN non-simple Object*, or a NaN simple
struct __attribute__((__packed__)) Value {
  union {
    double num;
    uint64_t u64 = (uint64_t)0xfff8000000000000; //Is nil (simple, NaN, N) by default
    struct __attribute__((__packed__)) {
      uint8_t : 3;
      uintptr_t ptr : 48;
      uint16_t qNaN : 12;
      uint8_t isSimple : 1;
    } tags;
    struct __attribute__((__packed__)) {
      SimpleValType type : 8;
      char s08 : 8;
      uint64_t : 40;
    } simple;
  } as;
  Value () {}
  Value (double num) { as.num = num; }
  Value (bool b) { as.simple.type = b ? T : F; }
  Value (SimpleValType type) { as.simple.type = type; }
  Value (char ch) {
    as.simple.type = Character;
    as.simple.s08 = ch;
  }
  Value (Object* obj) {
    as.tags.isSimple = false;
    as.tags.ptr = (uintptr_t)obj;
  }
  bool isNum () { return as.tags.qNaN != 0b111111111111; }
  bool isObj () { return !isNum() && !as.tags.isSimple; }
  bool truthy () { return !as.tags.isSimple || (as.simple.type != N && as.simple.type != F); }
  Object* asObj () { return (Object*)(uintptr_t)as.tags.ptr; }
  void tryDelete () { if (isObj()) delete asObj(); }
};


struct __attribute__((__packed__)) Operation {
  OpType what;
  uint8_t numArgs;
};

struct __attribute__((__packed__)) Call {
  fid fID;
  uint8_t numArgs;
};

struct __attribute__((__packed__)) Instruction {
  InsType what;
  union {
    double num;
    Call call;
    Operation op;
    Object* obj;
  } as;
  uint64_t asInt () { return as.num; }
};


//Encapsulates a Function created in the Parser.
//  It owns (frees) the Object pointers.
struct Function {
  fid id;
  vector<Instruction> ins;
  vector<Object*> objs;
  ~Function ();
  void mergeIn (unique_ptr<Function>);
};