#pragma once
#include <vector>
#include "classes.hpp"

//Offers linear look-up of fid -> Instruction*,
//  and is the owner of func const heap objects
template <int L>
class FuncTable {
  fid ids[L];
  Instruction* ins[L];
  vector<Object*> objs[L];
  uint16_t count = 0;
public:
  void add (fid id, Instruction* data, inum numIns, vector<Object*> fObjs) {
    ids[count] = id;
    ins[count] = (Instruction*)malloc(numIns * sizeof(Instruction));
    memcpy(ins[count], data, numIns * sizeof(Instruction));
    objs[count] = fObjs;
    ++count;
  }
  Instruction* get (fid id) {
    for (auto i = 0; i < count; ++i)
      if (ids[i] == id)
        return ins[i];
    return nullptr;
  }
  ~FuncTable ();
};


class KuanVM {
  Value stack[1024];
  argnum sp = -1; //Points to last Value on stack
public:
  FuncTable<255> functions;
  ~KuanVM ();
  void executeFunction (fid, argnum, argnum);
};