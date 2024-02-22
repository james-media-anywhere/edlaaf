#include "stdafx.h"
#include "cTimeCode.h"
#include "cSegment.h"
#include "math.h"

cSegment::cSegment()
{
  OutFadeDuration = 0;
}

cSegment::~cSegment()
{
}

void cSegment::set(int edit,wstring cassette,cTimeCode in,cTimeCode destIn,cTimeCode destOut,aafRational_t speedRatio) 
{
  Edit = edit;
  Cassette = cassette;
  In = in;
  DestIn = destIn;
  DestOut = destOut;
  SpeedRatio = speedRatio;
}

wstring cSegment::cassette() 
{
  return Cassette;
}

cTimeCode cSegment::in() 
{
  return In;
}

cTimeCode cSegment::out() 
{
  cTimeCode out = In;
  aafRational_t speed = speedRatio();
  int numerator = abs(speed.numerator);//may be negative - but length always positive
  out += length() * numerator / speed.denominator;
  return out;
}

cTimeCode cSegment::destIn()
{
  return DestIn;
}

cTimeCode cSegment::destOut()
{
  return DestOut;
}

int cSegment::length()
{
  return DestOut.framesSinceMidnight() - DestIn.framesSinceMidnight();
}

aafRational_t cSegment::speedRatio()
{
  return SpeedRatio;
}

void cSegment::setSpeedRatio(aafRational_t speedRatio)
{
  SpeedRatio = speedRatio;
}

void cSegment::adjustSrcIn(int diff)
{
  In += diff;
}

void cSegment::adjustDestIn(int diff)
{
  DestIn += diff;
}

void cSegment::adjustDestOut(int diff)
{
  DestOut += diff;
}

void cSegment::edit()
{
  wcout << setw(3) << setfill(L'0') << Edit;
}

void cSegment::frames()
{
  wcout << setw(8) << setfill(L' ') << Cassette;
  wcout << L" " << in() << L" " << out();
  wcout << L" " << destIn() << L" " << destOut();
  wcout << L"\n";
}

int cSegment::getEditNumber()
{
  return Edit;
}

int cSegment::getOutFade()
{
  return OutFadeDuration;
}

void cSegment::setOutFade(int duration)
{
  OutFadeDuration = duration;
}

const aafUID_t kEditNameUID = 
{ 0xfedcba98, 0x2267, 0x11d3, 
{ 0x8a, 0x4c, 0x0, 0x50, 0x4, 0xe, 0xf7, 0xd2 } };

const wchar_t* kEditNameString = L"EditName";

static void RegisterEditName(
  cUnknownPtr<IAAFDictionary> pDictionary)
{
  cUnknownPtr<IAAFClassDef> pCDComponent;

  // Get the class def for AAFComponent
  checkResult(pDictionary->LookupClassDef(
    AUID_AAFComponent,
    &pCDComponent));

  // Get the Unicode String type out of the dictionary
  cUnknownPtr<IAAFTypeDef> pStringTypeDef;
  checkResult(pDictionary->LookupTypeDef(
    kAAFTypeID_String,
		&pStringTypeDef));

  cUnknownPtr<IAAFPropertyDef> pJunk; //not required
  checkResult(pCDComponent->RegisterOptionalPropertyDef(
    kEditNameUID,
    kEditNameString,
    pStringTypeDef,
    &pJunk));
}

