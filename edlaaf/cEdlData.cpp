#include "stdafx.h"
#include "cTimeCode.h"
#include "cEdlData.h"
#include <math.h>

cEdlData::cEdlData(float fps) 
{
  Fps = fps;
  SrcDropFrame = false;
  DestDropFrame = false;
  HadFirstFCM = false;
}

int cEdlData::getFps() 
{
  int frameRate = (int)ceil(Fps);
  return frameRate;
}

float cEdlData::getRealFps()
{
  return Fps;
}

aafRational_t cEdlData::getFrameRate()
{
  int frameRate = (int)ceil(Fps);
  int lower = (int)floor(Fps);
  if(frameRate != lower)
  {
    aafRational_t rate;
    rate.numerator = frameRate * 1000;
    rate.denominator = 1001;
    return rate;
  }
  aafRational_t rate;
  rate.numerator = frameRate * 1000;
  rate.denominator = 1000;
  return rate;
}

bool cEdlData::isSrcDropFrame() 
{
  return SrcDropFrame;
}

//utility functions

cTimeCode cEdlData::srcTimecode(wstring str) 
{
  return cTimeCode(str,(eFps)getFps(),SrcDropFrame);
}

cTimeCode cEdlData::destTimecode(wstring str) 
{
  return cTimeCode(str,(eFps)getFps(),DestDropFrame);
}

void cEdlData::fcm(wstring str)
{
  wstringstream in(str);
  wstring buffer;
  getline(in,buffer,in.widen(' '));

  if(buffer==L"DROP")
  {
    SrcDropFrame = true;
  }
  else
  {
    SrcDropFrame = false;
  }

  if(!HadFirstFCM)
  {
    DestDropFrame = SrcDropFrame;
    HadFirstFCM = true;
  }
}

