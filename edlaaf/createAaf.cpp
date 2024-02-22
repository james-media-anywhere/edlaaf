#include "stdafx.h"
#include "cTimeCode.h"
#include "cEdlData.h"
#include "cToken.h"
#include "cLine.h"
#include "cEdl.h"
#include "cAafEdl.h"
#include "cCut.h"
#include "cDisolve.h"
#include "cWipe.h"

string narrow(const wstring & str) throw()
{
  const ctype<wchar_t>& char_facet = _USE(locale(),ctype<wchar_t>);
  string ret;
  ret.reserve(str.size());//avoid incremental allocation
  for(wstring::const_iterator it = str.begin(); it != str.end(); ++it)
  {
    ret += char_facet.widen(*it);
  }
  return ret;
}

cUnknownPtr<IAAFDataDef> cAafEdl::getDataDef(int track)
{
  if(!track)return pPictureDataDef;
  return pSoundDataDef;
}

void cAafEdl::initAaf()
{
  cUnknownPtr<IAAFPluginManager> pPluginMgr;
  checkResult(AAFGetPluginManager(&pPluginMgr));
  checkResult(pPluginMgr->RegisterSharedPlugins());
}

void cAafEdl::createAafFile(wstring filename)
{
  wifstream test;
  test.open(narrow(filename).c_str());
  if(!test.fail())
  {
    throw AAFRESULT_FILE_EXISTS;
  }

  aafProductVersion_t version;
  version.major = 1;
  version.minor = 0;
  version.tertiary = 0;
  version.patchLevel = 0;
  version.type = kAAFVersionDebug;

  aafProductIdentification_t ident;
  ident.companyName = L"AAF example";
  ident.productName = L"edlaaf";
  ident.productVersionString = L"1.0.0.0";
  ident.productID.Data1 = 0x9438acfd;
  ident.productID.Data2 = 0xd233;
  ident.productID.Data3 = 0x41b0;
  ident.productID.Data4[0] = 0x8a;
  ident.productID.Data4[1] = 0x89;
  ident.productID.Data4[2] = 0xb4;
  ident.productID.Data4[3] = 0x48;
  ident.productID.Data4[4] = 0x75;
  ident.productID.Data4[5] = 0xdc;
  ident.productID.Data4[6] = 0x48;
  ident.productID.Data4[7] = 0x4;
  ident.platform = L"MS Windows 2000";
  ident.productVersion = &version;

  // Open the file
  checkResult(AAFFileOpenNewModify(filename.c_str(), 0, &ident, &pFile));

  // We can't really do anthing in AAF without the header.
  checkResult(pFile->GetHeader(&pHeader));

  //The dictionary has information about the type of data that is in the file.
  checkResult(pFile->GetDictionary(&pDictionary));

  assert(pFile && pHeader && pDictionary);
}

cTimeCode getTimeCode(aafRational_t rate,bool dropFrame)
{
  div_t d = div(rate.numerator,rate.denominator);
  if(d.rem != 0) ++d.quot;
  return cTimeCode(0,(eFps)d.quot,dropFrame);
}

void cAafEdl::saveEssence()
{
  cUnknownPtr<IAAFEssenceDescriptor> pEssenceDescriptor;

  // save cassettes with tapeDescriptors
  for(tCassIt cass = cassettes.begin(); cass != cassettes.end(); ++cass)
  {
    wstring cassName = cass->first;
    aafRational_t rate = cass->second->Rate;
    bool dropframe = cass->second->DropFrame;

    if((L"BL" != cassName) && (L"BLK" != cassName) && (L"AX" != cassName)) //do not store blank as a master mob
    {
      cUnknownPtr<IAAFTapeDescriptor> pTapeDescriptor;
      checkResult(pDictionary->CreateInstance(AUID_AAFTapeDescriptor,IID_IAAFTapeDescriptor,(IUnknown **)&pTapeDescriptor));

      // Objects must ALWAYS be initialised before use.
      pTapeDescriptor->Initialize();

      pTapeDescriptor.demandQI(IID_IAAFEssenceDescriptor, (void **)&pEssenceDescriptor);

      // create mobs
      cTimeCode tc = getTimeCode(rate,dropframe);
      cTimeCode in = cTimeCode(0, tc.fps(), tc.isDropFrame()); // assume tape starts at midnight
      cTimeCode out = cTimeCode(tc.getFramesPerDay()-1, tc.fps(), tc.isDropFrame()); // assume tape is 24 hours long

      createEssenceMobs(cassName, cassName, rate, dropframe, in, out, pEssenceDescriptor); // name mobs from tape name
    }
  }
}

