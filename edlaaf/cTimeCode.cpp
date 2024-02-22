#include "StdAfx.h"
#include "cTimeCode.h"
#define QMSG(PARAMS) fwprintf##PARAMS

// DropFrame Timecode
// drop frame occurs only in 30 fps which means that the first 2 frames of the every minute
// are dropped EXCEPT for the first minute of each ten minute period.

static const int kSecondsPerMinute = 60;
static const int kSecondsPerHour = 60 * 60;                             // 3600
static const int kSecondsPerDay = 24 * 60 * 60;                         // 86400

static const int kMinutesPerHour = 60;
static const int kMinutesPerDay = 24 * 60;                              // 1440

static const int kHoursPerDay = 24;

static const int kDFFramesPerSecond = 30;
static const int kDFFramesPerMinute = 60 * 30 - 2;                      // 1798
static const int kDFAdjust = 2;
static const int kDFFramesPerTenMinute = 10 * kDFFramesPerMinute + 2;   // 17982
static const int kDFFramesPerHour = 6 * kDFFramesPerTenMinute;          // 107892

// Packed timecode format constants
// packed timecode packs as follows:
// bits:00-23   FramesSinceMidnight
// bits:24-27   Frames per second code
// bit: 28      Drop frame flag
// bits:29-31   Spare

static const unsigned int kPackedFramesMask = 0x00FFFFFF;
static const unsigned int kPackedFpsMask    = 0x0F000000;
static const unsigned int kPackedFps25      = 0x00000000;       // default, hence first
static const unsigned int kPackedFps24      = 0x01000000;
static const unsigned int kPackedFps30      = 0x02000000;
static const unsigned int kPackedFps60      = 0x03000000;
static const unsigned int kPackedDropFrameBit=0x10000000;

/*
Constructs a cTimeCode of hour::minute::second::frame.

hour the cTimeCode's hour, in the range 0 - 23.
minute the cTimeCode's minute, in the range 0 - 59.
second the cTimeCode's second, in the range 0 - 59.
frame the cTimeCode's frame, in the range 0 - (frame rate - 1).

Note that in 30fps drop-frame mode the dropped TimeCodes are out of range.
*/
cTimeCode::cTimeCode (int hour, int minute, int second, int frame, eFps fps, bool dropFrame) 
  : Fps(fps), DropFrame(dropFrame)
{
checkDropFrameState();
updateFramesPerDay();
//{{{
Hour = (unsigned char) limit (hour, 0, kHoursPerDay-1);
if (Hour != hour)
{ 
  QMSG((stdout,L"WARNING: Bad Timecode hour value: %d limited to %d\n", hour, Hour)); 
  QMSG((stderr,L"WARNING: Bad Timecode hour value: %d limited to %d\n", hour, Hour)); 
}
//}}}
//{{{
Minute = (unsigned char) limit (minute, 0, kMinutesPerHour-1);
if (Minute != minute)
{ 
  QMSG((stdout,L"WARNING: Bad Timecode minute value: %d limited to %d\n", minute, Minute)); 
  QMSG((stderr,L"WARNING: Bad Timecode minute value: %d limited to %d\n", minute, Minute)); 
}
//}}}
//{{{
Second = (unsigned char) limit (second, 0, kSecondsPerMinute-1);
if (Second != second)
{ 
  QMSG((stdout,L"WARNING: Bad Timecode second value: %d limited to %d\n", second, Second)); 
  QMSG((stderr,L"WARNING: Bad Timecode second value: %d limited to %d\n", second, Second)); 
}
//}}}
//{{{
Frame = (unsigned char) limit (frame, 0, Fps-1);
if (Frame != frame)
{ 
  QMSG((stdout,L"WARNING: Bad Timecode frame value: %d limited to %d\n", frame, Frame)); 
  QMSG((stderr,L"WARNING: Bad Timecode frame value: %d limited to %d\n", frame, Frame)); 
}
//}}}
//{{{  if drop frame, check valid time, bump up to 2 frames into the minute
if (DropFrame && (Second == 0) && (Minute % 10) && (Frame < 2))
{
  QMSG((stdout,L"WARNING: Bumping up drop frame value\n"));
  QMSG((stderr,L"WARNING: Bumping up drop frame value\n"));
  Frame = 2;
}
//}}}
timeCodeToFrames();  // setup frames since midnight
}
//}}}
//{{{
/*
Constructs a cTimeCode as a number of frames since midnight.
Note that in drop-frame mode the dropped TimeCodes are out of range.
*/
cTimeCode::cTimeCode (int framesSinceMidnight, eFps fps, bool dropFrame) 
  : Fps(fps), DropFrame(dropFrame)
{
checkDropFrameState();
updateFramesPerDay();
setFramesSinceMidnight (framesSinceMidnight);
}
//}}}