static void setEditName(cUnknownPtr<IAAFDictionary> pDictionary,
						cUnknownPtr<IAAFComponent> pComponent,
						wstring editName)
{
  cUnknownPtr<IAAFClassDef> pCDComponent;
  checkResult(pDictionary->LookupClassDef(AUID_AAFComponent,&pCDComponent));

  cUnknownPtr<IAAFPropertyDef> pPDComponentEditName;
  if(FAILED(pCDComponent->LookupPropertyDef(kEditNameUID,&pPDComponentEditName)))
  {
    RegisterEditName(pDictionary);
    checkResult(pCDComponent->LookupPropertyDef(kEditNameUID,&pPDComponentEditName));
  }

  cUnknownPtr<IAAFObject> pObj;
  pComponent.demandQI(IID_IAAFObject,(void**)&pObj);

  aafBool present;
  checkResult(pObj->IsPropertyPresent(pPDComponentEditName,&present));

  // make sure it's *NOT* there
  if(present)
    throw AAFRESULT_PROP_ALREADY_PRESENT;

  // Create a property value, through the type def, which contains our
  // edit name value.  
  // First get the type def.
  //
  cUnknownPtr<IAAFTypeDef> pTypeDef;
  checkResult(pDictionary->LookupTypeDef(kAAFTypeID_String,&pTypeDef));

  cUnknownPtr<IAAFTypeDefString> pTypeDefString;
  pTypeDef.demandQI(IID_IAAFTypeDefString,(void**)&pTypeDefString);

  // Now create a property value object with that value.
  aafUInt32 numBytes = (editName.size()+1)*2;
  cUnknownPtr<IAAFPropertyValue> pEditNameVal;
  checkResult(pTypeDefString->CreateValueFromCString(
                 (aafMemPtr_t) editName.c_str(),
                 numBytes,
                 &pEditNameVal));

  checkResult(pObj->SetPropertyValue(pPDComponentEditName,pEditNameVal));

  // The 'set' above should have added the optional property to the
  // object, and set its value.  Verify that the optional property is
  // now present in the object.
  //
  checkResult(pObj->IsPropertyPresent(pPDComponentEditName,&present));

  // make sure it's there
  if(!present)
    throw AAFRESULT_PROP_NOT_PRESENT;
}

void cSegment::setEditNumber(cUnknownPtr<IAAFDictionary> pDictionary,cUnknownPtr<IAAFComponent> pComponent,int editNum)
{
  wstringstream str;
  str << editNum;

  setEditName(pDictionary,pComponent,str.str());
}

cUnknownPtr<IAAFComponent> cSegment::createSourceClip(
                 cUnknownPtr<IAAFDictionary> pDictionary,
                 cUnknownPtr<IAAFDataDef> pDataDef,
                 cUnknownPtr<IAAFMob> pMob, 
                 aafSlotID_t masterSlotID, 
                 cTimeCode srcIn, 
                 cTimeCode destIn, 
                 cTimeCode destOut,
                 int inFade,
                 int outFade,
                 int editNum,
                 aafRational_t speedRatio)
{
  cUnknownPtr<IAAFComponent> pComponent;
  if(destOut.framesSinceMidnight() > destIn.framesSinceMidnight())//ignore zero length cuts
  {
    cUnknownPtr<IAAFSourceClip> pSourceClip;
    checkResult(pDictionary->CreateInstance(AUID_AAFSourceClip,IID_IAAFSourceClip,(IUnknown **)&pSourceClip));

    aafPosition_t start = srcIn.framesSinceMidnight();

    aafLength_t length = 1;//default to 1 frame 
    if(0 != speedRatio.numerator)
    {
      int numerator = abs(speedRatio.numerator);//may be negative - but length always positive
      length = ((destOut.framesSinceMidnight() - destIn.framesSinceMidnight()) * numerator) / speedRatio.denominator;
    }

    if(0 > speedRatio.numerator)
    {
      //The CMX3600 EDL format takes the source in frame as the first frame to be PLAYED.
      //Therefore a reverse stretch will require material ENDING at srcIn!!!
      start -= length;
    }
    
    aafSourceRef_t srcRef;
    pMob->GetMobID(&srcRef.sourceID);
    srcRef.sourceSlotID = masterSlotID;
    srcRef.startTime = start;

    checkResult(pSourceClip->Initialize(pDataDef,length,srcRef));

    aafBoolean_t IsSound;
    pDataDef->IsSoundKind(&IsSound);
    if(IsSound)
    {
      pSourceClip->SetFade( inFade,inFade?kAAFFadeLinearAmp:kAAFFadeNone,
                            outFade,outFade?kAAFFadeLinearAmp:kAAFFadeNone);
    }

    if(speedRatio.numerator != speedRatio.denominator)
    {
      //Component is an OperationGroup that points to the SourceClip
      pComponent = createTimeWarp(pDictionary,pDataDef,pSourceClip,speedRatio);
    }
    else
    {
      pSourceClip.demandQI(IID_IAAFComponent, (void **)&pComponent);
    }

    setEditNumber(pDictionary,pComponent,editNum);
  }
  return pComponent;
}