void cAafEdl::createEssenceMobs(wstring mobName, wstring cassName, aafRational_t rate, bool dropFrame, cTimeCode in, cTimeCode out, cUnknownPtr<IAAFEssenceDescriptor> pEssenceDescriptor,bool V,const bool* pA)
{
  // Get a Master MOB Interface
  cUnknownPtr<IAAFMasterMob> pMasterMob;
  checkResult(pDictionary->CreateInstance(AUID_AAFMasterMob,IID_IAAFMasterMob,(IUnknown **)&pMasterMob));
  
  //Objects must ALWAYS be initialised before use.
  checkResult(pMasterMob->Initialize());
  
  // Get a Mob interface and set its variables.
  cUnknownPtr<IAAFMob> pMobMast;
  pMasterMob.demandQI(IID_IAAFMob, (void **)&pMobMast);
  
  // Set the master mob name
  checkResult(pMobMast->SetName(mobName.c_str()));
  
  // Add it to the file 
  checkResult(pHeader->AddMob(pMobMast));
  
  // Get a Source MOB Interface
  cUnknownPtr<IAAFSourceMob> pSourceMob;
  checkResult(pDictionary->CreateInstance(AUID_AAFSourceMob,IID_IAAFSourceMob,(IUnknown **)&pSourceMob));
  
  //Objects must ALWAYS be initialised before use.
  checkResult(pSourceMob->Initialize());
  
  // Get a Mob interface and set its variables.
  cUnknownPtr<IAAFMob> pMobSrc;
  pSourceMob.demandQI(IID_IAAFMob, (void **)&pMobSrc);
  
  // Set the master mob name
  checkResult(pMobSrc->SetName(mobName.c_str()));
  
  checkResult(pSourceMob->SetEssenceDescriptor(pEssenceDescriptor));
  
  // Add it to the file 
  checkResult(pHeader->AddMob(pMobSrc));
  
  saveEssenceVA(rate,in,out,pSourceMob,pMobSrc,pMasterMob, pMobMast);
  saveEssenceTC(rate,dropFrame,in,out,pSourceMob,pMobSrc);

  bool channels[1+NUM_AUDIO_CHANNELS];
  channels[0] = V;
  for(int i = 0; i < NUM_AUDIO_CHANNELS; ++i)
  {
    channels[1+i] = pA?pA[i]:true;
  }

  tMobLookup::value_type val(cassName, pMobMast);
  cassetteToMasterMob.insert(val);
}

