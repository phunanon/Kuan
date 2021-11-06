#include "classes.hpp"

//ARC memory management
uint8_t rc[NUM_OBJ] = {0};
onum leftmostRef = 1; //0 is reserved for consts
onum newRef () {
  onum ref = leftmostRef++;
  while (rc[ref] && ref++ < NUM_OBJ); //TODO: handle memory overflow
  return ref;
}

void Value::tryDelete () {
  if (isObj() && obj()->ref) {
    auto ref = obj()->ref;
    delete obj();
    as.tags.isSimple = !rc[ref];
  }
}

Value Value::copy () {
  Value newValue {as};
  if (isObj() && obj()->ref)
    ++rc[obj()->ref];
  return newValue;
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