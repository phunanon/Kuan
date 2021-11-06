#include "classes.hpp"

//ARC memory management
uint8_t rc[NUM_OBJ] = {0};
onum leftmostRef = 1;
onum newRef () {
  onum ref = leftmostRef++;
  while (rc[ref] && ref++ < NUM_OBJ);
  return ref;
}

onum Value::checkMemLeak () {
  onum ref = 0;
  onum leaks = 0;
  while (ref < NUM_OBJ)
    leaks += rc[ref++];
  return leaks;
}

Value::~Value () {
  if (ref && --rc[ref])
    return;
  switch (as.tags.type) {
    case String: delete asStr(); break;
    case Vector: delete asVec(*this); break;
  }
  if (ref && ref < leftmostRef)
    leftmostRef = ref;
}

Value vecValue (immer::flex_vector<Value>* vec) {
  auto value = Value();
  value.as.tags.type = Vector;
  value.as.tags.ptr = (uintptr_t)vec;
  ++rc[value.ref = newRef()];
  return value;
}

immer::flex_vector<Value>* asVec (Value& value) {
  return (immer::flex_vector<Value>*)(uintptr_t)value.as.tags.ptr;
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