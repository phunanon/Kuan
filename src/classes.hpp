#pragma once
#include <string>
#include <vector>
#include <memory>
#include <immer/flex_vector.hpp>
#include <immer/flex_vector_transient.hpp>
#include "config.hpp"
using namespace std;


//ARC memory management
extern uint8_t rc[NUM_OBJ];
extern onum leftmostRef;
extern onum newRef ();

#define DEFAULT_VAL (uint64_t)0xfff8000000000000 //Is nil (simple, NaN, N) by default

//A Value is either a valid double,
//or NaN and a pointer or simple data for something else.
struct __attribute__((__packed__)) Value {
  onum ref = 0; //Remains 0 for non-objects and consts
  union {
    double num;
    uint64_t u64 = DEFAULT_VAL;
    struct __attribute__((__packed__)) {
      ValType type : 3;
      uintptr_t ptr : 48;
      uint16_t qNaN : 12;
      uint8_t : 1;
    } tags;
  } as;

  Value () {}
  Value (double num) { as.num = num; }
  Value (bool b) { as.tags.type = b ? T : F; }
  Value (char ch) {
    as.tags.type = Character;
    as.tags.ptr = ch;
  }
  Value (string* str, bool isConst = false) {
    as.tags.type = String;
    as.tags.ptr = (uintptr_t)str;
    if (!isConst)
      ++rc[ref = newRef()];
  }
  bool isNum () {
    return as.tags.qNaN != 0b111111111111;
  }
  bool truthy () {
    return as.tags.type != N && as.tags.type != F;
  }
  double asNum () {
    return as.num;
  }
  bool asBool () {
    return as.tags.type == T;
  }
  char asChar () {
    return (char)as.tags.ptr;
  }
  string* asStr () {
    return (string*)(uintptr_t)as.tags.ptr;
  }
  Value (const Value& value) {
    as = value.as;
    if ((ref = value.ref))
      ++rc[ref];
  }
  Value& operator= (const Value& value) {
    as = value.as;
    if ((ref = value.ref))
      ++rc[ref];
    return *this;
  }
  ~Value ();
  static onum checkMemLeak ();
};

Value vecValue (immer::flex_vector<Value>* vec);
immer::flex_vector<Value>* asVec (Value& value);


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
    char ch;
    bool tru;
    uint64_t u64;
    Call call;
    Operation op;
    Value* obj;
  } as;
};


//Encapsulates a Function created in the Parser.
struct Function {
  fid id;
  vector<Instruction> ins;
  vector<Value> objs;
  string name = "entry";
  void mergeIn (unique_ptr<Function>);
};