cTimeCode::cTimeCode (String tc, eFps fps, bool dropFrame) 
  : Fps(fps), DropFrame(dropFrame)
{
checkDropFrameState();
updateFramesPerDay();

wstringstream in(tc);

int hour;
int minute;
int second;
int frame;
wchar_t ch;

in >> hour;
in >> ch;
if(ch != L':')
{ 
  QMSG((stdout,L"WARNING: Bad Timecode hour/minute separator: %c\n", ch)); 
  QMSG((stderr,L"WARNING: Bad Timecode hour/minute separator: %c\n", ch)); 
}

in >> minute;
in >> ch;
if(ch != L':')
{ 
  QMSG((stdout,L"WARNING: Bad Timecode minute/second separator: %c\n", ch)); 
  QMSG((stderr,L"WARNING: Bad Timecode minute/second separator: %c\n", ch)); 
}

in >> second;
in >> ch;
if(ch != L':' && ch != L';')
{ 
  QMSG((stdout,L"WARNING: Bad Timecode second/frame separator: %c\n", ch)); 
  QMSG((stderr,L"WARNING: Bad Timecode second/frame separator: %c\n", ch)); 
}

in >> frame;

//{{{
Hour = (unsigned char) limit (hour, 0, kHoursPerDay-1);
if (Hour != hour)
{ 
  QMSG((stdout,L"WARNING: Bad Timecode hour value: %d limited to %d\n", hour, Hour)); 
  QMSG((stderr,L"WARNING: Bad Timecode hour value: %d limited to %d\n", hour, Hour)); 
}
//}}}
//{{{
Minute = (unsigned char) limit (minute, 0, kMinutesPerHour-1);
if (Minute != minute)
{
  QMSG((stdout,L"WARNING: Bad Timecode minute value: %d limited to %d\n", minute, Minute)); 
  QMSG((stderr,L"WARNING: Bad Timecode minute value: %d limited to %d\n", minute, Minute)); 
}
//}}}
//{{{
Second = (unsigned char) limit (second, 0, kSecondsPerMinute-1);
if (Second != second)
{ 
  QMSG((stdout,L"WARNING: Bad Timecode second value: %d limited to %d\n", second, Second)); 
  QMSG((stderr,L"WARNING: Bad Timecode second value: %d limited to %d\n", second, Second)); 
}
//}}}
//{{{
Frame = (unsigned char) limit (frame, 0, Fps-1);
if (Frame != frame)
{ 
  QMSG((stdout,L"WARNING: Bad Timecode frame value: %d limited to %d\n", frame, Frame)); 
  QMSG((stderr,L"WARNING: Bad Timecode frame value: %d limited to %d\n", frame, Frame)); 
}
//}}}
//{{{  if drop frame, check valid time, bump up to 2 frames into the minute
if (DropFrame && (Second == 0) && (Minute % 10) && (Frame < 2))
{
  QMSG((stdout,L"WARNING: Bumping up drop frame value\n"));
  QMSG((stderr,L"WARNING: Bumping up drop frame value\n"));
  Frame = 2;
}
//}}}
timeCodeToFrames();  // setup frames since midnight
};


