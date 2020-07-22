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