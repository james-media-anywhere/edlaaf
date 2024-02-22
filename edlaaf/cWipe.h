class cWipe : public cSegment
{
  int Duration;
  aafInt32 Smpte;
  aafBoolean_t Reverse;

  cUnknownPtr<IAAFComponent> createTransition(
    cUnknownPtr<IAAFDictionary> pDictionary,
    cUnknownPtr<IAAFDataDef> pDataDef);

public:
  typedef cWipe* Ptr;

  cWipe(aafInt32 smpte,int duration,aafBoolean_t reverse);

  void print();

  bool removeZeroLengthCut(cSegment::Ptr prevprev,cSegment::Ptr prev);

  void adjust(cSegment::Ptr prev,bool sound);

  cSegment::Ptr clone();

  int getTransitionDuration();
};
