#pragma once
#include <vector>
#include <memory>
#include "classes.hpp"

//Offers linear look-up of fid -> Instruction*,
//  and is the final owner of Function objects.
class FuncTable {
  vector<unique_ptr<Function>> funcs;
  bool funcIdAt (fid id, uint16_t& i) {
    for (i = 0; i < funcs.size(); ++i)
      if (funcs[i]->id == id)
        return true;
    return false;
  }
public:
  void remove (fid);
  void add (unique_ptr<Function>);
  Instruction* get (fid id) {
    uint16_t fAt;
    return funcIdAt(id, fAt) ? funcs[fAt]->ins.data() : nullptr;
  }
};


class KuanVM {
  Value stack[1024];
  argnum sp = -1; //Points to last Value on stack
  void executeFunction (fid, argnum, argnum);
  void Op_Str_V (argnum);
public:
  FuncTable functions;
  ~KuanVM ();
  Value executeFunction (fid, Value);
};