#include "stdafx.h"
#include "cToken.h"
#include "cTimeCode.h"
#include "cEdlData.h"
#include "cLine.h"
#include "math.h"

cLine::cLine(wstring line)
{
  wstringstream str(line);
  wstring token;
  do {
    getline(str,token,str.widen(' '));
    if(!token.empty())
    {
      Tokens.push_back(cToken(token));
    }
  } while(str.good());
} 

bool cLine::empty() {
  return Tokens.empty();
}

void cLine::print() {
  for(tTokenIt it = Tokens.begin(); it != Tokens.end(); ++it)
  {
    it->print();
    wcout << L" ";
  }
}

void cLine::process(cEdlData::Ptr data) {
  Data = data;
  tTokenIt it = Tokens.begin();
  if(it != Tokens.end())
  {
    if(it->isInt())
    {
      decision();
    }
    else if(it->text() == L"TITLE:")
    {
      title();
    }
    else if(it->text() == L"FCM:")
    {
      fcm();
    }
    else if(it->text() == L"*")
    {
      comment();
    }
    else if(it->text() == L"COMMENT:")
    {
      comment();
    }
    else if(it->text() == L"M2")
    {
      m2();
    }
    else if(it->text() == L"AUD")
    {
      audioTracks();
    }
    else
    {
      other();
    }
  }
}

cLine::tTokenIt cLine::editNum(tTokenIt it,wstring str) {
  int num = it->integer();
  Data->editNum(num);
  return ++it;
}

cLine::tTokenIt cLine::cassette(tTokenIt it,wstring str) {
  Data->cassette(str);
  return ++it;
}

cLine::tTokenIt cLine::channels(tTokenIt it,wstring str) {
  bool v = false;
  bool a1 = false;
  bool a2 = false;
  if(str==L"A")
  {
    a1 = true;
  }
  else if(str==L"B")
  {
    v = true;
    a1 = true;
  }
  else if(str==L"V")
  {
    v = true;
  }
  else if(str==L"A2")
  {
    a2 = true;
  }
  else if(str==L"A2/V")
  {
    v = true;
    a2 = true;
  }
  else if(str==L"AA")
  {
    a1 = true;
    a2 = true;
  }
  else if(str==L"AA/V")
  {
    v = true;
    a1 = true;
    a2 = true;
  }
  Data->channels(v,a1,a2);
  return ++it;
}

cLine::tTokenIt cLine::transition(tTokenIt it,wstring str) {
  if(str==L"C")
  {
    Data->cut();
  }
  else if(str==L"D")
  {
    ++it;
    if(it->isInt())
    {
      Data->dissolve(it->integer());
    }
  }
  else if(str[0] == L'W')
  {
    cToken t(std::wstring(str, 1));
    if(t.isInt())
    {
      int smpte = t.integer();
      bool reverse = false;
      if(smpte < 0)
      {
        smpte = abs(smpte);
        reverse = true;
      }
      ++it;
      if(it->isInt())
      {
        Data->wipe(smpte,it->integer(),reverse);
      }
    }
  }
  return ++it;
}

void cLine::decision() {
  tTokenIt it = Tokens.begin();
  if(it != Tokens.end())
  {
    it = editNum(it,it->text());
    it = cassette(it,it->text());
    it = channels(it,it->text());
    it = transition(it,it->text());
    Data->srcIn(Data->srcTimecode(it->text()));
    ++it;
    Data->srcOut(Data->srcTimecode(it->text()));
    ++it;
    Data->destIn(Data->destTimecode(it->text()));
    ++it;
    Data->destOut(Data->destTimecode(it->text()));
  }
}

void cLine::title() {
  tTokenIt it = Tokens.begin();
  ++it;
  wstringstream str;
  while(it != Tokens.end())
  {
    str << it->text() << L" ";
    ++it;
  }
  Data->title(str.str());
}

void cLine::fcm() {
  tTokenIt it = Tokens.begin();
  ++it;
  wstringstream str;
  while(it != Tokens.end())
  {
    str << it->text() << L" ";
    ++it;
  }
  Data->fcm(str.str());
}

void cLine::comment() 
{
  tTokenIt it = Tokens.begin();
  ++it;
  wstringstream str;
  while(it != Tokens.end())
  {
    str << it->text() << L" ";
    ++it;
  }
  Data->comment(str.str());
}

void cLine::m2()
{
  tTokenIt it = Tokens.begin();
  ++it;

  wstring cass = it->text();
  it++;

  float speed = it->floating();
  it++;

  cTimeCode tc = Data->srcTimecode(it->text());

  Data->m2(cass, speed, tc);
}

void cLine::audioTracks() 
{
  unsigned int audioChannelBitMask = 0;
  tTokenIt it = Tokens.begin();
  while(it != Tokens.end())
  {
    if(it->isInt())
    {
      int chan = it->integer();
      audioChannelBitMask |= 1 << (chan-1);
    }
    ++it;
  }
  Data->audioTracks(audioChannelBitMask);
}

void cLine::other() 
{
  tTokenIt it = Tokens.begin();
  wstringstream str;
  while(it != Tokens.end())
  {
    str << it->text() << L" ";
    ++it;
  }
  Data->other(str.str());
}


