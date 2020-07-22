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
    if (!strcmp(symbol, opSymbols[o]) && opArities[o] == arity)
      return (OpType)o;
  return NONE_0;
}


//Take a vector of tokens and parameters,
//  which constitutes one form,
//  and return a vector of Instruction
vector<Instruction> serialise (deque<Token> &tokens, vector<string> paras) {
  return {};
}


//Take a vector of tokens,
//  either a function declaration or entry form,
//  and return a vector of Instructions for the function
//fid will either be 0 for an entry form or the hashed function name
pair<fid, vector<Instruction>> serialise (vector<Token> form) {
 return {};
}


//Parses a string source into a vector of Instructions per function
//  with fid 0 as the entry Instructions
map<fid, vector<Instruction>> Parser::parse (string source) {
  auto noExtraneousSpace = removeExtraneousSpace(source);
  auto tokens = tokenise(noExtraneousSpace);
//for (auto t : tokens) printf("%d %s\t", t.type, t.str.c_str());
//printf("\n");
//return {};
  auto separatedTokens = separate(tokens);
  auto funcs = map<fid, vector<Instruction>>();
  for (auto tokens : separatedTokens) {
    auto instructs = serialise(tokens);
    //If this is a non-entry function, insert it as new
    //  otherwise insert it into the entry function's vector
    if (instructs.first)
      funcs.insert(instructs);
    else
      if (instructs.second.size())
        funcs[0].insert(funcs[0].end(), instructs.second.begin(), instructs.second.end());
  }
  return funcs;
}