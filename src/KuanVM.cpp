#include "KuanVM.hpp"

template <int L>
FuncTable<L>::~FuncTable ()  {
  for (auto i = 0; i < count; ++i) {
    free(ins[i]);
    for (auto o : objs[i])
      delete o;
  }
}

KuanVM::~KuanVM () {
  //Free the stack
  for (; sp != (onum)-1; --sp)
    if (stack[sp].isObj())
      delete stack[sp].asObj();
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