cUnknownPtr<IAAFComponent> cSegment::createTimeWarp(
                 cUnknownPtr<IAAFDictionary> pDictionary,
                 cUnknownPtr<IAAFDataDef> pDataDef,
                 cUnknownPtr<IAAFSourceClip> pSourceClip,
                 aafRational_t speedRatio)
{
  cUnknownPtr<IAAFComponent> pComponent;

  aafLength_t len = length();
  if(len>0)//ignore zero length cuts
  {
    cUnknownPtr<IAAFOperationDef> pOperationDef;
    cUnknownPtr<IAAFParameterDef> pParameterDef;

    GetVideoSpeedControl(pDictionary,pDataDef,kAAFEffectVideoSpeedControl,&pOperationDef);
    GetVideoSpeedControlParam(pDictionary,pDataDef,&pParameterDef);

    cUnknownPtr<IAAFConstantValue> pConstantValue; 
    checkResult(pDictionary->CreateInstance(AUID_AAFConstantValue,IID_IAAFConstantValue,(IUnknown **)&pConstantValue));
    checkResult(pConstantValue->Initialize(pParameterDef,sizeof(aafRational_t),(unsigned char*)&speedRatio));

    cUnknownPtr<IAAFOperationGroup> pOperationGroup;
    checkResult(pDictionary->CreateInstance(AUID_AAFOperationGroup,IID_IAAFOperationGroup,(IUnknown **)&pOperationGroup));
    checkResult(pOperationGroup->Initialize(pDataDef,len,pOperationDef));

    cUnknownPtr<IAAFParameter> pParameter;
    pConstantValue.demandQI(IID_IAAFParameter,(void**)&pParameter);
    checkResult(pOperationGroup->AddParameter(pParameter));

    cUnknownPtr<IAAFSegment> pSourceSegment;
    pSourceClip.demandQI(IID_IAAFSegment, (void **)&pSourceSegment);
    checkResult(pOperationGroup->AppendInputSegment(pSourceSegment));

    pOperationGroup.demandQI(IID_IAAFComponent, (void **)&pComponent);
  }

  return pComponent;
}

cUnknownPtr<IAAFComponent> cSegment::createFiller(
                 cUnknownPtr<IAAFDictionary> pDictionary,
                 cUnknownPtr<IAAFDataDef> pDataDef,
                 cTimeCode destIn, 
                 cTimeCode destOut)
{
  cUnknownPtr<IAAFComponent> pComponent;
  if(destOut.framesSinceMidnight() > destIn.framesSinceMidnight())//ignore zero length cuts
  {
    cUnknownPtr<IAAFFiller> pFiller;
    checkResult(pDictionary->CreateInstance(AUID_AAFFiller,IID_IAAFFiller,(IUnknown **)&pFiller));

    aafLength_t length = destOut.framesSinceMidnight() - destIn.framesSinceMidnight();
    checkResult(pFiller->Initialize(pDataDef,length));

    pFiller.demandQI(IID_IAAFComponent, (void **)&pComponent);
  }
  return pComponent;
}

