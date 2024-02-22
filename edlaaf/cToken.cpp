#include "stdafx.h"
#include "cToken.h"

cToken::cToken(wstring token) : Token(token) 
{
}

void cToken::print()
{
  wcout << Token;
}

int cToken::integer() 
{
  wistringstream str(Token);
  int ret = INT_MIN;
  str >> ret;
  return ret;
}

bool cToken::isInt() 
{
  int val = integer();
  return val != INT_MIN && (float)val == floating();
}

float cToken::floating() 
{
  wistringstream str(Token);
  float ret = FLT_MIN;
  str >> ret;
  return ret;
}

bool cToken::isFloat() 
{
  float val = floating();
  return val != FLT_MIN && val != (float)integer();
}

wstring cToken::text() 
{
  return Token;
}
