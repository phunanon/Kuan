#include "KuanVM.hpp"
#include <cmath>

void FuncTable::remove (fid id) {
  uint16_t fAt;
  if (funcIdAt(id, fAt))
    funcs.erase(funcs.begin() + fAt);
}

void FuncTable::add (unique_ptr<Function> func) {
  if (!func->ins.size() || func->ins.back().what != RETURN)
    func->ins.push_back({RETURN}); //Ensure a function returns
  uint16_t prevFunc;
  if (funcIdAt(func->id, prevFunc))
    funcs[prevFunc] = move(func);
  else
    funcs.push_back(move(func));
}


KuanVM::~KuanVM () {
  //Free the stack
  for (; sp != (onum)-1; --sp)
    stack[sp].~Value();
}

Value KuanVM::executeFunction (fid id, Value param) {
  stack[sp = 0] = param;
  executeFunction(id, 0, 1);
  return stack[sp--];
}


//Used for 2 arg + - * /
#define ARITH_2(OPERATOR) \
  stack[sp - 1].as.num OPERATOR stack[sp].as.num; \
  --sp; \
  NEXT_INSTRUCTION();

//Used for 2 arg < > <= >=
#define COMPARE_2(OPERATOR) \
  stack[sp - 1] = Value(stack[sp - 1].as.num OPERATOR stack[sp].as.num); \
  --sp; \
  NEXT_INSTRUCTION();

//Used for varadic + - * /
#define ARITH_V(OPERATOR) { \
  argnum aN = f->as.op.numArgs; \
  argnum a = (sp - (aN - 1)), aEnd = sp + 1; \
  double sum = stack[a].as.num; \
  for (++a; a < aEnd; ++a) \
    sum OPERATOR stack[a].as.num; \
  sp -= aN - 1; \
  stack[sp] = Value(sum); \
  NEXT_INSTRUCTION(); \
}

//Used for varadic < > <= >=
#define COMPARE_V(OPERATOR) { \
  argnum aN = f->as.op.numArgs; \
  argnum a = (sp - (aN - 1)), aEnd = sp + 1; \
  double prev = stack[a].as.num; \
  bool isTrue = true; \
  for (++a; a < aEnd; ++a) \
    if (!(prev OPERATOR stack[a].as.num)) { \
      isTrue = false; \
      break; \
    } else prev = stack[a].as.num; \
  sp -= aN - 1; \
  stack[sp] = Value(isTrue); \
  NEXT_INSTRUCTION(); \
}

//Used for PUSH_NUM PUSH_BOOL PUSH_CHAR PUSH_STR
#define PUSH_X(WHAT) \
  stack[++sp] = Value(WHAT); \
  NEXT_INSTRUCTION();