void cAafEdl::saveEssenceVA(aafRational_t rate, cTimeCode In, cTimeCode Out, cUnknownPtr<IAAFSourceMob> pSourceMob, cUnknownPtr<IAAFMob> pMobSrc, cUnknownPtr<IAAFMasterMob> pMasterMob, cUnknownPtr<IAAFMob> pMobMast)
{
  for(unsigned long int	channel = 0; channel <= NUM_AUDIO_CHANNELS; ++channel)
  {
    //even though alot of these numbers mathematically are related, they have separate semantics.

    aafSlotID_t sourceSlotID = channel+1;
    aafSlotID_t masterSlotID = sourceSlotID;

    aafUInt32 physicalNum = channel; // count up from 1 for each slot type
    if(0==channel) physicalNum = 1;
    
    cUnknownPtr<IAAFTimelineMobSlot> pTimelineMobSlot;
    checkResult(pDictionary->CreateInstance(AUID_AAFTimelineMobSlot,IID_IAAFTimelineMobSlot,(IUnknown **)&pTimelineMobSlot));

    //Objects must ALWAYS be initialised before use.
    checkResult(pTimelineMobSlot->Initialize());

    checkResult(pTimelineMobSlot->SetEditRate(rate));
    checkResult(pTimelineMobSlot->SetOrigin(0));

    cUnknownPtr<IAAFSourceClip> pSourceClip;
    checkResult(pDictionary->CreateInstance(AUID_AAFSourceClip,IID_IAAFSourceClip,(IUnknown **)&pSourceClip));

    aafLength_t length = Out.framesSinceMidnight() - In.framesSinceMidnight();

    aafSourceRef_t srcRef;
    memset(&srcRef,0,sizeof(srcRef));//no predecesor - AAF spec. says null aafSourceRef_t for this.

    //Objects must ALWAYS be initialised before use.
    checkResult(pSourceClip->Initialize(getDataDef(channel),length,srcRef));

    cUnknownPtr<IAAFSegment> pSegment;
    pSourceClip.demandQI(IID_IAAFSegment, (void **)&pSegment);

    cUnknownPtr<IAAFMobSlot> pMobSlot;
    pTimelineMobSlot.demandQI(IID_IAAFMobSlot, (void **)&pMobSlot);

    checkResult(pMobSlot->SetSegment(pSegment));
    checkResult(pMobSlot->SetPhysicalNum(physicalNum));
    checkResult(pMobSlot->SetName(L""));

    checkResult(pMobSlot->SetSlotID(sourceSlotID));

    checkResult(pMobSrc->AppendSlot(pMobSlot));

    checkResult(pMasterMob->AddMasterSlot(getDataDef(channel),sourceSlotID,pSourceMob,masterSlotID,L""));

    cUnknownPtr<IAAFMobSlot> pMastMobSlot;
    checkResult(pMobMast->LookupSlot(masterSlotID,&pMastMobSlot));

    checkResult(pMastMobSlot->SetPhysicalNum(physicalNum));
  }
}

void cAafEdl::saveEssenceTC(aafRational_t rate, bool dropFrame, cTimeCode In, cTimeCode Out, cUnknownPtr<IAAFSourceMob> pSourceMob, cUnknownPtr<IAAFMob> pMobSrc)
{
  //even though alot of these numbers mathematically are related, they have separate semantics.

  aafSlotID_t channel = NUM_AUDIO_CHANNELS+1;

  aafSlotID_t sourceSlotID = channel+1;
  aafSlotID_t masterSlotID = sourceSlotID;

  aafUInt32 physicalNum = 1; // count up from 1 for each slot type

  //add a timecode object
  cUnknownPtr<IAAFTimelineMobSlot> pTimelineMobSlot;
  checkResult(pDictionary->CreateInstance(AUID_AAFTimelineMobSlot,IID_IAAFTimelineMobSlot,(IUnknown **)&pTimelineMobSlot));

  //Objects must ALWAYS be initialised before use.
  checkResult(pTimelineMobSlot->Initialize());

  checkResult(pTimelineMobSlot->SetEditRate(rate));
  checkResult(pTimelineMobSlot->SetOrigin(0));

  cUnknownPtr<IAAFMobSlot> pMobSlot;
  pTimelineMobSlot.demandQI(IID_IAAFMobSlot, (void **)&pMobSlot);

  cUnknownPtr<IAAFTimecode> pTimecode;
  checkResult(pDictionary->CreateInstance(AUID_AAFTimecode,IID_IAAFTimecode,(IUnknown **)&pTimecode));

  //Objects must ALWAYS be initialised before use.
  aafLength_t length = Out.framesSinceMidnight() - In.framesSinceMidnight();
  cTimeCode timecode = getTimeCode(rate,dropFrame);
  aafTimecode_t tc;
  tc.startFrame = In.framesSinceMidnight();
  tc.drop = timecode.isDropFrame()?1:0;
  tc.fps = timecode.fps();
  checkResult(pTimecode->Initialize(length,&tc));

  cUnknownPtr<IAAFSegment> pSegment;
  pTimecode.demandQI(IID_IAAFSegment, (void **)&pSegment);

  checkResult(pMobSlot->SetSegment(pSegment));
  checkResult(pMobSlot->SetPhysicalNum(physicalNum));
  checkResult(pMobSlot->SetName(L""));
  checkResult(pMobSlot->SetSlotID(sourceSlotID));

  checkResult(pMobSrc->AppendSlot(pMobSlot));
}

