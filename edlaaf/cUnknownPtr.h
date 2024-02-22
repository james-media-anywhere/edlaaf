#ifndef cUnknownPtr_DEFINED
#define cUnknownPtr_DEFINED

template <typename T>
class cUnknownPtr
{
  T* ptr;
public:
  cUnknownPtr() : ptr(0) 
  {
  }

  cUnknownPtr(T* p) : ptr(p) 
  {
  }

  cUnknownPtr(const cUnknownPtr<T>& other) : ptr(other.ptr) 
  {
    if(ptr)
    {
      ptr->AddRef();
    }
  }

  ~cUnknownPtr() 
  {
    setNull();
  }

  cUnknownPtr<T>& operator = (const cUnknownPtr<T>& other) //treat as pointer
  {
    if(other.ptr != ptr)
    {
      if(ptr)ptr->Release();
      ptr = other.ptr;
      if(ptr)ptr->AddRef();
    }
    return *this;
  }

  T* operator -> () //treat as pointer
  {
    return ptr;
  }

  operator T* () //silent cast operator
  {
    return ptr;
  }

  T** operator & () //address of operator
  {
    return &ptr;
  }

  bool isValid()
  {
    return ptr != 0;
  }

  bool isNull()
  {
    return ptr == 0;
  }

  void setNull()
  {
    if(ptr)
    {
      ptr->Release();
      ptr = 0;
    }
  }

  bool QI(REFIID riid,void** ppObj)
  {
    if(isNull())return false;
    return SUCCEEDED(ptr->QueryInterface(riid,ppObj));
  }

  void demandQI(REFIID riid,void** ppObj) throw(HRESULT)
  {
    if(isNull())throw E_FAIL;
    HRESULT hr = ptr->QueryInterface(riid,ppObj);
    if(FAILED(hr))
    {
      throw hr;
    }
  }
};

inline void checkResult(HRESULT r)
{
  if (FAILED(r))
    throw r;
}

#endif//cUnknownPtr_DEFINED