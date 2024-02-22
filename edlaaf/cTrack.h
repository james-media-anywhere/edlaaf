#include "cSegment.h"
#include "cEvent.h"

class cTimelineTrack
{
public:
  typedef std::list<cSegment::Ptr> tList;
  tList Segments;

  cTimeCode TimeIn;

  void add(cSegment::Ptr seg);
  void print();
  void setInTime(cTimeCode tc);
private:
  void insertSegment(cSegment::Ptr segmentToInsert);
  cSegment::Ptr findSegmentFromTimeCode(cTimeCode tc);
  void adjustInSeg(cSegment::Ptr InSeg,cTimeCode InPoint);
  void adjustOutSeg(cSegment::Ptr OutSeg,cTimeCode OutPoint);
};

class cEventTrack
{
public:
  typedef std::list<cEvent::Ptr> tList;
  tList Events;

  void print();
  void add(cEvent::Ptr event);
};