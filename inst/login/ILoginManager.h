#ifndef _ILOGINMANAGER_H_
#define _ILOGINMANAGER_H_

class QObject;
class ILoginProcess
{
public:
	virtual QObject* instance() = 0;
	virtual QString name() const = 0;
	virtual bool start() = 0;
};

Q_DECLARE_INTERFACE(ILoginProcess, "Inst.Login.ILoginProcess/1.0");
#endif //_ILOGINMANAGER_H_