//{{{
cTimeCode::cTimeCode (const cTimeCode& other) 
  : Hour(other.Hour), Minute(other.Minute), Second(other.Second), Frame(other.Frame),
    FramesSinceMidnight(other.FramesSinceMidnight), DropFrame(other.DropFrame), Fps(other.Fps),
    FramesPerDay(other.FramesPerDay)
{
}
//}}}
//{{{
cTimeCode::cTimeCode()
  : Hour(0), Minute(0), Second(0), Frame(0), FramesSinceMidnight(0),
    DropFrame(false), Fps(e25fps)
{
FramesPerDay = kSecondsPerDay * Fps;
}
//}}}

//{{{
void cTimeCode::incWithLimit (int frames) 
{
// do calc based upon frames since midnight - increment WITHOUT wraparound
FramesSinceMidnight = limit (FramesSinceMidnight+frames, 0, FramesPerDay-1);
framesToTimeCode();
}
//}}}
//{{{
cTimeCode& cTimeCode::operator += (int frames) 
{
// do calc based upon frames since midnight - increment WITH wraparound
FramesSinceMidnight += frames;
FramesSinceMidnight %= FramesPerDay;
if (FramesSinceMidnight < 0) FramesSinceMidnight += FramesPerDay;       // make sure it is positive
framesToTimeCode();
return *this;
}
//}}}
//{{{
cTimeCode& cTimeCode::operator += (const cTimeCode& timecode) 
{
// do calc based upon frames since midnight
FramesSinceMidnight += timecode.framesSinceMidnight();
FramesSinceMidnight %= FramesPerDay;
if (FramesSinceMidnight < 0) FramesSinceMidnight += FramesPerDay;       // make sure it is positive
framesToTimeCode();
return *this;
}
//}}}
//{{{
cTimeCode& cTimeCode::operator -= (int frames) 
{
// do calc based upon frames since midnight
FramesSinceMidnight -= frames;
FramesSinceMidnight %= FramesPerDay;
if (FramesSinceMidnight < 0) FramesSinceMidnight += FramesPerDay;       // make sure it is positive
framesToTimeCode();
return *this;
}
//}}}
//{{{
cTimeCode& cTimeCode::operator -= (const cTimeCode& timecode) 
{
// do calc based upon frames since midnight
FramesSinceMidnight -= timecode.framesSinceMidnight();
FramesSinceMidnight %= FramesPerDay;
if (FramesSinceMidnight < 0) FramesSinceMidnight += FramesPerDay;       // make sure it is positive
framesToTimeCode();
return *this;
}
//}}}

//{{{
bool cTimeCode::operator == (const cTimeCode &time) const 
// test this cTimeCode for equivalence with another.
// TimeCodes are equal if they have the same mode frame rate and represent the same timecode.
{
return
   (FramesSinceMidnight == time.FramesSinceMidnight)
&& (DropFrame == time.DropFrame)
&& (Fps == time.Fps);
}
//}}}
//{{{
bool cTimeCode::operator != (const cTimeCode &time) const 
// test this cTimeCode for non-equivalence with another cTimeCode.
// TimeCodes are not equal if they have difference mode, frame rate or represent different timecodes.
{
return (FramesSinceMidnight != time.FramesSinceMidnight)
|| (DropFrame != time.DropFrame)
|| (Fps != time.Fps);
}
//}}}
//{{{
bool cTimeCode::operator <  (const cTimeCode &time) const 
// takes no notice of modes and frame rates
// since we perform a byte packed compare which will be independent of fps differences
{
unsigned int packedThis = (Hour<<24) | (Minute<<16)| (Second<<8) | Frame;
unsigned int packedTime = (time.Hour<<24) | (time.Minute<<16)| (time.Second<<8) | time.Frame;
return packedThis < packedTime;
}
//}}}
//{{{
bool cTimeCode::operator <= (const cTimeCode &time) const 
// take no notice of modes and frame rates
// since we perform a byte packed compare which will be independent of fps differences
{
unsigned int packedThis = (Hour<<24) | (Minute<<16)| (Second<<8) | Frame;
unsigned int packedTime = (time.Hour<<24) | (time.Minute<<16)| (time.Second<<8) | time.Frame;
return packedThis <= packedTime;
}
//}}}
//{{{
bool cTimeCode::operator >  (const cTimeCode &time) const 
// take no notice of modes and frame rates
// since we perform a byte packed compare which will be independent of fps differences
{
unsigned int packedThis = (Hour<<24) | (Minute<<16)| (Second<<8) | Frame;
unsigned int packedTime = (time.Hour<<24) | (time.Minute<<16)| (time.Second<<8) | time.Frame;
return packedThis > packedTime;
}
//}}}
//{{{
bool cTimeCode::operator >= (const cTimeCode &time) const 
// take no notice of modes and frame rates
// since we perform a byte packed compare which will be independent of fps differences
{
unsigned int packedThis = (Hour<<24) | (Minute<<16)| (Second<<8) | Frame;
unsigned int packedTime = (time.Hour<<24) | (time.Minute<<16)| (time.Second<<8) | time.Frame;
return packedThis >= packedTime;
}
//}}}

