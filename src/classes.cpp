#include "classes.hpp"

//ARC memory management
uint8_t rc[NUM_OBJ] = {0};
onum leftmostRef = 1;
onum newRef () {
  onum ref = leftmostRef++;
  while (rc[ref] && ref++ < NUM_OBJ);
  return ref;
}

onum Object::checkMemLeak () {
  onum ref = 0;
  onum leaks = 0;
  while (ref < NUM_OBJ)
    leaks += rc[ref++];
  return leaks;
}

Function::~Function () {
  for (auto o : objs)
    delete o;
}

//Merges two functions together,
//  retaining the id of the target function,
//  linearly appending the ins of the source into the target,
//  and invalidating the objects of the source
void Function::mergeIn (unique_ptr<Function> source) {
  ins.insert(ins.end(), source->ins.begin(), source->ins.end());
  objs.insert(objs.end(), source->objs.begin(), source->objs.end());
  source->objs.clear();
}