#include "cTrack.h"

//#define NUM_AUDIO_CHANNELS 2 //to ignore AUD commands
//#define NUM_AUDIO_CHANNELS 4 //to accept AUD commands
#define NUM_AUDIO_CHANNELS 8 //to accept AUD commands

#define MERGE_B_ROLLS true // set to false to avoid merging A and B rolls

struct CassetteInfo
{
  aafRational_t Rate;
  bool DropFrame;
  bool V;
  bool A[NUM_AUDIO_CHANNELS];
  CassetteInfo();
  CassetteInfo(aafRational_t rate,bool dropFrame);
};

class cAafEdl : public cEdlData
{
  typedef std::map<wstring,CassetteInfo*> tCass;
  typedef tCass::iterator tCassIt;
  tCass cassettes;

  typedef std::multimap<wstring,cUnknownPtr<IAAFMob> > tMobLookup;
  typedef tMobLookup::iterator tMobLookupIt;
  tMobLookup cassetteToMasterMob;

  wstring Cassette;
  CassetteInfo* CurrentCassette;

  int EditNum;

  cTimelineTrack Video;
  cTimelineTrack Audio[NUM_AUDIO_CHANNELS];
  cEventTrack CommentTrack;

  cTimeCode In;
  cTimeCode Out;
  cTimeCode DestIn;
  cTimeCode DestOut;
  
  wstring Comment;

  aafRational_t SpeedRatio;
  
  cSegment::Ptr Seg;

  void Commit();
  void Reset();

  wstring Title;

  cTimeCode TimelineDestIn;
  bool setTimelineDestIn;
  cTimeCode TimelineDestOut;

//AAF stuff
  cUnknownPtr<IAAFFile> pFile;
  cUnknownPtr<IAAFHeader> pHeader;
  cUnknownPtr<IAAFDictionary> pDictionary;

  cUnknownPtr<IAAFDataDef> pPictureDataDef;
  cUnknownPtr<IAAFDataDef> pSoundDataDef;
  cUnknownPtr<IAAFDataDef> pTimecodeDataDef;

  void initAaf();

  void createAafFile(wstring filename);
  void closeFile();

  void saveEssence();
  void saveComposition();

  void createEssenceMobs(wstring mobName, wstring cassName, aafRational_t rate, bool dropFrame, cTimeCode in, cTimeCode out, cUnknownPtr<IAAFEssenceDescriptor> pEssenceDescriptor,bool V = true,const bool* pA = 0);

  void saveEssenceVA(aafRational_t rate, cTimeCode In, cTimeCode Out, cUnknownPtr<IAAFSourceMob> pSourceMob, cUnknownPtr<IAAFMob> pMobSrc, cUnknownPtr<IAAFMasterMob> pMasterMob, cUnknownPtr<IAAFMob> pMobMast);
  void saveEssenceTC(aafRational_t rate, bool dropFrame, cTimeCode In, cTimeCode Out, cUnknownPtr<IAAFSourceMob> pSourceMob, cUnknownPtr<IAAFMob> pMobSrc);

  void saveCompositionVA(cUnknownPtr<IAAFMob> pMobComp, aafRational_t rate);
  void saveCompositionTC(cUnknownPtr<IAAFMob> pMobComp, aafRational_t rate);
  void saveCompositionComments(cUnknownPtr<IAAFMob> pMobComp, aafRational_t rate);

  void removeZeroLengthCuts();
  void adjustComposition();

  cUnknownPtr<IAAFDataDef> getDataDef(int track);


public:
  typedef cAafEdl* Ptr;

  cAafEdl(float fps);

  virtual void fcm(wstring str);
  virtual void title(wstring str);
  virtual void cassette(wstring str);
  virtual void editNum(int n);
  virtual void channels(bool v, bool a1, bool a2);
  virtual void cut();
  virtual void dissolve(int n);
  virtual void wipe(int smpte,int n,bool reverse);
  virtual void srcIn(cTimeCode tc);
  virtual void srcOut(cTimeCode tc);
  virtual void destIn(cTimeCode tc);
  virtual void destOut(cTimeCode tc);
  virtual void comment(wstring str);
  virtual void m2(wstring cass, float speed, cTimeCode tc);
  virtual void audioTracks(unsigned int audioChannelBitMask);
  virtual void other(wstring str);

  virtual void begin();
  virtual void end();

  void createAaf(wstring filename);

  void m2CommandMightApplyToPreviousLineInThisEdit(wstring cass, float speed, cTimeCode tc);
};

