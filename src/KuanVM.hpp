#pragma once
#include <vector>
#include <memory>
#include "classes.hpp"

//Offers linear look-up of fid -> Instruction*,
//  and is the owner of Function objects.
class FuncTable {
  vector<unique_ptr<Function>> funcs;
  bool funcIdAt (fid id, uint16_t& i) {
    for (i = 0; i < funcs.size(); ++i)
      if (funcs[i]->id == id)
        return true;
    return false;
  }
public:
  void remove (fid id) {
    uint16_t fAt;
    if (funcIdAt(id, fAt))
      funcs.erase(funcs.begin() + fAt);
  }
  void add (unique_ptr<Function> func) {
    uint16_t prevFunc;
    if (funcIdAt(func->id, prevFunc))
      funcs[prevFunc] = move(func);
    else
      funcs.push_back(move(func));
  }
  Instruction* get (fid id) {
    uint16_t fAt;
    return funcIdAt(id, fAt) ? funcs[fAt]->ins.data() : nullptr;
  }
};


class KuanVM {
  Value stack[1024];
  argnum sp = -1; //Points to last Value on stack
  void executeFunction (fid, argnum, argnum);
public:
  FuncTable functions;
  ~KuanVM ();
  Value executeFunction (fid, Value);
};