void KuanVM::executeFunction (fid id, argnum p0, argnum pN) {
  Instruction* f = functions.get(id);
  if (!f) return;
  
  static void* instructions[] = {
    &&ins_return, &&ins_push_num, &&ins_push_bool, &&ins_push_char, &&ins_push_str, 
    &&ins_push_para, &&ins_call, &&ins_execute, &&ins_skip, &&ins_if
  };
  static void* ops[] = {
    &&op_none_0, &&op_exe_v, &&op_inc_1, &&op_dec_1,
    &&op_pos_1, &&op_add_2, &&op_add_v, &&op_neg_1, &&op_sub_2, &&op_sub_v,
    &&op_mul_2, &&op_mul_v, &&op_div_2, &&op_div_v,
    &&op_gthan_2, &&op_gthan_v, &&op_lthan_2, &&op_lthan_v,
    &&op_geto_2, &&op_geto_v, &&op_leto_2, &&op_leto_v,
    &&op_get_str_0, &&op_get_str_1, &&op_get_num_0, &&op_get_num_1,
    &&op_vec_v, &&op_str_v, &&op_println_v
  };
  #define NEXT_INSTRUCTION() goto *instructions[(++f)->what]
  
  --f;
  NEXT_INSTRUCTION();
  ins_return:
    stack[p0] = stack[sp];
    sp = p0;
    return;
  ins_push_num:  PUSH_X(f->as.num)
  ins_push_bool: PUSH_X(f->as.tru)
  ins_push_char: PUSH_X(f->as.ch)
  ins_push_str:
    stack[++sp] = *f->as.val;
    NEXT_INSTRUCTION();
  ins_push_para: {
    argnum p = f->as.u64;
    stack[++sp] = stack[p0 + p];
    NEXT_INSTRUCTION();
  }
  ins_skip:
    f += (int)f->as.u64;
    NEXT_INSTRUCTION();
  ins_if:
    if (!stack[sp].truthy())
      f += (int)f->as.u64;
    --sp;
    NEXT_INSTRUCTION();
  ins_call: {
    argnum aN = f->as.call.numArgs;
    executeFunction(f->as.call.fID, (sp+1) - aN, aN);
    NEXT_INSTRUCTION();
  }
  ins_execute:
    goto *ops[f->as.op.what];

  op_none_0: NEXT_INSTRUCTION();
  op_exe_v:
    //TODO
    NEXT_INSTRUCTION();
  op_inc_1:
    ++stack[sp].as.num;
    NEXT_INSTRUCTION();
  op_dec_1:
    --stack[sp].as.num;
    NEXT_INSTRUCTION();
  op_pos_1:
    stack[sp].as.num = fabs(stack[sp].as.num);
    NEXT_INSTRUCTION();
  op_add_2: ARITH_2(+=)
  op_add_v: ARITH_V(+=)
  op_neg_1:
    stack[sp].as.num = -stack[sp].as.num;
    NEXT_INSTRUCTION();
  op_sub_2: ARITH_2(-=)
  op_sub_v: ARITH_V(-=)
  op_mul_2: ARITH_2(*=)
  op_mul_v: ARITH_V(*=)
  op_div_2: ARITH_2(/=)
  op_div_v: ARITH_V(/=)
  op_gthan_2: COMPARE_2(>)
  op_gthan_v: COMPARE_V(>)
  op_lthan_2: COMPARE_2(<)
  op_lthan_v: COMPARE_V(<)
  op_geto_2: COMPARE_2(>=)
  op_geto_v: COMPARE_V(>=)
  op_leto_2: COMPARE_2(<=)
  op_leto_v: COMPARE_V(<=)
  op_get_str_0: {
    stack[++sp] = Value(new string("Patrick")); //TODO
    NEXT_INSTRUCTION();
  }
  op_get_str_1: {
    //TODO
    NEXT_INSTRUCTION();
  }
  op_get_num_0: {
    //TODO
    NEXT_INSTRUCTION();
  }
  op_get_num_1: {
    //TODO
    NEXT_INSTRUCTION();
  }
  op_vec_v:
    Op_Vec_V(f->as.op.numArgs);
    NEXT_INSTRUCTION();
  op_str_v:
    Op_Str_V(f->as.op.numArgs);
    NEXT_INSTRUCTION();
  op_println_v:
    Op_Str_V(f->as.op.numArgs);
    printf("%s\n", stack[sp].asStr()->c_str());
    stack[sp] = Value();
    NEXT_INSTRUCTION();
}

void KuanVM::Op_Str_V (argnum aN) {
  auto str = new string("");
  for (argnum a = (sp - aN) + 1, aEnd = sp + 1; a < aEnd; ++a)
    *str += ValAsStr(stack[a]);
  sp -= aN - 1;
  stack[sp] = Value(str);
}

string KuanVM::ValAsStr (Value& val) {
  auto str = string("");
  if (val.isNum()) {
    auto num = to_string(val.as.num);
    auto t0 = num.find_last_not_of('0');
    num.erase(t0 + (t0 != num.find('.')), std::string::npos);
    str += num;
  } else {
    switch (val.as.tags.type) {
      case N: str += string(1, 'N'); break;
      case T: str += string(1, 'T'); break;
      case F: str += string(1, 'F'); break;
      case Character: str += string(1, val.asChar()); break;
      case String: str.append(val.asStr()->c_str()); break;
      case Vector:
        str.append("[" + to_string(asVec(val)->size()) + "]");
        break;
    }
  }
  return str;
}

void KuanVM::Op_Vec_V (argnum aN) {
  auto iVec = immer::flex_vector_transient<Value>();
  for (argnum a = sp - aN ; a < sp; ++a)
    iVec.push_back(stack[a]);
  auto vec = new immer::flex_vector<Value>(iVec.persistent());
  stack[sp -= aN - 1] = vecValue(vec);
}