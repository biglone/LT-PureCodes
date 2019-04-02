#ifndef _PSMSCOMMON_UTILITY_H_INCLUDED
#define _PSMSCOMMON_UTILITY_H_INCLUDED

#include <string>
#include "iks/iksemel.h"
#include "PSMSConstants.h"

using namespace std;

namespace psmscommon
{

/// 这组函数内部产生iks*,需要外部iks_delete
/// 函数内部不做参数检查,需要外部保证
/// 参数0表示从iks中提取,参数""表示不用此参数
iks* NewMessage(const char* pszType,
                const char* pszSeq, 
                const char* pszFrom, 
                const char* pszTo,
                const char* pszModule,
                const char* pszDomain);

iks* NewRequest(const char* pszSeq, const char* pszFrom, const char* pszTo, const char* pszModule);
iks* NewResponse(iks* pnRequest, const char* pszFrom, const char* pszTo);
iks* NewErrResponse(iks* pnRequest, const char* pszFrom, int nErrCode, const char* pszErrMsg);
iks* NewNotification(const char* pszFrom, const char* pszTo, const char* pszModule);
iks* NewHeartBeatRequest(const char* pszSeq);
iks* NewHeartBeatResponse(iks* pnHeartBeat);

/// 格式校验
bool IsValidHeartBeatRequest(iks* pnHeartBeat);
bool IsValidHeartBeatResponse(iks* pnHeartBeat);

} // namespace psmscommon

#endif //_PSMSCOMMON_UTILITY_H_INCLUDED