void cAafEdl::removeZeroLengthCuts()
{
  // first dissolve/wipe cannot appear before 3rd line of edl if unwanted zero length cut is present 
  {
    for(cTimelineTrack::tList::iterator it = Video.Segments.begin(); it != Video.Segments.end(); ++it)
    {
      cTimelineTrack::tList::iterator next = it;
      ++next;
      if(next != Video.Segments.end())
      {
        cTimelineTrack::tList::iterator nextnext = next;
        ++nextnext;
        if(nextnext != Video.Segments.end())
        {
          bool removeElem;
          removeElem = (*nextnext)->removeZeroLengthCut(*it,*next);
          if (removeElem) Video.Segments.erase(next);
        }
      }
    }
  }
  for(int i = 0; i < NUM_AUDIO_CHANNELS; ++i)
  {
    for(cTimelineTrack::tList::iterator it = Audio[i].Segments.begin(); it != Audio[i].Segments.end(); ++it)
    {
      cTimelineTrack::tList::iterator next = it;
      ++next;
      if(next != Audio[i].Segments.end())
      {
        cTimelineTrack::tList::iterator nextnext = next;
        ++nextnext;
        if(nextnext != Audio[i].Segments.end())
        {
          bool removeElem;
          removeElem = (*nextnext)->removeZeroLengthCut(*it,*next);
          if (removeElem) Audio[i].Segments.erase(next);
        }
      }
    }
  }
}

void cAafEdl::adjustComposition()
{
  // extend tail of segment before transition 
  {
    for(cTimelineTrack::tList::iterator it = Video.Segments.begin(); it != Video.Segments.end(); ++it)
    {
      cTimelineTrack::tList::iterator next = it;
      ++next;
      if(next != Video.Segments.end())
      {
        (*next)->adjust(*it,false);
      }
    }
  }
  for(int i = 0; i < NUM_AUDIO_CHANNELS; ++i)
  {
    for(cTimelineTrack::tList::iterator it = Audio[i].Segments.begin(); it != Audio[i].Segments.end(); ++it)
    {
      cTimelineTrack::tList::iterator next = it;
      ++next;
      if(next != Audio[i].Segments.end())
      {
        (*next)->adjust(*it,true);
      }
    }
  }
}

void cAafEdl::saveComposition()
{
  // Get a Master MOB Interface
  cUnknownPtr<IAAFCompositionMob> pCompositionMob;
  checkResult(pDictionary->CreateInstance(AUID_AAFCompositionMob,IID_IAAFCompositionMob,(IUnknown **)&pCompositionMob));

  //Objects must ALWAYS be initialised before use.
  checkResult(pCompositionMob->Initialize(Title.c_str()));//EDL title becomes the compositions title

  // Get a Mob interface and set its variables.
  cUnknownPtr<IAAFMob> pMobComp;
  pCompositionMob.demandQI(IID_IAAFMob, (void **)&pMobComp);

  // Add it to the file 
  checkResult(pHeader->AddMob(pMobComp));

  aafRational_t rate = getFrameRate();

  saveCompositionVA(pMobComp, rate);
  saveCompositionTC(pMobComp, rate);
  saveCompositionComments(pMobComp, rate);
}

