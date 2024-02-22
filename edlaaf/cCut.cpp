#include "stdafx.h"
#include "cTimeCode.h"
#include "cSegment.h"
#include "cCut.h"

void cCut::print()
{
  edit();
  wcout << L" Cut       ";
  frames();
}

bool cCut::removeZeroLengthCut(cSegment::Ptr prevprev,cSegment::Ptr prev)
{
  bool removePrev = false;
  return removePrev;
}

void cCut::adjust(cSegment::Ptr prev,bool sound)
{
  // NOP
}

cSegment::Ptr cCut::clone()
{
  cCut::Ptr seg = new cCut;
  seg->set(getEditNumber(), cassette(), in(), destIn(), destOut(), speedRatio());
  return seg;
}

cUnknownPtr<IAAFComponent> cCut::createTransition(cUnknownPtr<IAAFDictionary> pDictionary,cUnknownPtr<IAAFDataDef> pDataDef)
{
  return cUnknownPtr<IAAFComponent>();
};

int cCut::getTransitionDuration() 
{
  return 0;
}