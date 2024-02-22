// edlaaf.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "cTimeCode.h"
#include "cEdlData.h"
#include "cToken.h"
#include "cLine.h"
#include "cEdl.h"
#include "cAafEdl.h"

const wchar_t* VersionString = L"21.1";
const unsigned int NumAudioChannels = NUM_AUDIO_CHANNELS;

wstring widen(const string & str) throw()
{
  const ctype<wchar_t>& char_facet = std::use_facet<std::ctype<wchar_t> >(locale());
  //const ctype<wchar_t>& char_facet = _USE(locale(),ctype<wchar_t>);
  wstring ret;
  ret.reserve(str.size());//avoid incremental allocation
  for(string::const_iterator it = str.begin(); it != str.end(); ++it)
  {
    ret += char_facet.widen(*it);
  }
  return ret;
}

int main(int argc, char* argv[])
{
  wcout << L"EDL to AAF converter. Version " << VersionString << L". Num Audio Channels = " << NumAudioChannels << endl;
  wcerr << L"EDL to AAF converter. Version " << VersionString << L". Num Audio Channels = " << NumAudioChannels << endl;

  if(argc != 3)
  {
    wcerr << L"Usage:\n\tedlaaf n file\nWhere:\n";
    wcerr << L"\tn is the edl fps (including the source material 1001 flag i.e. 30, 29.97, 25, 24, 23.98)\n";
    wcerr << L"\tfile is the edl file to convert (without the file extension)\n";
    return -1;
  }

  istringstream str(argv[1]);
  float fps = 0;
  str >> fps;

  if(fps != 30.00f &&
     fps != 29.97f &&
     fps != 25.00f &&
     fps != 24.00f &&
     fps != 23.98f)
  {
    wcerr << L"Error: Acceptable frames rates are: 30, 29.97, 25, 24 or 23.98\n";
    return -1;
  }

  string edlname(argv[2]);
  edlname += ".edl";
  wifstream in(edlname.c_str());
  if(in.fail())
  {
    wcerr << L"Error: Failed to open file " << widen(edlname.c_str()) << endl;
    return -1;
  }

  cAafEdl::Ptr data = new cAafEdl(fps);
  cEdl edl(data);

  wstring buffer;
  do {
    getline(in,buffer,in.widen('\n'));
    edl.addline(buffer);
  } while(in.good());

  edl.print();
  wcout << L"\n";

  edl.process();

  wstring aafname(widen(argv[2]));
  aafname += L".aaf";
  try
  {
    data->createAaf(aafname);
  }
  catch(HRESULT& hr)
  {
    if(AAFRESULT_FILE_EXISTS == hr)
    {
      wcerr << L"Error: " << aafname << L" already exists" << endl;
    }
    else
    {
      wcerr << L"Error: 0x" << hex << hr << L" while creating aaf file " << aafname << endl;
    }
  }

  return 0;
}


