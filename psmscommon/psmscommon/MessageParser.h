#ifndef _PSMSCOMMON_MESSAGEPARSER_H_INCLUDED
#define _PSMSCOMMON_MESSAGEPARSER_H_INCLUDED

#include "iks/iksemel.h"

#define SAFE_DELETE_IKS(x) do { if (x) { iks_delete(x); x = 0; } } while (0);
#define SAFE_DELETE_IKSPARSER(x) do { if (x) { iks_parser_delete(x); x = 0; } } while (0);

namespace psmscommon
{

/**
* @class CMessageParser
* ��Ϣ������.
*/
class CMessageParser
{
public:
    CMessageParser();
    virtual ~CMessageParser();

public:
    /// ������Ҫ����������
    bool Append(const char* pBuffer, const int nLen);

    /// ��ǰ��������ϢTAG����
    int GetCurrentTagLen();

public:
    virtual void OnNode(iks* pnNode) = 0;

private:
    static int TagHook(void* pArg, char* pszName, char** pszAtts, int nType);
    static int CDataHook(void* pArg, char* pszCData, size_t nLen);

    int OnTagHook(char* pszName, char** pszAtts, int nType);
    int OnCDataHook(char* pszCData, size_t nLen);

private:
    iksparser* m_pParser;           /// XML��������
    iks* m_pnCurrentNode;           /// ��ǰ���ڲ��NODE
};

} // namespace psmscommon

#endif //_PSMSCOMMON_MESSAGEPARSER_H_INCLUDED
