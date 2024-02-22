#include "stdafx.h"
#include "cTimeCode.h"
#include "cEdlData.h"
#include "cToken.h"
#include "cLine.h"
#include "cEdl.h"
#include "cAafEdl.h"
#include "cCut.h"
#include "cDisolve.h"
#include "cWipe.h"

CassetteInfo::CassetteInfo()
{
  Rate.numerator = 0;
  Rate.denominator = 0;
  DropFrame = false;
  V = false;
  for(int i = 0; i < NUM_AUDIO_CHANNELS; ++i)
  {
    A[i] = false;
  }
}

CassetteInfo::CassetteInfo(aafRational_t rate,bool dropFrame) : Rate(rate), DropFrame(dropFrame)
{
  V = false;
  for(int i = 0; i < NUM_AUDIO_CHANNELS; ++i)
  {
    A[i] = false;
  }
}

cAafEdl::cAafEdl(float fps) : cEdlData(fps), CurrentCassette(0), EditNum(0), setTimelineDestIn(false)
{
  initAaf();
  Seg = 0;
  wcout << L"Fps: " << getRealFps() << L"\n";
}

void cAafEdl::fcm(wstring str)
{
  cEdlData::fcm(str);
  wcout << L"FCM: " << str << L"\n";
}

void cAafEdl::title(wstring str)
{
  wcout << L"Title: " << str << L"\n";
  Title = str;
}

void cAafEdl::cassette(wstring str)
{
  wcout << L"cassette " << str << L"\n";

  if(MERGE_B_ROLLS && !str.empty())
  {
    wstring::iterator it = str.end();
    --it;
    if(*it == L'B')
    {
      str.erase(it,str.end());
      wcout << L"Found B reel " << str << L"\n";
    }
  }

  aafRational_t rate = getFrameRate();
  Cassette = str;
  CurrentCassette = new CassetteInfo(rate,isSrcDropFrame());

  tCass::value_type val(Cassette,CurrentCassette);
  cassettes.insert(val);
}

void cAafEdl::Commit()
{
  if(CurrentCassette)
  {
    Seg->set(EditNum,Cassette,In,DestIn,DestOut,SpeedRatio);

    //each call to add makes a *deep* copy of the segment
    if(CurrentCassette->V)Video.add(Seg);

    for(int i = 0; i < NUM_AUDIO_CHANNELS; ++i)
    {
      if(CurrentCassette->A[i])Audio[i].add(Seg);
    }

    if(Comment != L"") {
      cEvent::Ptr event = new cEvent();
      event->set(DestIn, Comment);
      CommentTrack.add(event); // not cloned - so don't delete
    }
  }

  Reset();
}

void cAafEdl::Reset()
{
  CurrentCassette = 0;//this is not a leak - the CurrentCassette is also stored in the cassette map
  Cassette.erase();
  delete Seg;
  Seg = 0;
  Comment = L"";
  SpeedRatio.numerator = 1000;
  SpeedRatio.denominator = 1000;
}

void cAafEdl::editNum(int n) 
{
  wcout << L"\n";
  wcout << L"editNum " << n << L"\n";

  if(EditNum)
  {
    Commit();
  }

  EditNum = n;
}

void cAafEdl::channels(bool v, bool a1, bool a2) {
  wcout << L"channels ";
  if(v)wcout << L"video ";
  if(a1)wcout << L"audio1 ";
  if(a2)wcout << L"audio2 ";
  wcout << L"\n";

  if(CurrentCassette)
  {
    CurrentCassette->V = v;
    CurrentCassette->A[0] = a1;
    CurrentCassette->A[1] = a2;
  }
}

void cAafEdl::cut() 
{
  wcout << L"cut" << L"\n";
  Seg = new cCut;
}

void cAafEdl::dissolve(int n) 
{
  wcout << L"dissolve " << n << L"\n";
  Seg = new cDisolve(n);
}

void cAafEdl::wipe(int smpte,int n,bool reverse) 
{
  wcout << L"wipe " << smpte << L" " << n << L"\n";
  Seg = new cWipe(smpte,n,reverse);
}

void cAafEdl::srcIn(cTimeCode tc) 
{
  wcout << L"srcIn " << tc << L"\n";
  In = tc;
}

void cAafEdl::srcOut(cTimeCode tc) 
{
  wcout << L"srcOut " << tc << L"\n";
  Out = tc;
}

void cAafEdl::destIn(cTimeCode tc) 
{
  wcout << L"destIn " << tc << L"\n";
  DestIn = tc;
  if(!setTimelineDestIn)
  {
    setTimelineDestIn = true;
    TimelineDestIn = tc;

    Video.setInTime(TimelineDestIn);
    for(int j = 0; j < NUM_AUDIO_CHANNELS; ++j)
    {
      Audio[j].setInTime(TimelineDestIn);
    }
  }
}

