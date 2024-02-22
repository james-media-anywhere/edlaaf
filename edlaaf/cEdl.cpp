#include "stdafx.h"
#include "cTimeCode.h"
#include "cEdlData.h"
#include "cToken.h"
#include "cLine.h"
#include "cEdl.h"

cEdl::cEdl(cEdlData::Ptr data) : Data(data)
{
}

void cEdl::addline(wstring str)
{
  cLine line(str);
  if(!line.empty())
  {
    Lines.push_back(line);
  }
}

void cEdl::print() 
{
  for(tLineIt it = Lines.begin(); it != Lines.end(); ++it)
  {  
    it->print();
    wcout << L"\n";
  }
}

void cEdl::process() 
{
  Data->begin();
  for(tLineIt it = Lines.begin(); it != Lines.end(); ++it)
  {  
    it->process(Data);
  }
  Data->end();
}

