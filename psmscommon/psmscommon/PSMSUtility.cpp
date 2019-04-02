#if defined(_WIN32)
#else //_WIN32
#  include <stdlib.h>
#  include <string.h>
#endif //_WIN32

#include <assert.h>
#include <iostream>
#include <sstream>
#include "PSMSUtility.h"

namespace psmscommon
{

iks* NewMessage(const char* pszType,
                const char* pszSeq, 
                const char* pszFrom, 
                const char* pszTo,
                const char* pszModule,
                const char* pszDomain)
{
    iks* pnMessage = iks_new(TAG_MESSAGE);
    do 
    {
        if (!pnMessage) break;

        if (!pszType) break;

        bool bUseSeq = false;
        if (strcmp(pszType, VALUE_REQUEST) == 0 
         || strcmp(pszType, VALUE_RESPONSE) == 0)
        {
            bUseSeq = true;
        }

        iks_insert_attrib(pnMessage, ATTRIBUTE_TYPE, pszType);
        if (bUseSeq)
        {
            iks_insert_attrib(pnMessage, ATTRIBUTE_SEQ, pszSeq);
        }

        if (pszFrom && (strlen(pszFrom) > 0))
        {
            iks_insert_attrib(pnMessage, ATTRIBUTE_FROM, pszFrom);
        }

        if (pszTo && (strlen(pszTo) > 0))
        {
            iks_insert_attrib(pnMessage, ATTRIBUTE_TO, pszTo);
        }

        if (pszModule && (strlen(pszModule) > 0))
        {
            iks_insert_attrib(pnMessage, ATTRIBUTE_MODULE, pszModule);
        }

        if (pszDomain && (strlen(pszDomain) > 0))
        {
            iks_insert_attrib(pnMessage, ATTRIBUTE_DOMAIN, pszDomain);
        }
    }while(0);

    return pnMessage;
}

iks* NewRequest(const char* pszSeq, const char* pszFrom, const char* pszTo, const char* pszModule)
{
    return NewMessage(VALUE_REQUEST, pszSeq, pszFrom, pszTo, pszModule, 0);
}

iks* NewResponse(iks* pnRequest, const char* pszFrom, const char* pszTo)
{
    const char* pszSeq = iks_find_attrib(pnRequest, ATTRIBUTE_SEQ);
    const char* pszModule = iks_find_attrib(pnRequest, ATTRIBUTE_MODULE);
    const char* pszDomain = iks_find_attrib(pnRequest, ATTRIBUTE_DOMAIN);

    if (!pszTo)
    {
        pszTo = iks_find_attrib(pnRequest, ATTRIBUTE_FROM);
    }

    if (!pszFrom)
    {
        pszFrom = iks_find_attrib(pnRequest, ATTRIBUTE_TO);
    }

    return NewMessage(VALUE_RESPONSE, pszSeq, pszFrom, pszTo, pszModule, pszDomain);
}

iks* NewErrResponse(iks* pnRequest, const char* pszFrom, int nErrCode, const char* pszErrMsg)
{
    iks* pnBody = iks_first_tag(pnRequest);
    if (!pnBody) return 0;

    const char* pszTag = iks_name(pnBody);
    if (!pszTag || (strlen(pszTag) <= 0)) return 0;

    iks *pnResponse = NewResponse(pnRequest, pszFrom, 0);

    do 
    {
        if (!pnResponse) break;
        iks* pnBody = iks_insert(pnResponse, pszTag);
        if (!pnBody) break;
        ostringstream oStr;
        oStr << nErrCode;

        iks_insert_attrib(pnBody,ATTRIBUTE_ERRCODE,oStr.str().c_str());
        iks_insert_attrib(pnBody,ATTRIBUTE_ERRMSG,pszErrMsg);
    }while(0);

    return pnResponse;
}

iks* NewNotification(const char* pszFrom, const char* pszTo, const char* pszModule)
{
    return NewMessage(VALUE_NOTIFICATION, 0, pszFrom, pszTo, pszModule, 0);
}

iks* NewHeartBeatRequest(const char* pszSeq)
{
    iks* pnHeartBeat = iks_new(TAG_HEARTBEAT);
    do 
    {
        if (!pnHeartBeat) break;
        iks_insert_attrib(pnHeartBeat, ATTRIBUTE_TYPE, VALUE_REQUEST);
        iks_insert_attrib(pnHeartBeat, ATTRIBUTE_SEQ, pszSeq);
    }while(0);
    return pnHeartBeat;
}

iks* NewHeartBeatResponse(iks* pnHeartBeat)
{
    const char* pszSeq = iks_find_attrib(pnHeartBeat, ATTRIBUTE_SEQ);
    iks* pnResponse = iks_new(TAG_HEARTBEAT);
    do 
    {
        if (!pnResponse) break;
        iks_insert_attrib(pnResponse, ATTRIBUTE_TYPE, VALUE_RESPONSE);
        iks_insert_attrib(pnResponse, ATTRIBUTE_SEQ, pszSeq);
    }while(0);

    return pnResponse;
}

bool IsValidHeartBeatRequest(iks* pnHeartBeat)
{
    assert(pnHeartBeat);

    const char* pszName = iks_name(pnHeartBeat);
    if (!pszName || (strcmp(pszName, TAG_HEARTBEAT) != 0)) return false;

    const char* pszType = iks_find_attrib(pnHeartBeat, ATTRIBUTE_TYPE);
    const char* pszSeq = iks_find_attrib(pnHeartBeat, ATTRIBUTE_SEQ);

    if (!pszType || !pszSeq) return false;

    if (strcmp(pszType, VALUE_REQUEST) != 0) return false;
    if (strlen(pszSeq) <= 0) return false;

    return true;
}

bool IsValidHeartBeatResponse(iks* pnHeartBeat)
{
    assert(pnHeartBeat);

    const char* pszName = iks_name(pnHeartBeat);

    if (!pszName || (strcmp(pszName, TAG_HEARTBEAT) != 0)) return false;

    const char* pszType = iks_find_attrib(pnHeartBeat, ATTRIBUTE_TYPE);
    const char* pszSeq = iks_find_attrib(pnHeartBeat, ATTRIBUTE_SEQ);
    if (!pszType || !pszSeq) return false;
    if (strcmp(pszType, VALUE_RESPONSE) != 0) return false;
    if (strlen(pszSeq) <= 0) return false;

    return true;
}

} // namespace psmscommon
