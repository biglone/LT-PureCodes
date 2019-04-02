#ifndef FTJOB_H
#define FTJOB_H

#include "filetransfer_global.h"
#include <QString>

class QFile;

/*
�ļ���������FTJob
�����麯������FTHandler����
*/
class FILETRANSFER_EXPORT FTJob
{
public:
	enum OverCode
	{
		OC_OK,
		OC_FAIL,
		OC_CANCEL,
	};

	enum Type
	{
		UPLOAD,
		DOWNLOAD,
	};

public:
	FTJob():m_bCancel(false){};
	virtual ~FTJob(){};
public:
	// ʲô���ͣ��ϴ���������
	virtual Type type() = 0;

	// �Ƿ�ϵ�����
	virtual bool isContinuous() = 0;
	
	// ʲô�ļ���������ȷ�ķ�ʽ���ļ��������upload�����rb�������download����дwb��
	// �򿪵��ļ���������������close
	virtual QFile* file() = 0;

	// ʲô��������,�������Ψһ��������
	virtual QString transferName() = 0;

	// uuid
	virtual QString uuid() = 0;

	// ʲô�û���
	virtual QString userId() = 0;

	// ÿ�������kb֪ͨһ��
	virtual int transferPerKb() = 0;

	// �������false���������ٴ��䣬��Ҫ���°�job����thread��thread������������ش�
	virtual bool transferKb(int nKb, int nKbTotal) = 0;
	
	// ����������м������ok��fail��cancel
	virtual void transferOver(OverCode oc, const QString &param = QString()) = 0;

	// ����֮ǰ��Ԥ��������false��ʾԤ����ʧ�ܣ������д���
	virtual OverCode preTransfer() = 0;

	// �������֮��ĺ�������false��ʾ����ʧ�ܣ���ʹ����ɹ�Ҳ��ʾ�˴�ʧ��
	virtual OverCode postTransfer() = 0;

public:
	void cancel(){m_bCancel = true;}
	bool isCancel(){return m_bCancel;}

private:
	bool m_bCancel;
};

#endif // FTJOB_H