void cAafEdl::saveCompositionVA(cUnknownPtr<IAAFMob> pMobComp, aafRational_t rate)
{
  for(aafSlotID_t channel = 0; channel <= NUM_AUDIO_CHANNELS; ++channel)
  {
    //even though alot of these numbers mathematically are related, they have separate semantics.

    aafSlotID_t compositionSlotID = channel + 1;
    aafSlotID_t masterSlotID = compositionSlotID;
    
    aafUInt32 physicalNum = channel; // count up from 1 for each slot type
    if(0==channel) physicalNum = 1;
    
    cUnknownPtr<IAAFSequence> pSequence;
    checkResult(pDictionary->CreateInstance(AUID_AAFSequence,IID_IAAFSequence,(IUnknown **)&pSequence));

    //Objects must ALWAYS be initialised before use.
    checkResult(pSequence->Initialize(getDataDef(channel)));

    cTimelineTrack* pTrack = 0;
    if(0==channel) pTrack = &Video;
    else pTrack = &Audio[channel-1];

    if (pTrack->Segments.empty())
    {
      aafRational_t speedRatio = {1000, 1000};
      cSegment::Ptr dummySeg = new cCut;
      dummySeg->set(0,L"",cTimeCode(),TimelineDestIn,TimelineDestOut,speedRatio);
      dummySeg->createFiller(pDictionary,getDataDef(channel),pSequence);
      delete dummySeg;
      dummySeg = 0;
    }
    else
    {
      for(cTimelineTrack::tList::iterator it = pTrack->Segments.begin(); it != pTrack->Segments.end(); ++it)
      {
        cSegment::Ptr seg = *it;
        if(L"BL" == seg->cassette() || L"BLK" == seg->cassette() || L"AX" == seg->cassette())
        {
          seg->createFiller(pDictionary,getDataDef(channel),pSequence);
        }
        else
        {
          tMobLookup::iterator mob = cassetteToMasterMob.find(seg->cassette());
          assert(mob != cassetteToMasterMob.end());
          seg->createComponent(mob->second,masterSlotID,pDictionary,getDataDef(channel),pSequence);
        }
      }
    }

    cUnknownPtr<IAAFSegment> pSegment;
    pSequence.demandQI(IID_IAAFSegment, (void **)&pSegment);

    cUnknownPtr<IAAFTimelineMobSlot> pTimelineMobSlot;
    checkResult(pMobComp->AppendNewTimelineSlot(rate,pSegment,compositionSlotID,L"",0,&pTimelineMobSlot));

    cUnknownPtr<IAAFMobSlot> pMobSlot;
    pTimelineMobSlot.demandQI(IID_IAAFMobSlot, (void **)&pMobSlot);

    checkResult(pMobSlot->SetPhysicalNum(physicalNum));
  }
}

