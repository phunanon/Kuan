#include "KuanVM.hpp"

KuanVM::~KuanVM () {
  //Free the stack
  for (; sp != (onum)-1; --sp)
    if (stack[sp].isObj())
      delete stack[sp].asObj();
}

Value KuanVM::executeFunction (fid id, Value param) {
  stack[sp = 0] = param;
  executeFunction(id, 0, 1);
  return stack[sp--];
}



//Used for varadic + - * /
#define ARITH_V(OPERATOR) { \
  double sum = stack[sp].as.num; \
  argnum aN = f->as.op.numArgs; \
  for (argnum a = (sp - aN) + 2, aEnd = sp + 1; a < aEnd; ++a) \
    sum OPERATOR stack[a].as.num; \
  sp -= aN - 1; \
  stack[sp] = Value(sum); \
  NEXT_INSTRUCTION(); \
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

//Used for varadic < > <= >=
#define COMPARE_V(OPERATOR) { \
  double prev = stack[sp].as.num; \
  argnum aN = f->as.op.numArgs; \
  bool isTrue = true; \
  for (argnum a = (sp - aN) + 2, aEnd = sp + 1; a < aEnd; ++a) \
    if (!(prev OPERATOR stack[a].as.num)) { \
      isTrue = false; \
      break; \
    } \
  sp -= aN - 1; \
  stack[sp] = Value(isTrue); \
  NEXT_INSTRUCTION(); \
}


void KuanVM::executeFunction (fid id, argnum p0, argnum pN) {
  Instruction* f = functions.get(id);
  if (!f) return;
  
  static void* instructions[] = {
    &&ins_halt, &&ins_push_num, &&ins_push_char, &&ins_push_str, &&ins_push_para,
    &&ins_call, &&ins_execute, &&ins_skip, &&ins_if
  };
  static void* ops[] = {
    &&op_none_0, &&op_exe_v, &&op_inc_1, &&op_dec_1,
    &&op_add_2, &&op_add_v, &&op_neg_1, &&op_sub_2, &&op_sub_v,
    &&op_mul_2, &&op_mul_v, &&op_div_2, &&op_div_v,
    &&op_gthan_2, &&op_gthan_v, &&op_lthan_2, &&op_lthan_v,
    &&op_geto_2, &&op_geto_v, &&op_leto_2, &&op_leto_v,
    &&op_get_str_0, &&op_get_str_1, &&op_get_num_0, &&op_get_num_1,
    &&op_str_v, &&op_println_1
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

    op_none_0:
      NEXT_INSTRUCTION();
    op_exe_v:
      //TODO
      NEXT_INSTRUCTION();
    op_inc_1:
      ++stack[sp].as.num;
      NEXT_INSTRUCTION();
    op_dec_1:
      --stack[sp].as.num;
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
      stack[++sp] = Value(new Object(new string("Patrick"))); //TODO
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
    op_str_v: {
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
    op_println_1: {
      printf("%s\n", stack[sp].asObj()->as.str->c_str());
      stack[sp].asObj()->ARCsub();
      stack[sp] = Value();
      NEXT_INSTRUCTION();
    }
  }
}