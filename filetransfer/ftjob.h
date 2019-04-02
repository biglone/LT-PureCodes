#ifndef FTJOB_H
#define FTJOB_H

#include "filetransfer_global.h"
#include <QString>

class QFile;

/*
文件传输任务FTJob
所有虚函数都由FTHandler调用
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
	// 什么类型，上传还是下载
	virtual Type type() = 0;

	// 是否断点续传
	virtual bool isContinuous() = 0;
	
	// 什么文件，请以正确的方式打开文件，如果是upload，则读rb，如果是download，则写wb，
	// 打开的文件请在析构函数中close
	virtual QFile* file() = 0;

	// 什么传输名称,这个可以唯一区分任务
	virtual QString transferName() = 0;

	// uuid
	virtual QString uuid() = 0;

	// 什么用户名
	virtual QString userId() = 0;

	// 每传输多少kb通知一次
	virtual int transferPerKb() = 0;

	// 如果返回false，若还想再传输，需要重新把job交给thread，thread不会进行主动重传
	virtual bool transferKb(int nKb, int nKbTotal) = 0;
	
	// 传输结束，有几种情况ok，fail，cancel
	virtual void transferOver(OverCode oc, const QString &param = QString()) = 0;

	// 传输之前的预处理，返回false表示预处理失败，不进行传输
	virtual OverCode preTransfer() = 0;

	// 传输完成之后的后处理，返回false表示后处理失败，即使传输成功也表示此次失败
	virtual OverCode postTransfer() = 0;

public:
	void cancel(){m_bCancel = true;}
	bool isCancel(){return m_bCancel;}

private:
	bool m_bCancel;
};

#endif // FTJOB_H