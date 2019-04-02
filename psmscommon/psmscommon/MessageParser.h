#ifndef _PSMSCOMMON_MESSAGEPARSER_H_INCLUDED
#define _PSMSCOMMON_MESSAGEPARSER_H_INCLUDED

#include "iks/iksemel.h"

#define SAFE_DELETE_IKS(x) do { if (x) { iks_delete(x); x = 0; } } while (0);
#define SAFE_DELETE_IKSPARSER(x) do { if (x) { iks_parser_delete(x); x = 0; } } while (0);

namespace psmscommon
{

/**
* @class CMessageParser
* 消息解析器.
*/
class CMessageParser
{
public:
    CMessageParser();
    virtual ~CMessageParser();

public:
    /// 增加需要解析的数据
    bool Append(const char* pBuffer, const int nLen);

    /// 当前解析的消息TAG长度
    int GetCurrentTagLen();

public:
    virtual void OnNode(iks* pnNode) = 0;

private:
    static int TagHook(void* pArg, char* pszName, char** pszAtts, int nType);
    static int CDataHook(void* pArg, char* pszCData, size_t nLen);

    int OnTagHook(char* pszName, char** pszAtts, int nType);
    int OnCDataHook(char* pszCData, size_t nLen);

private:
    iksparser* m_pParser;           /// XML流解析器
    iks* m_pnCurrentNode;           /// 当前最内层的NODE
};

} // namespace psmscommon

#endif //_PSMSCOMMON_MESSAGEPARSER_H_INCLUDED
