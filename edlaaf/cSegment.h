class cSegment
{
private:
  int Edit;
  wstring Cassette;
  cTimeCode In;
  cTimeCode DestIn;
  cTimeCode DestOut;
  aafRational_t SpeedRatio;
  int OutFadeDuration;
public:
  typedef cSegment* Ptr;

  cSegment();
  virtual ~cSegment();

  virtual void print() = 0;

  void createComponent(
    cUnknownPtr<IAAFMob> pMob,
    int track,
    cUnknownPtr<IAAFDictionary> pDictionary,
    cUnknownPtr<IAAFDataDef> pDataDef,
    cUnknownPtr<IAAFSequence> pSequence);

  void createFiller(
    cUnknownPtr<IAAFDictionary> pDictionary,
    cUnknownPtr<IAAFDataDef> pDataDef,
    cUnknownPtr<IAAFSequence> pSequence);

  virtual bool removeZeroLengthCut(cSegment::Ptr prevprev,cSegment::Ptr prev) = 0;

  virtual void adjust(cSegment::Ptr prev,bool sound) = 0;

  void set(int edit,wstring cassette,cTimeCode in,cTimeCode destIn,cTimeCode destOut,aafRational_t speedRatio); 

  int getEditNumber();
  wstring cassette();
  cTimeCode in();
  cTimeCode out();
  cTimeCode destIn();
  cTimeCode destOut();
  int length();
  aafRational_t speedRatio();
  void setSpeedRatio(aafRational_t speedRatio);

  void adjustSrcIn(int diff);
  void adjustDestIn(int diff);
  void adjustDestOut(int diff);

  int getOutFade();
  void setOutFade(int duration);

  virtual cSegment::Ptr clone() = 0; 

  virtual int getTransitionDuration() = 0; 

protected:
  void edit();
  void frames();


  cUnknownPtr<IAAFComponent> 
  createSourceClip(cUnknownPtr<IAAFDictionary> pDictionary,
                   cUnknownPtr<IAAFDataDef> pDataDef,
                   cUnknownPtr<IAAFMob> pMob, 
                   aafSlotID_t masterSlotID, 
                   cTimeCode srcIn, 
                   cTimeCode destIn, 
                   cTimeCode destOut,
                   int inFade,
                   int outFade,
                   int editNum,
                   aafRational_t speedRatio);
  cUnknownPtr<IAAFComponent> 
  createFiller(cUnknownPtr<IAAFDictionary> pDictionary,
               cUnknownPtr<IAAFDataDef> pDataDef,
               cTimeCode destIn, 
               cTimeCode destOut);

  void GetDisolve(cUnknownPtr<IAAFDictionary> pDictionary,
                       cUnknownPtr<IAAFDataDef> pDataDef,
                       const aafUID_t& uid,
                       IAAFOperationDef ** ppOpDef);
  void GetWipeParamSMPTE(cUnknownPtr<IAAFDictionary> pDictionary,
                    cUnknownPtr<IAAFDataDef> pDataDef,
                    IAAFParameterDef** ppParamDef);
  void GetWipeParamReverse(cUnknownPtr<IAAFDictionary> pDictionary,
                    cUnknownPtr<IAAFDataDef> pDataDef,
                    IAAFParameterDef** ppParamDef);
  void GetWipe(cUnknownPtr<IAAFDictionary> pDictionary,
                       cUnknownPtr<IAAFDataDef> pDataDef,
                       IAAFOperationDef ** ppOpDef);
  void GetVideoSpeedControlParam(cUnknownPtr<IAAFDictionary> pDictionary,
                                 cUnknownPtr<IAAFDataDef> pDataDef,
                                 IAAFParameterDef** ppParamDef);
  void GetVideoSpeedControl(cUnknownPtr<IAAFDictionary> pDictionary,
                            cUnknownPtr<IAAFDataDef> pDataDef,
                            const aafUID_t& uid,
                            IAAFOperationDef ** ppOpDef);

  void setEditNumber( cUnknownPtr<IAAFDictionary> pDictionary,
                      cUnknownPtr<IAAFComponent> pComponent,
                      int editNum);

  virtual cUnknownPtr<IAAFComponent> createTransition(
    cUnknownPtr<IAAFDictionary> pDictionary,
    cUnknownPtr<IAAFDataDef> pDataDef) = 0;

private:
  cUnknownPtr<IAAFComponent>  
  createTimeWarp( cUnknownPtr<IAAFDictionary> pDictionary,
                  cUnknownPtr<IAAFDataDef> pDataDef,
                  cUnknownPtr<IAAFSourceClip> pSourceClip,
                  aafRational_t speedRatio);
};
