class cDisolve : public cSegment
{
  int Duration;

  cUnknownPtr<IAAFComponent> createTransition(
    cUnknownPtr<IAAFDictionary> pDictionary,
    cUnknownPtr<IAAFDataDef> pDataDef);

public:
  typedef cDisolve* Ptr;

  cDisolve(int duration);

  void print();

  bool removeZeroLengthCut(cSegment::Ptr prevprev,cSegment::Ptr prev);

  void adjust(cSegment::Ptr prev,bool sound);

  cSegment::Ptr clone();

  int getTransitionDuration();
};