//{{{
String cTimeCode::toString() const 
{
const Char zero = '0';
String str = L"00:00:00:00";
str[0] = (Char) (zero + (Hour / 10));
str[1] = (Char) (zero + (Hour % 10));
str[3] = (Char) (zero + (Minute / 10));
str[4] = (Char) (zero + (Minute % 10));
str[6] = (Char) (zero + (Second / 10));
str[7] = (Char) (zero + (Second % 10));
str[9] = (Char) (zero + (Frame / 10));
str[10] = (Char) (zero + (Frame % 10));

if (DropFrame)
  str[8] = (Char)('.');
else
  str[8] = (Char)(':');

return str;
}
//}}}

//{{{
void cTimeCode::checkDropFrameState()
// checks for consistency of drop frame flag with fps value
{
bool origDropFrame = DropFrame;
if (Fps == e25fps)
  DropFrame = false;            // force to drop frame off if not 30 fps
if (DropFrame != origDropFrame)
  { 
  QMSG((stdout,L"WARNING: changing dropframe mode to %d : %dfps\n", DropFrame, Fps)); 
  QMSG((stderr,L"WARNING: changing dropframe mode to %d : %dfps\n", DropFrame, Fps)); 
  }
}
//}}}
//{{{
void cTimeCode::updateFramesPerDay()
// set number of frames per day based upon fps and drop frame states
{
if (DropFrame)
  FramesPerDay = kHoursPerDay * kDFFramesPerHour;
else
  FramesPerDay = kSecondsPerDay * Fps;
}
//}}}
//{{{
void cTimeCode::framesToTimeCode()
// assumes FramesSinceMidnight, DropFrame, Fps are all set correctly
{
if (DropFrame)
  {
  int frames = FramesSinceMidnight;

  Hour = (unsigned char) (frames / kDFFramesPerHour);
  frames %= kDFFramesPerHour;

  int tenMinute = frames / kDFFramesPerTenMinute;
  frames %= kDFFramesPerTenMinute;

  // must adjust frame count to make minutes calculation work
  // calculations from here on in just assume that within the 10 minute cycle
  // there are only kDFFramesPerMinute (1798) frames per minute - even for the first minute
  // in the ten minute cycle. Hence we decrement the frame count by 2 to get the minutes count
  // So for the first two frames of the ten minute cycle we get a negative frames number

  frames -= kDFAdjust;

  int unitMinute = frames / kDFFramesPerMinute;
  frames %= kDFFramesPerMinute;
  Minute = (unsigned char) (tenMinute * 10 + unitMinute);

  // frames now contains frame in minute @ 1798 frames per minute
  // put the 2 frame adjustment back in to get the correct frame count
  // For the first two frames of the ten minute cycle, frames is negative and this
  // adjustment makes it non-negative. For other minutes in the cycle the frames count
  // goes from 0 upwards, thus this adjusment gives the required 2 frame offset

  frames += kDFAdjust;

  Second = (unsigned char) (frames / 30);
  Frame = (unsigned char) (frames % 30);
  }
else
  {
  int frames = FramesSinceMidnight;

  int framesDivisor = kSecondsPerHour * Fps;
  Hour = (unsigned char) (frames / framesDivisor);
  frames %= framesDivisor;

  framesDivisor = kSecondsPerMinute * Fps;
  Minute = (unsigned char) (frames / framesDivisor);
  frames %= framesDivisor;

  Second = (unsigned char) (frames / Fps);

  Frame = (unsigned char) (frames - Second * Fps);
  }
}
//}}}
//{{{
void cTimeCode::timeCodeToFrames()
// assumes hh:mm:ss:ff are normalised and DropFrame, Fps are set correctly
{
if (DropFrame)
  {
  FramesSinceMidnight =
    Frame +
    Second * Fps +
    (Minute % 10) * kDFFramesPerMinute +
    (Minute / 10) * kDFFramesPerTenMinute +
    Hour * kDFFramesPerHour;
  }
else
  FramesSinceMidnight =
    Frame + Fps * (Second + Minute * kSecondsPerMinute + Hour * kSecondsPerHour);
}
//}}}

