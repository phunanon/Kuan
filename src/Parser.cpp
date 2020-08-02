#include "Parser.hpp"
#include <cstring>
#include <cstdlib>
#include <functional>
#include <queue>
#include <algorithm>
using namespace std;

bool isWhite (char c) {
  return c == ' ' || c == '\n';
}

bool isNumber (const string& s) {
  if (s.substr(0, 2) == "0x")
    return strspn(s.c_str(), "-.0123456789x") == s.size();
  return strspn(s.c_str(), "-.0123456789") == s.size();
}


//Remove extraneous space such as
//  double spaces, spaces before or after parens,
//  single-line comments, multi-line comments
string removeExtraneousSpace (const string& input) {
  string output = "";
  char prev = ' ';
  bool inString = false;
  bool inSComment = false, inMComment = false;
  for (uint16_t i = 0, iLen = input.length(); i < iLen; ++i) {
    char c = input[i];
    char next = i+1 == iLen ? '\0' : input[i+1];
    bool inComment = inSComment || inMComment;
    //Handle entering/leaving string
    if (!inComment && c == '"' && prev != '\\')
      inString = !inString;
    //Handle comments
    if (!inString) {
      if (!inComment && c == '/') {
        inSComment = next == '/';
        inMComment = next == '*';
      }
      if ((inSComment && c == '\n')
       || (inMComment && c == '*' && next == '/')) {
        i += inMComment ? 2 : 1;
        inSComment = inMComment = false;
        continue;
      }
      if (inSComment || inMComment)
        continue;
    }
    //Handle extraneous spaces
    if (!inString && isWhite(c)) {
      bool doubleWhite       = isWhite(prev);
      bool whiteAfterOpen    = prev == '(' || prev == '[';
      bool whiteBeforeClose  = next == ')' || next == ']';
      if (doubleWhite || whiteAfterOpen || whiteBeforeClose)
        continue;
    }
    if (c == '\n') c = ' ';
    output += (prev = c);
  }
  return output;
}

struct Token {
  enum {
    Unknown,
    LParen, RParen, LSquare, RSquare,
    Hash, Period, Para,
    Char, Number, String, Symbol
  } type;
  string str;
};

vector<Token> tokenise (const string& input) {
  auto tokens = vector<Token>();
  for (uint16_t i = 0, iLen = input.length(); i < iLen; ++i) {
    char c = input[i];
    //Handle # ( ) [ ]
    {
      auto type = Token::Unknown;
      switch (c) {
        case '#': type = Token::Hash;    break;
        case '(': type = Token::LParen;  break;
        case ')': type = Token::RParen;  break;
        case '[': type = Token::LSquare; break;
        case ']': type = Token::RSquare; break;
      }
      if (type != Token::Unknown) {
        tokens.push_back(Token{type, string(1, c)});
        continue;
      }
    }
    //Handle string
    if (c == '"') {
      auto end = input.find('"', ++i);
      auto str = string(&input[i], &input[end]);
      tokens.push_back(Token{Token::String, str});
      i += str.length();
      continue;
    }
    //Skip spaces
    if (c == ' ' || c == '\n')
      continue;
    //Collect next string before
    //  space newline " ( ) [ ]
    auto nextDelim = input.find_first_of(" \n\"()[]", i);
    if (nextDelim == string::npos)
      nextDelim = input.length();
    string next = string(&input[i], &input[nextDelim]);
    //Handle character, number, parameter, period, or other symbol
    {
      auto type = Token::Symbol;
      if (c == '\\')
        type = Token::Char;
      else if (c == '%')
        type = Token::Para;
      else if (isNumber(next) && !(next.length() == 1 && c == '-'))
        type = Token::Number;
      else if (c == '.')
        type = Token::Period;
      tokens.push_back(Token{type, next});
    }
    i += next.length() - 1;
  }
  return tokens;
}


//Separate all tokens by the highest level of parenthesis
vector<vector<Token>> separate (vector<Token> tokens) {
  auto funcs = vector<vector<Token>>();
  uint8_t depth = 0;
  for (auto t : tokens) {
    if (!depth) funcs.push_back(vector<Token>());
    funcs.back().push_back(t);
    if (t.type == Token::LParen || t.type == Token::LSquare) ++depth;
    if (t.type == Token::RParen || t.type == Token::RSquare) --depth;
  }
  return funcs;
}


//Accepts a symbol and arity, where arity 255 is varadic
OpType symToOp (const char* symbol, uint8_t arity) {
  for (uint8_t o = 1; opSymbols[o]; ++o)
    if (!strcmp(symbol, opSymbols[o]))
      if (opArities[o] == arity || opArities[o] == 255)
        return (OpType)o;
  return NONE_0;
}


