#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <cinttypes>
#include <string>
using namespace std;

typedef uint16_t refnum;
typedef uint16_t inum;
typedef uint16_t argnum;
typedef uint32_t fid;

#define NUM_OBJ 255
uint8_t rc[NUM_OBJ] = {0};
refnum leftmostRef = 1;

static refnum newRef () {
  refnum ref = leftmostRef++;
  while (rc[ref] && ref++ < NUM_OBJ);
  return ref;
}

enum ObjType : uint8_t {
  String
};
enum SimpleValType : uint8_t {
  N, T, F, Character
};
enum OpType : uint8_t {
  INC_1, DEC_1, ADD_2, ADD_V, SUB_2, SUB_V, GTHAN_2,
  STR_V, GET_STR_0, PRINTLN_1
};
enum InsType : uint8_t {
  RETURN, PUSH_NUM, PUSH_CHAR, PUSH_STR, PUSH_PARA,
  CALL, EXECUTE, SKIP, IF
};


struct __attribute__((__packed__)) Object {
  ObjType type;
  union {
    string* str;
  } as;
  refnum ref = 0; //Remains 0 for consts
  Object (string* str, bool isConst = false) {
    type = String;
    as.str = str;
    if (!isConst)
      ++rc[ref = newRef()];
  }
  void ARCsub () {
    if (ref && !--rc[ref])
      free();
  }
  void free () {
    switch (type) {
      case String: delete as.str; break;
    }
    if (ref < leftmostRef)
      leftmostRef = ref;
  }
  static refnum checkMemLeak () {
    refnum ref = 0;
    refnum leaks = 0;
    while (ref < NUM_OBJ)
      leaks += rc[ref++];
    return leaks;
  }
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
  Value (double num) {
    as.num = num;
  }
  Value (SimpleValType type) {
    as.simple.type = type;
  }
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
  bool truthy () { return !as.tags.isSimple || as.simple.type != N && as.simple.type != F; }
  Object* asObj () { return (Object*)(uintptr_t)as.tags.ptr; } //TODO
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


template <class I, class T, size_t S, uint16_t L>
class HeapTable {
  I indices[L];
  T* items[L];
  uint16_t count = 0;
public:
  void add (I index, T* data, int size) {
    indices[count] = index;
    items[count] = (Instruction*)malloc(size * S);
    memcpy(items[count], data, size * S);
    ++count;
  }
  T* get (I index) {
    for (auto i = 0; i < count; ++i)
      if (indices[i] == index)
        return items[i];
    return nullptr;
  }
  ~HeapTable () {
    for (auto i = 0; i < count; ++i)
      free(items[i]);
  }
};


class KuanVM {
  Value stack[1024];
  argnum sp = -1; //Points to last Value on stack
  HeapTable<fid, Instruction, sizeof(Instruction), 255> functions;
public:
  void freeStack ();
  void addFunction (fid, Instruction*, inum);
  void executeFunction (fid, argnum, argnum);
};

void KuanVM::freeStack () {
  for (; sp != (refnum)-1; --sp)
    if (stack[sp].isObj())
      stack[sp].asObj()->free();
}

void KuanVM::addFunction (fid id, Instruction* instructions, inum length) {
  functions.add(id, instructions, length);
}

void KuanVM::executeFunction (fid id, argnum p0, argnum pN) {
  Instruction* f = functions.get(id);
  if (!f) return;
  
  static void* instructions[] = {
    &&ins_halt, &&ins_push_num, &&ins_push_char, &&ins_push_str, &&ins_push_para,
    &&ins_call, &&ins_execute, &&ins_skip, &&ins_if
  };
  static void* ops[] = {
    &&op_inc_1, &&op_dec_1, &&op_add_2, &&op_add_v, &&op_sub_2, &&op_sub_v,
    &&op_gthan_2, &&op_str_v, &&op_get_str_0, &&op_println_1
  };
  #define NEXT_INSTRUCTION() goto *instructions[(++f)->what]
  
  --f;
  NEXT_INSTRUCTION();
  while (true) {
    ins_halt:
//printf(" RETURN");
      stack[p0] = stack[sp];
      sp = p0;
      break;
    ins_push_num:
//printf(" PUSH_NUM");
      stack[++sp] = Value(f->as.num);
      NEXT_INSTRUCTION();
    ins_push_char: {
//printf(" PUSH_CHAR");
      stack[++sp] = Value((char)f->as.num);
      NEXT_INSTRUCTION();
    }
    ins_push_str: {
//printf(" PUSH_STR");
      stack[++sp] = Value(f->as.obj);
      NEXT_INSTRUCTION();
    }
    ins_push_para: {
//printf(" PUSH_PARA");
      argnum p = f->asInt();
      stack[++sp] = p < pN ? stack[p0 + p] : Value();
      NEXT_INSTRUCTION();
    }
    ins_skip:
//printf(" SKIP");
      f += (int)f->as.num;
      NEXT_INSTRUCTION();
    ins_if:
//printf(" IF");
      if (!stack[sp].truthy())
        f += (int)f->as.num;
      --sp;
      NEXT_INSTRUCTION();
    ins_call: {
//printf(" CALL");
      argnum aN = f->as.call.numArgs;
      executeFunction(f->as.call.fID, (sp+1) - aN, aN);
      NEXT_INSTRUCTION();
    }
    ins_execute:
//printf(" EXECUTE");
      goto *ops[f->as.op.what];

    op_inc_1:
      ++stack[sp].as.num;
      NEXT_INSTRUCTION();
    op_dec_1:
      --stack[sp].as.num;
      NEXT_INSTRUCTION();
    op_add_2:
      stack[sp - 1].as.num += stack[sp].as.num;
      --sp;
      NEXT_INSTRUCTION();
    op_add_v:
      //TODO
      NEXT_INSTRUCTION();
    op_sub_2:
      stack[sp - 1].as.num -= stack[sp].as.num;
      --sp;
      NEXT_INSTRUCTION();
    op_sub_v:
      //TODO
      NEXT_INSTRUCTION();
    op_gthan_2:
      stack[sp - 1] = Value(stack[sp - 1].as.num < stack[sp].as.num ? T : F);
      --sp;
      NEXT_INSTRUCTION();
    op_str_v: {
//printf(" STR_V");
      auto str = new string("");
      argnum aN = f->as.op.numArgs;
      for (argnum a = (sp - aN) + 1, aEnd = sp + 1; a < aEnd; ++a) {
        if (stack[a].isNum())
          *str += to_string(stack[a].as.num);
        else {
          if (stack[a].as.tags.isSimple)
            switch (stack[a].as.simple.type) {
              case N: *str += string(1, 'N'); break;
              case T: *str += string(1, 'T'); break;
              case F: *str += string(1, 'F'); break;
              case Character: *str += string(1, (char)stack[a].as.simple.s08); break;
            }
          else {
            switch (stack[a].asObj()->type) {
              case String: str->append(stack[a].asObj()->as.str->c_str()); break;
            }
            stack[a].asObj()->ARCsub();
          }
        }
      }
      sp -= aN - 1;
      stack[sp] = Value(new Object(str));
      NEXT_INSTRUCTION();
    }
    op_get_str_0: {
      stack[++sp] = Value(new Object(new string("Patrick")));
      NEXT_INSTRUCTION();
    }
    op_println_1: {
      printf("%s\n", stack[sp].asObj()->as.str->c_str());
      stack[sp].asObj()->ARCsub();
      stack[sp] = Value();
      NEXT_INSTRUCTION();
    }
  }
}

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
  vm.addFunction(0, function0, sizeof(function0)/sizeof(Instruction));
  //vm.addFunction(1, function1, sizeof(function1)/sizeof(Instruction));
  vm.executeFunction(0, 0, 0);
  vm.freeStack();
  hello->free();
  
  printf("\nDone.\n");
  if (auto leaks = Object::checkMemLeak())
    printf("Warning: %d ARC memory leak/s detected.\n", leaks);
}