std::wstring getName(cUnknownPtr<IAAFDefObject> ptr)
{
  std::wstring ret;
	aafUInt32 bufSize;
	if(ptr.isValid() && SUCCEEDED(ptr->GetNameBufLen (&bufSize)))
	{
    aafCharacter* buffer = new aafCharacter[bufSize/2];
    if(SUCCEEDED(ptr->GetName(buffer, bufSize)))
    {
      ret = buffer;
    }
    delete [] buffer;
  }
  return ret;
}

void cSegment::GetDisolve(cUnknownPtr<IAAFDictionary> pDictionary,
                     cUnknownPtr<IAAFDataDef> pDataDef,
                     const aafUID_t& uid,
                     IAAFOperationDef ** ppOpDef)
{
  // Look in the dictionary to find if the effect Definition exists
  // if it exists use it.
  if (FAILED(pDictionary->LookupOperationDef(uid, ppOpDef)))
  {
    cUnknownPtr<IAAFOperationDef> pEffectDef;
    checkResult(pDictionary->CreateInstance(AUID_AAFOperationDef, IID_IAAFOperationDef,(IUnknown **) &pEffectDef));

    cUnknownPtr<IAAFDefObject> pDefObject;
    pEffectDef.demandQI(IID_IAAFDefObject, (void **)&pDefObject);
    checkResult(pDefObject->Initialize(uid, L"Dissolve"));

    cUnknownPtr<IAAFDefObject> DefObject;
    pDataDef.demandQI(IID_IAAFDefObject,(void**)&DefObject);
    std::wstring dataName = getName(DefObject);

    std::wstring description = L"Dissolve between neighbouring ";
    description += dataName;
    description += L" segments.";
    checkResult(pDefObject->SetDescription(description.c_str()));

    checkResult(pEffectDef->SetIsTimeWarp((aafBool)0));//does not affect time
    checkResult(pEffectDef->SetCategory(uid));
    checkResult(pEffectDef->SetBypass((aafUInt32 )1));//use next material on bypass
    checkResult(pEffectDef->SetNumberInputs(2));//disolve takes two source clips
    checkResult(pEffectDef->SetDataDef(pDataDef));//affects pictures / sound etc.

    checkResult(pDictionary->RegisterOperationDef(pEffectDef));
  }
  checkResult(pDictionary->LookupOperationDef(uid, ppOpDef));
}

void cSegment::GetWipeParamSMPTE(cUnknownPtr<IAAFDictionary> pDictionary,
                  cUnknownPtr<IAAFDataDef> pDataDef,
                  IAAFParameterDef** ppParamDef)
{
  const aafUID_t& uid = kAAFParameterDefSMPTEWipeNumber;
  // Look in the dictionary to find if the effect Definition exists
  // if it exists use it.
  if (FAILED(pDictionary->LookupParameterDef(uid, ppParamDef)))
  {
    cUnknownPtr<IAAFTypeDef> pTypeDef;
    checkResult(pDictionary->LookupTypeDef(kAAFTypeID_Int32,&pTypeDef));

    cUnknownPtr<IAAFParameterDef> pParamDef;
    checkResult(pDictionary->CreateInstance(AUID_AAFParameterDef, IID_IAAFParameterDef,(IUnknown **) &pParamDef));
    checkResult(pParamDef->Initialize(uid,L"SMPTE Wipe Number",L"This number repesents the type of Wipe, as defined by SMPTE",pTypeDef));

    checkResult(pDictionary->RegisterParameterDef(pParamDef));
  }
  checkResult(pDictionary->LookupParameterDef(uid, ppParamDef));
}

