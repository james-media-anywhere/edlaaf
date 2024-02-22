class cLine
{
  cToken::tList Tokens;
  typedef cToken::tList::iterator tTokenIt;

  cEdlData::Ptr Data;

public:
  typedef std::list<cLine> tList;

  cLine(wstring line); 
  
  bool empty();
  void print();
  void process(cEdlData::Ptr data);
  tTokenIt editNum(tTokenIt it,wstring str);
  tTokenIt cassette(tTokenIt it,wstring str);
  tTokenIt channels(tTokenIt it,wstring str);
  tTokenIt transition(tTokenIt it,wstring str) ;
  void decision();
  void title();
  void fcm();
  void comment();
  void m2();
  void audioTracks();
  void other();
};

