#ifndef _PSMSCOMMON_UTILITY_H_INCLUDED
#define _PSMSCOMMON_UTILITY_H_INCLUDED

#include <string>
#include "iks/iksemel.h"
#include "PSMSConstants.h"

using namespace std;

namespace psmscommon
{

/// ���麯���ڲ�����iks*,��Ҫ�ⲿiks_delete
/// �����ڲ������������,��Ҫ�ⲿ��֤
/// ����0��ʾ��iks����ȡ,����""��ʾ���ô˲���
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

/// ��ʽУ��
bool IsValidHeartBeatRequest(iks* pnHeartBeat);
bool IsValidHeartBeatResponse(iks* pnHeartBeat);

} // namespace psmscommon

#endif //_PSMSCOMMON_UTILITY_H_INCLUDED