void cSegment::GetWipeParamReverse(cUnknownPtr<IAAFDictionary> pDictionary,
                  cUnknownPtr<IAAFDataDef> pDataDef,
                  IAAFParameterDef** ppParamDef)
{
  const aafUID_t& uid = kAAFParameterDefSMPTEReverse;
  // Look in the dictionary to find if the effect Definition exists
  // if it exists use it.
  if (FAILED(pDictionary->LookupParameterDef(uid, ppParamDef)))
  {
    cUnknownPtr<IAAFTypeDef> pTypeDef;
    checkResult(pDictionary->LookupTypeDef(kAAFTypeID_Boolean,&pTypeDef));

    cUnknownPtr<IAAFParameterDef> pParamDef;
    checkResult(pDictionary->CreateInstance(AUID_AAFParameterDef, IID_IAAFParameterDef,(IUnknown **) &pParamDef));
    checkResult(pParamDef->Initialize(uid,L"SMPTE Reverse",L"This boolean repesents the direction of the Wipe, as defined by SMPTE",pTypeDef));

    checkResult(pDictionary->RegisterParameterDef(pParamDef));
  }
  checkResult(pDictionary->LookupParameterDef(uid, ppParamDef));
}

void cSegment::GetWipe(cUnknownPtr<IAAFDictionary> pDictionary,
                     cUnknownPtr<IAAFDataDef> pDataDef,
                     IAAFOperationDef ** ppOpDef)
{
  const aafUID_t& uid = kAAFEffectSMPTEVideoWipe;
  // Look in the dictionary to find if the effect Definition exists
  // if it exists use it.
  if (FAILED(pDictionary->LookupOperationDef(uid, ppOpDef)))
  {
    cUnknownPtr<IAAFOperationDef> pEffectDef;
    checkResult(pDictionary->CreateInstance(AUID_AAFOperationDef, IID_IAAFOperationDef,(IUnknown **) &pEffectDef));

    cUnknownPtr<IAAFDefObject> pDefObject;
    pEffectDef.demandQI(IID_IAAFDefObject, (void **)&pDefObject);
    checkResult(pDefObject->Initialize(uid, L"SMPTE Wipe"));
    std::wstring description = L"Wipe between neighbouring picture segments";
    checkResult(pDefObject->SetDescription(description.c_str()));

    checkResult(pEffectDef->SetIsTimeWarp((aafBool)0));//does not affect time
    checkResult(pEffectDef->SetCategory(uid));
    checkResult(pEffectDef->SetBypass((aafUInt32 )1));//use next material on bypass
    checkResult(pEffectDef->SetNumberInputs(2));//wipe takes two source clips
    checkResult(pEffectDef->SetDataDef(pDataDef));//affects pictures / sound etc.

    cUnknownPtr<IAAFParameterDef> pParameterDefSMPTE;
    GetWipeParamSMPTE(pDictionary,pDataDef,&pParameterDefSMPTE);
    checkResult(pEffectDef->AddParameterDef(pParameterDefSMPTE));

    cUnknownPtr<IAAFParameterDef> pParameterDefReverse;
    GetWipeParamReverse(pDictionary,pDataDef,&pParameterDefReverse);
    checkResult(pEffectDef->AddParameterDef(pParameterDefReverse));

    checkResult(pDictionary->RegisterOperationDef(pEffectDef));
  }
  checkResult(pDictionary->LookupOperationDef(uid, ppOpDef));
}

void cSegment::GetVideoSpeedControlParam(cUnknownPtr<IAAFDictionary> pDictionary,
                               cUnknownPtr<IAAFDataDef> pDataDef,
                               IAAFParameterDef** ppParamDef)
{
  const aafUID_t& uid = kAAFParameterDefSpeedRatio;
  // Look in the dictionary to find if the effect Definition exists
  // if it exists use it.
  if (FAILED(pDictionary->LookupParameterDef(uid, ppParamDef)))
  {
    cUnknownPtr<IAAFTypeDef> pTypeDef;
    checkResult(pDictionary->LookupTypeDef(kAAFTypeID_Rational,&pTypeDef));

    cUnknownPtr<IAAFParameterDef> pParamDef;
    checkResult(pDictionary->CreateInstance(AUID_AAFParameterDef, IID_IAAFParameterDef,(IUnknown **) &pParamDef));
    checkResult(pParamDef->Initialize(uid,L"Speed ratio",L"This rational number repesents the relative video frame rate",pTypeDef));

    checkResult(pDictionary->RegisterParameterDef(pParamDef));
  }
  checkResult(pDictionary->LookupParameterDef(uid, ppParamDef));
}

