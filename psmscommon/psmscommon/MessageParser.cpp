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

        // ������һ��Ϊ'\0',�����ȥ��,��Ȼparseʧ��
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
        ����û����tagͷ��tagβ�Ƿ�ƥ���У��
        <message></message>��<message></message1>���������Ľ����һ����
        ����,ͨ��iks_string�õ��Ľ����Ϊ"<message/>
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
        
        // �յ���һ����������Ϣ
        if (x == NULL)
        {
            // ������Ϣ
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
