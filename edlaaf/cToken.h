class cToken
{
  wstring Token;
public:
  typedef std::list<cToken> tList;

  cToken(wstring token);
  void print();
  int integer();
  bool isInt();
  float floating();
  bool isFloat();
  wstring text();
};

