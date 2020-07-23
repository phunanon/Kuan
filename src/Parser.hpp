#pragma once
#include <vector>
#include <memory>
#include "classes.hpp"

struct Parser {
  static vector<unique_ptr<Function>> parse (string);
};