void cSegment::GetVideoSpeedControl(cUnknownPtr<IAAFDictionary> pDictionary,
                          cUnknownPtr<IAAFDataDef> pDataDef,
                          const aafUID_t& uid,
                          IAAFOperationDef ** ppOpDef)
{
  // Look in the dictionary to find if the effect Definition exists
  // if it exists use it.
  if (FAILED(pDictionary->LookupOperationDef(uid, ppOpDef)))
  {
    cUnknownPtr<IAAFOperationDef> pEffectDef;
    checkResult(pDictionary->CreateInstance(AUID_AAFOperationDef, IID_IAAFOperationDef,(IUnknown **) &pEffectDef));

    cUnknownPtr<IAAFDefObject> pDefObject;
    pEffectDef.demandQI(IID_IAAFDefObject, (void **)&pDefObject);
    checkResult(pDefObject->Initialize(uid, L"VideoSpeedControl"));

    cUnknownPtr<IAAFDefObject> DefObject;
    pDataDef.demandQI(IID_IAAFDefObject,(void**)&DefObject);
    std::wstring dataName = getName(DefObject);

    std::wstring description = L"adjust frame rate (non-interpolating) for ";
    description += dataName;
    description += L" segments.";
    checkResult(pDefObject->SetDescription(description.c_str()));

    checkResult(pEffectDef->SetIsTimeWarp((aafBool)1)); //does affect time
    checkResult(pEffectDef->SetCategory(uid));
    checkResult(pEffectDef->SetNumberInputs(1));        //time warp takes one source clip
    checkResult(pEffectDef->SetDataDef(pDataDef));

    cUnknownPtr<IAAFParameterDef> pParameterDef;
    GetVideoSpeedControlParam(pDictionary,pDataDef,&pParameterDef);
    checkResult(pEffectDef->AddParameterDef(pParameterDef));

    checkResult(pDictionary->RegisterOperationDef(pEffectDef));
  }
  checkResult(pDictionary->LookupOperationDef(uid, ppOpDef));
}


void cSegment::createComponent(cUnknownPtr<IAAFMob> pMob,int track,cUnknownPtr<IAAFDictionary> pDictionary,cUnknownPtr<IAAFDataDef> pDataDef,cUnknownPtr<IAAFSequence> pSequence)
{
  if(length() > 0)//ignore zero length 
  {
    cUnknownPtr<IAAFComponent> pComponentTransition = 
      createTransition(pDictionary,pDataDef);

    if(pComponentTransition.isValid())
    {
      checkResult(pSequence->AppendComponent(pComponentTransition));
    }

    cUnknownPtr<IAAFComponent> pComponentSourceClip = 
      cSegment::createSourceClip(pDictionary,pDataDef,pMob,track,in(),destIn(),destOut(),getTransitionDuration(),getOutFade(),getEditNumber(),speedRatio());

    if(pComponentSourceClip.isValid())
    {
      checkResult(pSequence->AppendComponent(pComponentSourceClip));
    }
  }
}

void cSegment::createFiller(cUnknownPtr<IAAFDictionary> pDictionary,cUnknownPtr<IAAFDataDef> pDataDef,cUnknownPtr<IAAFSequence> pSequence)
{
  if(length() > 0)//ignore zero length 
  {
    cUnknownPtr<IAAFComponent> pComponentTransition = 
      createTransition(pDictionary,pDataDef);

    if(pComponentTransition.isValid())
    {
      checkResult(pSequence->AppendComponent(pComponentTransition));
    }

    cUnknownPtr<IAAFComponent> pComponentFiller = 
      cSegment::createFiller(pDictionary,pDataDef,destIn(),destOut());

    if(pComponentFiller.isValid())
    {
      checkResult(pSequence->AppendComponent(pComponentFiller));
    }
  }
}
