class cEdl
{
  cLine::tList Lines;
  typedef cLine::tList::iterator tLineIt;

  cEdlData::Ptr Data;

public:
  cEdl(cEdlData::Ptr data);
  void addline(wstring str);
  void print();
  void process();
};