void cAafEdl::saveCompositionTC(cUnknownPtr<IAAFMob> pMobComp, aafRational_t rate)
{
  if(TimelineDestIn != TimelineDestOut) // add the destination timecode track
  {
    //even though alot of these numbers mathematically are related, they have separate semantics.

    aafSlotID_t channel = NUM_AUDIO_CHANNELS + 1;
    aafSlotID_t compositionSlotID = channel + 1;
    
    aafUInt32 physicalNum = 1; // count up from 1 for each slot type
    
    cUnknownPtr<IAAFTimecode> pTimecode;
    checkResult(pDictionary->CreateInstance(AUID_AAFTimecode,IID_IAAFTimecode,(IUnknown **)&pTimecode));
    
    aafTimecode_t tc;
    tc.startFrame = TimelineDestIn.framesSinceMidnight();
    tc.fps = (aafUInt16)TimelineDestIn.fps();
    tc.drop = TimelineDestIn.isDropFrame() ? 1 : 0;
    checkResult(pTimecode->Initialize(TimelineDestOut.framesSinceMidnight() - TimelineDestIn.framesSinceMidnight(),&tc));
    
    cUnknownPtr<IAAFSegment> pSegment;
    pTimecode.demandQI(IID_IAAFSegment, (void **)&pSegment);
    
    cUnknownPtr<IAAFTimelineMobSlot> pTimelineMobSlot;
    checkResult(pMobComp->AppendNewTimelineSlot(rate,pSegment,compositionSlotID,L"Timecode",0,&pTimelineMobSlot));
    
    cUnknownPtr<IAAFMobSlot> pMobSlot;
    pTimelineMobSlot.demandQI(IID_IAAFMobSlot, (void **)&pMobSlot);
    
    checkResult(pMobSlot->SetPhysicalNum(physicalNum));
  }
}

void cAafEdl::saveCompositionComments(cUnknownPtr<IAAFMob> pMobComp, aafRational_t rate)
{
  if (CommentTrack.Events.size() > 0) // add comments
  {
    aafSlotID_t channel = NUM_AUDIO_CHANNELS + 2;
    aafSlotID_t compositionSlotID = channel + 1;
    
    aafUInt32 physicalNum = 1; // count up from 1 for each slot type
    
    cUnknownPtr<IAAFSequence> pSequence;
    checkResult(pDictionary->CreateInstance(AUID_AAFSequence,IID_IAAFSequence,(IUnknown **)&pSequence));
    
    //Objects must ALWAYS be initialised before use.
    checkResult(pSequence->Initialize(pPictureDataDef));
    
    std::greater<cEvent::Ptr> sortOrder;
    CommentTrack.Events.sort(sortOrder);//events MUST be sorted on timecode otherwise AAF SDK asserts.
    for(cEventTrack::tList::iterator it = CommentTrack.Events.begin(); it != CommentTrack.Events.end(); ++it)
    {
      cEvent::Ptr event = *it;
      event->createComment(pDictionary,pPictureDataDef,pSequence);
    }
    
    cUnknownPtr<IAAFSegment> pSegment;
    pSequence.demandQI(IID_IAAFSegment, (void **)&pSegment);
    
    cUnknownPtr<IAAFEventMobSlot> pEventMobSlot; // no initialise ???
    checkResult(pDictionary->CreateInstance(AUID_AAFEventMobSlot,IID_IAAFEventMobSlot,(IUnknown **)&pEventMobSlot));
    
    checkResult(pEventMobSlot->SetEditRate(&rate));
    
    cUnknownPtr<IAAFMobSlot> pMobSlot;
    pEventMobSlot.demandQI(IID_IAAFMobSlot, (void **)&pMobSlot);
    
    checkResult(pMobSlot->SetName(L"Comments"));
    checkResult(pMobSlot->SetPhysicalNum(physicalNum));
    checkResult(pMobSlot->SetSlotID(compositionSlotID));
    checkResult(pMobSlot->SetSegment(pSegment));
    
    checkResult(pMobComp->AppendSlot(pMobSlot));
  }
}

void cAafEdl::closeFile()
{
  checkResult(pFile->Save());
  checkResult(pFile->Close());
}

void cAafEdl::createAaf(wstring filename)
{
  createAafFile(filename);

  checkResult(pDictionary->LookupDataDef(DDEF_Picture,&pPictureDataDef));
  checkResult(pDictionary->LookupDataDef(DDEF_Sound,&pSoundDataDef));
  checkResult(pDictionary->LookupDataDef(DDEF_Timecode,&pTimecodeDataDef));

  saveEssence();

  removeZeroLengthCuts();

  adjustComposition();

  saveComposition();

  closeFile();
}

