class cEdlData
{
  float Fps;
  bool SrcDropFrame;
  bool DestDropFrame;
  bool HadFirstFCM;

protected:
  int getFps();
  float getRealFps();
  aafRational_t getFrameRate();
  bool isSrcDropFrame();

public:
  typedef cEdlData* Ptr;

  cEdlData(float fps);

  //utility functions
  cTimeCode srcTimecode(wstring str);
  cTimeCode destTimecode(wstring str);

  virtual void fcm(wstring str);

  virtual void title(wstring str) = 0;
  virtual void cassette(wstring str) = 0;
  virtual void editNum(int n) = 0;
  virtual void channels(bool v, bool a1, bool a2) = 0;
  virtual void cut() = 0;
  virtual void dissolve(int n) = 0;
  virtual void wipe(int smpte,int n,bool reverse) = 0;
  virtual void srcIn(cTimeCode tc) = 0;
  virtual void srcOut(cTimeCode tc) = 0;
  virtual void destIn(cTimeCode tc) = 0;
  virtual void destOut(cTimeCode tc) = 0;
  virtual void comment(wstring str) = 0;
  virtual void m2(wstring cass, float speed, cTimeCode tc) = 0;
  virtual void audioTracks(unsigned int audioChannelBitMask) = 0;
  virtual void other(wstring str) = 0;

  virtual void begin() = 0;
  virtual void end() = 0;
};