void cAafEdl::destOut(cTimeCode tc) 
{
  wcout << L"destOut " << tc << L"\n";
  DestOut = tc;
  if(tc > TimelineDestOut)
  {
    TimelineDestOut = tc;//always track the greatest out time
  }
}

void cAafEdl::comment(wstring str)
{
  // assume comments refer to preceding edit decision
  wcout << L"Comment: " << str << L"\n";

  if (!Seg) // ignore comments before first edit decision
  {
    wcout << L"Ignored comment: no preceding segment available\n";
    return;
  }
  
  Comment = Comment + str + L"\n";
}

void cAafEdl::m2CommandMightApplyToPreviousLineInThisEdit(wstring cass, float speed, cTimeCode tc)
{
  bool addedOk = false;
  for(int i = 0; i <= NUM_AUDIO_CHANNELS; ++i)
  {
    cTimelineTrack* track = &Video;
    if(i)
    {
      track = &Audio[i-1];
    }

    if(!track->Segments.empty())
    {
      cSegment::Ptr seg = track->Segments.back();
      if(seg->getEditNumber() == EditNum)//match edit number
      {
        //the source timecode drop frame may have changed between the first line and the m2 line!
        unsigned char h=tc.hour();
        unsigned char m=tc.minute();
        unsigned char s=tc.second();
        unsigned char f=tc.frame();
        cTimeCode tc1 = seg->in();
        unsigned char h1=tc1.hour();
        unsigned char m1=tc1.minute();
        unsigned char s1=tc1.second();
        unsigned char f1=tc1.frame();

        if(seg->cassette() == cass && h==h1 && m==m1 && s==s1 && f==f1)//match cassette and in time (note drop frame insensitive!)
        {
          aafRational_t speedRatio = seg->speedRatio();
          if(speedRatio.numerator == speedRatio.denominator)//not already set
          {
            speedRatio.numerator = (int)(speed * 10); // multiplied by 10 to allow for tenth of frames
            speedRatio.denominator = getFps() * 10;
            seg->setSpeedRatio(speedRatio);
            addedOk = true;
          }
        }
      }
    }
  }
  if(!addedOk)
  {
    wcerr << L"Ignored M2 command - does not match with edit:" << EditNum << L"\n";
    wcout << L"Ignored M2 command - does not match with edit:" << EditNum << L"\n";
  }
}

void cAafEdl::m2(wstring cass, float speed, cTimeCode tc)
{
  if(MERGE_B_ROLLS && !cass.empty())
  {
    wstring::iterator it = cass.end();
    --it;
    if(*it == L'B')
    {
      cass.erase(it,cass.end());
      wcout << L"Found B reel in M2 command " << cass << L"\n";
    }
  }

  wcout << L"M2 cass " << cass << L" speed " << speed << L" tc " << tc << L"\n";

  if(cass != Cassette) {
    m2CommandMightApplyToPreviousLineInThisEdit(cass, speed, tc);
    return;
  }

  if(tc != In) {
//    wcerr << L"Ignored M2 command - in timecodes do not match\n";
    m2CommandMightApplyToPreviousLineInThisEdit(cass, speed, tc);
    return;
  }

  SpeedRatio.numerator = (int)(speed * 10); // multiplied by 10 to allow for tenth of frames
  SpeedRatio.denominator = getFps() * 10;
}

void cAafEdl::audioTracks(unsigned int audioChannelBitMask)
{
  for(int index = 2; index < NUM_AUDIO_CHANNELS; ++index)
  {
    if(CurrentCassette)
    {
      CurrentCassette->A[index] = (audioChannelBitMask & (1 << (index))) > 0;
    }
  }
}

void cAafEdl::other(wstring str) 
{
  wcout << L"Ignoring: " << str << L"\n";
  wcerr << L"Ignoring: " << str << L"\n";
}

void cAafEdl::begin()
{
  wcout << L"begin" << L"\n\n";
  Reset();
}

void cAafEdl::end()
{
  if(EditNum)
  {
    Commit();
  }

  wcout << L"\n" << L"Title: " << Title << L"\n";
  wcout << L"Fps: " << getRealFps();
  if(isSrcDropFrame())
  {
    wcout << L" Drop Frame";
  }
  wcout << L"\nStart Time " << TimelineDestIn << L"\n";

  if(!cassettes.empty())
  {
    wcout << L"Cassettes:\n";
    for(tCassIt it = cassettes.begin(); it != cassettes.end(); ++it)
    {
      wcout << L"   " << it->first << L"\n";
    }
    wcout << L"\n";
  }

  wcout << L"Video Track" << L"\n";
  Video.print();

  for(int i = 0; i < NUM_AUDIO_CHANNELS; ++i)
  {
    wcout << L"\n" << L"Audio Track " << i+1 << L"\n";
    Audio[i].print();
  }

  wcout << L"\n" << L"Comment Track" << L"\n";
  CommentTrack.print();
}
