#include "stdafx.h"
#include "cTimeCode.h"
#include "cTrack.h"
#include "cCut.h"

//this function deals with insert mode etc.
void cTimelineTrack::add(cSegment::Ptr seg)
{
  cSegment::Ptr segmentToInsert = seg->clone();

  cTimeCode InPoint = segmentToInsert->destIn();
  cTimeCode OutPoint = segmentToInsert->destIn();

  cTimeCode lastOut;
  if(Segments.empty())
  {
    lastOut = TimeIn;
  }
  else
  {
    cSegment::Ptr back = Segments.back();
    lastOut = back->destOut();
  }

  if(lastOut == InPoint)
  {
    Segments.push_back(segmentToInsert);
    return;
  }

  if(lastOut < InPoint)
  {
    //There is a gap between the segments
    cSegment::Ptr blankSeg = new cCut;

    aafRational_t speed;
    speed.numerator = 1000;
    speed.denominator = 1000;
    
    blankSeg->set(0,L"BL",lastOut,lastOut,InPoint,speed);
    Segments.push_back(blankSeg);

    Segments.push_back(segmentToInsert);
    return;
  }

  //if we get here -> lastOut > InPoint
  //therefore we have some sort of overlap

  if(Segments.empty())//sanity check !!!
  {
    //duh ??? what does this mean ??? this should never happen
    //logically TimeIn should equal InPoint, lets warn and continue 
    wcerr << L"Warning: Logic error in track construction. Edit Num = " << segmentToInsert->getEditNumber() << endl;
    wcout << L"Warning: Logic error in track construction. Edit Num = " << segmentToInsert->getEditNumber() << endl;

    Segments.push_back(segmentToInsert);
    return;
  }

  insertSegment(segmentToInsert);
}

cSegment::Ptr cTimelineTrack::findSegmentFromTimeCode(cTimeCode tc)
{
  tList::iterator it = Segments.begin();
  while(it != Segments.end())
  {
    cTimeCode InPoint = (*it)->destIn();
    cTimeCode OutPoint = (*it)->destOut();
    if(tc >= InPoint && tc < OutPoint)
    {
      return *it;
    }
    ++it;
  }
  return cSegment::Ptr(0);
}

void cTimelineTrack::adjustInSeg(cSegment::Ptr InSeg,cTimeCode InPoint)
{
  int diff = InPoint.framesSinceMidnight() - InSeg->destOut().framesSinceMidnight();
  InSeg->adjustDestOut(diff);
}

void cTimelineTrack::adjustOutSeg(cSegment::Ptr OutSeg,cTimeCode OutPoint)
{
  int diff = OutPoint.framesSinceMidnight() - OutSeg->destIn().framesSinceMidnight();
  OutSeg->adjustSrcIn(diff);
  OutSeg->adjustDestIn(diff);
}

void cTimelineTrack::insertSegment(cSegment::Ptr segmentToInsert)
{
  //assert(!Segments.empty());

  cTimeCode InPoint = segmentToInsert->destIn();
  cTimeCode OutPoint = segmentToInsert->destOut();

  cSegment::Ptr InSeg = findSegmentFromTimeCode(InPoint);
  cSegment::Ptr OutSeg = findSegmentFromTimeCode(OutPoint);

  if(!InSeg)
  {
    //insert prior to first destIn of list!
    wcerr << L"ERROR: Insert edit prior to first time in list. Edit Num = " << segmentToInsert->getEditNumber() << endl;
    wcout << L"ERROR: Insert edit prior to first time in list. Edit Num = " << segmentToInsert->getEditNumber() << endl;
    return;
  }

  tList OutSegementStore;
  if(OutSeg)
  {
    //remove and keep extra segments up until to OutSeg
    cSegment::Ptr back = Segments.back();
    while(back != OutSeg)
    {
      OutSegementStore.push_front(back);
      Segments.pop_back();
      back = Segments.back();
    }

    //remove and destory segments up until to InSeg
    while(back != InSeg)
    {
      Segments.pop_back();
      if(back != OutSeg)
      {
        delete back;
      }
      back = Segments.back();
    }
  }
  else
  {
    //remove and destory segments up until to InSeg
    cSegment::Ptr back = Segments.back();
    while(back != InSeg)
    {
      Segments.pop_back();
      delete back;
      back = Segments.back();
    }
  }

  if(InSeg == OutSeg)
  {
    OutSeg = InSeg->clone();
  }

  if(InPoint == InSeg->destIn())
  {
    Segments.pop_back();
    delete InSeg;
    InSeg = 0;
  }
  else
  {
    adjustInSeg(InSeg,InPoint);
  }

  Segments.push_back(segmentToInsert);

  if(OutSeg)
  {
    if(InPoint == OutPoint && 
       segmentToInsert->cassette() == OutSeg->cassette() && 
       InPoint >= OutSeg->destIn() && 
       InPoint < OutSeg->destOut() &&
       OutSegementStore.empty())
    {
      //we are *probably* a cut prior to a transition that has eaten into the prior edit
      delete OutSeg;
    }
    else
    {
      adjustOutSeg(OutSeg,OutPoint);
      Segments.push_back(OutSeg);

      //append all extra Out Segments.
      for(tList::iterator it = OutSegementStore.begin();it != OutSegementStore.end();++it)
      {
        Segments.push_back(*it);
      }
    }
  }
}

void cTimelineTrack::print()
{
  wcout << L"NUM TYPE  DUR TAPE     TIME IN     TIME OUT\n";
  for(tList::iterator it = Segments.begin(); it != Segments.end();++it)
  {
    (*it)->print();
  }
}

void cTimelineTrack::setInTime(cTimeCode tc)
{
  TimeIn = tc;
}


void cEventTrack::add(cEvent::Ptr event)
{
  Events.push_back(event);
}

void cEventTrack::print()
{
  wcout << L"TYPE    TIME        COMMENT\n";
  for(tList::iterator it = Events.begin(); it != Events.end();++it)
  {
    (*it)->print();
  }
}
