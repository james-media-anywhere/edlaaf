class cEvent
{
private:
  cTimeCode Tc;
  wstring Comment;
public:
  typedef cEvent* Ptr;

  cEvent();
  virtual ~cEvent();

  void set(cTimeCode tc,wstring comment); 
  void print();

  cTimeCode tc();
  wstring comment();

  void createComment(
    cUnknownPtr<IAAFDictionary> pDictionary,
    cUnknownPtr<IAAFDataDef> pDataDef,
    cUnknownPtr<IAAFSequence> pSequence);
};

// template<>
// struct std::greater<cEvent::Ptr> : binary_function<_Ty, _Ty, bool> 
// {
//   bool operator()(const _Ty& x, const _Ty& y) const 
//   {
//     return y->tc() > x->tc();
//   }
// };
