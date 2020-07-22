#pragma once
#include <vector>
#include <map>
#include "classes.hpp"

struct Parser {
  static map<fid, vector<Instruction>> parse (string);
};