//Take a vector of tokens and parameters,
//  which constitutes one form,
//  and return a vector of Instruction
void serialise (deque<Token>& tokens, Function* func, vector<string> paras) {
  //Check if (op ...) or ((...) ...)
  //  so ((...) ...) becomes (exe (...) ...)
  Token opToken = tokens.front();
  tokens.pop_front();
  if (opToken.type == Token::LParen) {
    opToken = Token{Token::Symbol, "exe"};
    serialise(tokens, func, paras);
  }
  //Collect arguments until the closing parenthesis
  bool isIf = opToken.str == "if";
  inum ifAt, skipAt;
  uint8_t arity = 0;
  while (tokens[0].type != Token::RParen) {
    ++arity;
    auto token = tokens.front();
    tokens.pop_front();
    switch (token.type) {
      case Token::LParen:
        serialise(tokens, func, paras);
        break;
      case Token::Number: {
        bool isNeg  = token.str[0] == '-';
        bool preDot = token.str[isNeg] == '.';
        bool isHex  = token.str.find('x') != string::npos;
        token.str = token.str.substr(preDot ? isNeg + preDot : 0);
        func->ins.push_back({PUSH_NUM, {.num = stod(token.str)}});
        break;
      }
      case Token::Char: {
        //FIXME with nl sp
        func->ins.push_back({PUSH_CHAR, {.ch = token.str[1]}});
        break;
      }
      case Token::String: {
        auto obj = new Object(new string(token.str), true);
        func->ins.push_back(Instruction{PUSH_STR, {.obj = obj}});
        break;
      }
      case Token::Symbol: {
        //A symbol is
        //  a bool, (nil, variable, op, parameter, or function)
        char ch = token.str[0];
        //True/False
        if (ch == 'T' || ch == 'F') {
          func->ins.push_back(Instruction{PUSH_BOOL, {.tru = ch == 'T'}});
          break;
        }
      }
    }
    //If this is a special form, insert skip's in appropriate positions
    if (isIf) {
      if (arity == 1) { //(if cond ...)
        ifAt = func->ins.size();
        func->ins.push_back(Instruction{IF, 0});
      } else if (arity == 2) { //(if cond if-true ...)
        skipAt = func->ins.size();
        func->ins.push_back(Instruction{SKIP, 0});
      }
    }
  }
  //Pop right paren
  tokens.pop_front();
  //If a special form, handle differently
  if (isIf) {
    func->ins.at(ifAt).as.u32 = skipAt - ifAt;
    func->ins.at(skipAt).as.u32 = (func->ins.size() - skipAt) - 1;
    return;
  }
  //Append operation instruction
  OpType opType = symToOp(opToken.str.c_str(), arity);
  func->ins.push_back(Instruction{EXECUTE, {.op = {opType, arity}}});
}


//Take a vector of tokens,
//  either a function declaration or entry form,
//  and return a vector of Instructions for the function
//fid will either be 0 for an entry form or the hashed function name
unique_ptr<Function> serialise (vector<Token> form) {
  auto func = make_unique<Function>();
  auto paras = vector<string>();
  //Check if this is a function declaration
  //  or part of the entry function
  if (form.size() > 1 && form[1].str == "fn") {
    func->id = hash<string>{}(form[2].str);
    //Collect param symbols
    argnum t = 4;
    //TODO: destructuring goes here
    for (; form[t].type != Token::RSquare; ++t)
      paras.push_back(form[t].str);
    form = vector<Token>(&form[t+1], &form.back());
  }
  //Serialise all function forms, or the one entry form
  {
    auto formTokens = deque<Token>();
    uint8_t depth = 0;
    for (auto t : form) {
      formTokens.push_back(t);
      if (t.type == Token::LParen || t.type == Token::LSquare) ++depth;
      if (t.type == Token::RParen || t.type == Token::RSquare) --depth;
      if (!depth) {
        if (formTokens[0].type == Token::LParen)
          formTokens.pop_front(); //Pop first paren
        serialise(formTokens, func.get(), paras);
      }
    }
  }
  return func;
}


//Parses a string source into a vector of Instructions per function
//  with fid 0 as the entry Instructions
vector<unique_ptr<Function>> Parser::parse (string source) {
  auto noExtraneousSpace = removeExtraneousSpace(source);
  auto tokens = tokenise(noExtraneousSpace);
//for (auto t : tokens) printf("%d %s\t", t.type, t.str.c_str());
//printf("\n");
//return {};
  auto separatedTokens = separate(tokens);
  auto funcs = vector<unique_ptr<Function>>();
  //Add an entry function
  funcs.push_back(make_unique<Function>());
  //Serialise all functions/entry forms
  for (auto tokens : separatedTokens) {
    auto func = serialise(tokens);
    //If this is a non-entry function, insert it as new
    //  otherwise insert it into the entry function's vector
    if (func->id)
      funcs.push_back(move(func));
    else
      funcs[0]->mergeIn(move(func));
  }
  //Ensure the entry function returns a string for potential REPL output
  {
    auto last = funcs[0]->ins.back();
    if (last.what != EXECUTE || last.as.op.what != STR_V)
      funcs[0]->ins.push_back(Instruction{EXECUTE, {.op = {STR_V, 1}}});
  }
  return funcs;
}