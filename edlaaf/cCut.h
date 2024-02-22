class cCut : public cSegment
{
public:
  typedef cCut* Ptr;

  void print();

  bool removeZeroLengthCut(cSegment::Ptr prevprev,cSegment::Ptr prev);

  void adjust(cSegment::Ptr prev,bool sound);

  cSegment::Ptr clone();

  int getTransitionDuration();

protected:
  cUnknownPtr<IAAFComponent> createTransition(
    cUnknownPtr<IAAFDictionary> pDictionary,
    cUnknownPtr<IAAFDataDef> pDataDef);
};