//{{{
int cTimeCode::framesSinceMidnight() const 
{
return FramesSinceMidnight;
}
//}}}
//{{{
void cTimeCode::setFramesSinceMidnight (int framesSinceMidnight) 
{
FramesSinceMidnight = limit (framesSinceMidnight, 0, FramesPerDay-1);
if (FramesSinceMidnight != framesSinceMidnight)
{ 
  QMSG((stdout,L"WARNING: Bad Timecode frames value: %d limited to %d\n", framesSinceMidnight, FramesSinceMidnight)); 
  QMSG((stderr,L"WARNING: Bad Timecode frames value: %d limited to %d\n", framesSinceMidnight, FramesSinceMidnight)); 
}
framesToTimeCode();
}
//}}}
//{{{
int cTimeCode::maxFrames() const 
{
return FramesPerDay - 1;
}
//}}}

//{{{
unsigned int cTimeCode::getPacked() const 
{
// determine packed FPS code
unsigned int fpsCode;
switch (Fps)
  {
  case e24fps: fpsCode = kPackedFps24; break;
  default:
  case e25fps: fpsCode = kPackedFps25; break;
  case e30fps: fpsCode = kPackedFps30; break;
  case e60fps: fpsCode = kPackedFps60; break;
  }

return (unsigned int)
  (DropFrame ? kPackedDropFrameBit : 0) | fpsCode | (FramesSinceMidnight & kPackedFramesMask);
}
//}}}
//{{{
void cTimeCode::setFromPacked (unsigned int packedTimeCode) 
{
DropFrame = (packedTimeCode & kPackedDropFrameBit) ? true : false;
switch (packedTimeCode & kPackedFpsMask)
  {
  case kPackedFps24: Fps = e24fps; break;
  default:
  case kPackedFps25: Fps = e25fps; break;
  case kPackedFps30: Fps = e30fps; break;
  case kPackedFps60: Fps = e60fps; break;
  }
// validate drop frame state against fps value
checkDropFrameState();

// use set frames routine to validate it
setFramesSinceMidnight (packedTimeCode & kPackedFramesMask);
}
//}}}

//{{{
unsigned char cTimeCode::hour() const 
{
return Hour;
}
//}}}
//{{{
unsigned char cTimeCode::minute() const 
{
return Minute;
}
//}}}
//{{{
unsigned char cTimeCode::second() const 
{
return Second;
}
//}}}
//{{{
unsigned char cTimeCode::frame() const 
{
return Frame;
}
//}}}
//{{{
eFps cTimeCode::fps() const 
{
return Fps;
}
//}}}
//{{{
bool cTimeCode::isDropFrame() const 
{
return DropFrame;
}
//}}}

int cTimeCode::getFramesPerDay() 
{
  updateFramesPerDay();
  return FramesPerDay;
}
