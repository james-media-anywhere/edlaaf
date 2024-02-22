enum eFps {e24fps=24, e25fps=25, e30fps=30, e60fps=60};
typedef wstring String;
typedef wchar_t Char;

class cTimeCode
{
public:
  cTimeCode (int hour, int minute, int second, int frame, eFps fps, bool dropFrame=false);
  cTimeCode (int framesSinceMidnight, eFps fps, bool dropFrame=false);
  cTimeCode (wstring tc, eFps fps, bool dropFrame=false);
  cTimeCode (const cTimeCode& other);
  cTimeCode ();

  cTimeCode& operator += (int frames);
  cTimeCode& operator += (const cTimeCode& timecode);
  cTimeCode& operator -= (int frames);
  cTimeCode& operator -= (const cTimeCode& timecode);

  bool operator == (const cTimeCode &time) const;
  bool operator != (const cTimeCode &time) const;
  bool operator <  (const cTimeCode &time) const;
  bool operator <= (const cTimeCode &time) const;
  bool operator >  (const cTimeCode &time) const;
  bool operator >= (const cTimeCode &time) const;

  void incWithLimit (int frames);

  wstring toString() const;

  int framesSinceMidnight() const;
  void setFramesSinceMidnight (int framesSinceMidnight);

  int maxFrames() const;

  unsigned int getPacked() const;
  void setFromPacked (unsigned int packedTimeCode);

  unsigned char hour() const;
  unsigned char minute() const;
  unsigned char second() const;
  unsigned char frame() const;

  eFps fps() const;
  bool isDropFrame() const;

  int getFramesPerDay();

//{{{
private:
  unsigned char Hour;
  unsigned char Minute;
  unsigned char Second;
  unsigned char Frame;
  int FramesSinceMidnight;
  bool DropFrame;
  eFps Fps;

  int FramesPerDay;

  //{{{
  int limit (int num, int min, int max)         // worth inlining
    {
    if (num < min) return min;
    if (num > max) return max;
    return num;
    }
  //}}}
  void checkDropFrameState();
  void updateFramesPerDay();
  void framesToTimeCode();
  void timeCodeToFrames();
//}}}
};


inline wostream& __cdecl 
operator<<(wostream& s,const cTimeCode& tc)
{
  s << tc.toString();
  return (s); 
}
