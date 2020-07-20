#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <cinttypes>

typedef uint16_t refnum;
typedef uint16_t inum;
typedef uint16_t argnum;
typedef uint32_t fid;

enum ObjType : uint8_t {
  STRING
};
struct __attribute__((__packed__)) Object {
  ObjType type;
  refnum ref;
};

enum ValType : uint8_t {
  N, T, F, Number, String
};
struct __attribute__((__packed__)) Value {
  ValType type;
  union {
    double num;
    Object obj;
  } as;
  bool truthy () { return type != N && type != F; }
};


enum OpType : uint8_t {
  INC_1, DEC_1, ADD_2, ADD_V, SUB_2, SUB_V, GTHAN_2,
  PRINTLN_V
} what;
struct __attribute__((__packed__)) Operation {
  OpType what;
  uint8_t numArgs;
};

struct __attribute__((__packed__)) Call {
  fid fID;
  uint8_t numArgs;
};

enum InsType : uint8_t {
  RETURN, PUSH_NUM, PUSH_PARA, CALL, EXECUTE, SKIP, IF
};
struct __attribute__((__packed__)) Instruction {
  InsType what;
  union {
    double num;
    Call call;
    Operation op;
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
  void addFunction (fid, Instruction*, inum);
  void executeFunction (fid, argnum, argnum);
};

void KuanVM::addFunction (fid id, Instruction* instructions, inum length) {
  functions.add(id, instructions, length);
}

void KuanVM::executeFunction (fid id, argnum p0, argnum pN) {
  Instruction* f = functions.get(id);
  if (!f) return;
  
  static void* instructions[] = {
    &&ins_halt, &&ins_push_num, &&ins_push_para, &&ins_call, &&ins_execute,
    &&ins_skip, &&ins_if
  };
  static void* ops[] = {
    &&op_inc_1, &&op_dec_1, &&op_add_2, &&op_add_v, &&op_sub_2, &&op_sub_v,
    &&op_gthan_2, &&op_println_v
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
      stack[++sp] = {Number, {.num = f->as.num}};
      NEXT_INSTRUCTION();
    ins_push_para: {
//printf(" PUSH_PARA");
      argnum p = f->asInt();
      stack[++sp] = p < pN ? stack[p0 + p] : Value{N};
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
      stack[sp - 1] = Value{stack[sp - 1].as.num < stack[sp].as.num ? T : F};
      --sp;
      NEXT_INSTRUCTION();
    op_println_v: {
      argnum aN = f->as.op.numArgs;
      printf("%f\n", stack[sp].as.num); //TODO _V
      stack[sp -= aN - 1] = Value{};
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

  //(println (fib 5))
  Instruction function0[] {
    {PUSH_NUM, 35},
    {CALL, {.call = {1, 1}}},
    {EXECUTE, {.op = {PRINTLN_V, 1}}}
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
  auto vm = KuanVM();
  vm.addFunction(0, function0, sizeof(function0)/sizeof(Instruction));
  vm.addFunction(1, function1, sizeof(function1)/sizeof(Instruction));
  vm.executeFunction(0, 0, 0);
  
  printf("\nDone.\n");
}
