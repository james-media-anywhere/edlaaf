#include "stdafx.h"
#include "cTimeCode.h"
#include "cEdlData.h"
#include "cAafEdl.h"
#include "cDisolve.h"
#include "cCut.h"

cDisolve::cDisolve(int duration) 
 : Duration(duration) 
{
}

void cDisolve::print()
{
  edit();
  wcout << L" Dis   " << setw(3) << Duration << L" ";
  frames();
}

cUnknownPtr<IAAFComponent> cDisolve::createTransition(cUnknownPtr<IAAFDictionary> pDictionary,cUnknownPtr<IAAFDataDef> pDataDef)
{
  cUnknownPtr<IAAFComponent> pComponent;
  if(Duration > 0 && length() > 0)//ignore zero length disolves
  {
    //transition
    aafBoolean_t IsSound;
    checkResult(pDataDef->IsSoundKind(&IsSound));

    cUnknownPtr<IAAFOperationDef> pOperationDef;
    if(!IsSound)
    {
      GetDisolve(pDictionary,pDataDef,kAAFEffectVideoDissolve,&pOperationDef);
    }
    else
    {
      GetDisolve(pDictionary,pDataDef,kAAFEffectMonoAudioDissolve,&pOperationDef);
    }

    cUnknownPtr<IAAFOperationGroup> pOperationGroup;
    checkResult(pDictionary->CreateInstance(AUID_AAFOperationGroup,IID_IAAFOperationGroup,(IUnknown **)&pOperationGroup));
    checkResult(pOperationGroup->Initialize(pDataDef,Duration,pOperationDef));

    cUnknownPtr<IAAFTransition> pTransition;
    checkResult(pDictionary->CreateInstance(AUID_AAFTransition,IID_IAAFTransition,(IUnknown **)&pTransition));

    checkResult(pTransition->Initialize(pDataDef,Duration,0,pOperationGroup));
    pTransition.demandQI(IID_IAAFComponent, (void **)&pComponent);
    setEditNumber(pDictionary,pComponent,getEditNumber());
  }
  return pComponent;
}


bool cDisolve::removeZeroLengthCut(cSegment::Ptr prevprev,cSegment::Ptr prev)
{
  // remove zero length cut (if any) preceding transition 
  // if zero length cut (prev) matches segment preceding it (prevprev)

  bool removePrev = false;

  if (prev->length() != 0) return removePrev; // must be zero length

  if (!dynamic_cast<cCut::Ptr>(prev)) return removePrev; // must be cut

  // check tape names match
  wstring prevCassette = prev->cassette();
  wstring prevprevCassette = prevprev->cassette();
  if(MERGE_B_ROLLS && !prevCassette.empty())
  {
    wstring::iterator it = prevCassette.end();
    --it;
    if(*it == L'B') prevCassette.erase(it,prevCassette.end()); // remove 'B' if B roll
  }
  if(MERGE_B_ROLLS && !prevprevCassette.empty())
  {
    wstring::iterator it = prevprevCassette.end();
    --it;
    if(*it == L'B') prevprevCassette.erase(it,prevprevCassette.end()); // remove 'B' if B roll
  }
  if (prevCassette != prevprevCassette) return removePrev;

  // check speed ratios match
  aafRational_t prevSpeed = prev->speedRatio();
  aafRational_t prevprevSpeed = prevprev->speedRatio();
  if ((prevSpeed.numerator * prevprevSpeed.denominator) != (prevprevSpeed.numerator * prevSpeed.denominator)) return removePrev;

  // check prev source tc falls within prevprev segment
  cTimeCode prevSrcTc = prev->in();
  cTimeCode prevprevSrcIn = prevprev->in();
  cTimeCode prevprevSrcOut = prevprev->out();
  if ((prevSrcTc < prevprevSrcIn) || (prevSrcTc > prevprevSrcOut)) return removePrev;

  removePrev = true; // indicates that prev segment should be removed
  return removePrev;
}

void cDisolve::adjust(cSegment::Ptr prev,bool sound)
{
  prev->adjustDestOut(Duration);

  if(sound)
  {
    prev->setOutFade(Duration);
  }
}

cSegment::Ptr cDisolve::clone()
{
  cDisolve::Ptr seg = new cDisolve(Duration);
  seg->set(getEditNumber(), cassette(), in(), destIn(), destOut(), speedRatio());
  return seg;
}

int cDisolve::getTransitionDuration()
{
  return Duration;
}
