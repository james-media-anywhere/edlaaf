#include "stdafx.h"
#include "cTimeCode.h"
#include "cEvent.h"

cEvent::cEvent()
{
}

cEvent::~cEvent()
{
}

void cEvent::set(cTimeCode tc,wstring comment) 
{
  Tc = tc;
  Comment = comment;
}

void cEvent::print()
{
  wcout << L"Event   " << Tc.toString() << L" " << Comment.c_str();
}

cTimeCode cEvent::tc() 
{
  return Tc;
}

wstring cEvent::comment()
{
  return Comment;
}

void cEvent::createComment(cUnknownPtr<IAAFDictionary> pDictionary,
                           cUnknownPtr<IAAFDataDef> pDataDef,
                           cUnknownPtr<IAAFSequence> pSequence)
{
  if(Comment != L"") //ignore empty comments
  {
    cUnknownPtr<IAAFCommentMarker> pComment; // no initialise ???
    checkResult(pDictionary->CreateInstance(AUID_AAFCommentMarker,IID_IAAFCommentMarker,(IUnknown **)&pComment));
    
//    checkResult(pEvent->SetAnnotation());
    
    cUnknownPtr<IAAFEvent> pEvent;
    pComment.demandQI(IID_IAAFEvent, (void **)&pEvent);
    
    checkResult(pEvent->SetPosition(Tc.framesSinceMidnight()));
    checkResult(pEvent->SetComment(Comment.c_str()));
    
    cUnknownPtr<IAAFComponent> pComponent;
    pEvent.demandQI(IID_IAAFComponent, (void **)&pComponent);
  
    checkResult(pComponent->SetDataDef(pDataDef));

    if(pComponent.isValid())
    {
      checkResult(pSequence->AppendComponent(pComponent));
    }
  }
}
