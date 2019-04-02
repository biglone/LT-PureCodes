#include <assert.h>
#include "MessageParser.h"

namespace psmscommon
{

CMessageParser::CMessageParser()
: m_pParser(0)
, m_pnCurrentNode(0)
{

}

CMessageParser::~CMessageParser()
{
    SAFE_DELETE_IKSPARSER(m_pParser);
    SAFE_DELETE_IKS(m_pnCurrentNode);
}

bool CMessageParser::Append(const char* pBuffer, const int nLen)
{
    bool bRet = false;

    do 
    {
        if (!pBuffer || nLen <= 0) break;

        if (!m_pParser)
        {
            m_pParser = iks_sax_new(this, TagHook, CDataHook);
            if (!m_pParser) break;
        }

        // 如果最后一个为'\0',则把它去掉,不然parse失败
        if (*(pBuffer + nLen - 1) == 0)
        {
            int* pnLen = (int*)&nLen;
            *pnLen = 0;
        }

        int nRet = iks_parse(m_pParser, pBuffer, nLen, 0);
        if (nRet != IKS_OK) break;

        bRet = true;
    } while (0);

    return bRet;
}

int CMessageParser::GetCurrentTagLen()
{
    return m_pParser ? (int)iks_stack_pos(m_pParser) : 0;
}

int CMessageParser::TagHook(void* pArg, char* pszName, char** pszAtts, int nType)
{
    if (!pArg) return IKS_HOOK;

    CMessageParser* pThis = (CMessageParser*)pArg;
    return pThis->OnTagHook(pszName,pszAtts,nType);
}

int CMessageParser::CDataHook(void* pArg, char* pszCData, size_t nLen)
{
    if (!pArg) return IKS_HOOK;
    CMessageParser* pThis = (CMessageParser*)pArg;

    return pThis->OnCDataHook(pszCData,nLen);
}

int CMessageParser::OnTagHook(char* pszName, char** pszAtts, int nType)
{
    /*
    WARN!!!
        这里没有做tag头和tag尾是否匹配的校验
        <message></message>和<message></message1>解析出来的结果是一样的
        而且,通过iks_string得到的结果都为"<message/>
    */
    
    iks* x = 0;
    int err = IKS_OK;

    switch (nType)
    {
    case IKS_OPEN:
    case IKS_SINGLE:
        if (m_pnCurrentNode)
        {
            x = iks_insert(m_pnCurrentNode, pszName);
            if( !x ) return IKS_NOMEM;
            insert_attribs(x, pszAtts);
        }
        else
        {
            x = iks_new(pszName);
            if( !x ) return IKS_NOMEM;
            insert_attribs(x, pszAtts);
        }

        m_pnCurrentNode = x;

        if (nType == IKS_OPEN )
        {
            break;
        }
    case IKS_CLOSE:
        x = iks_parent(m_pnCurrentNode);
        
        // 收到了一个完整的消息
        if (x == NULL)
        {
            // 处理消息
            OnNode(m_pnCurrentNode);

            iks_delete(m_pnCurrentNode);
            m_pnCurrentNode = 0;
            break;
        }

        m_pnCurrentNode = x;

        break;
    default:
        // WARN
        break;
    }

    return IKS_OK;
}

int CMessageParser::OnCDataHook(char* pszCData, size_t nLen)
{
    if (m_pnCurrentNode)
    {
        iks_insert_cdata(m_pnCurrentNode, pszCData, nLen);
    }

    return IKS_OK;
}

} // namespace psmscommon
