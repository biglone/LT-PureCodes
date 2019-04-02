#ifndef STATUSCHANGER_H
#define STATUSCHANGER_H

#include <QObject>
#include "loginmgr.h"

class StatusChanger : public QObject
{
	Q_OBJECT
	Q_ENUMS(Status)

public:
	enum Status
	{
		Status_Online  = 0x00000000,
		Status_Away    = 0x00000001,
		Status_Chat    = 0x00000002,
		Status_Dnd     = 0x00000003,
		Status_Xa      = 0x00000004,
		Status_Offline = 0x00000005
	};

public:
	explicit StatusChanger(QObject *parent = 0);
	~StatusChanger();

	void reset();

	static QString status2Str(StatusChanger::Status status);
	static StatusChanger::Status str2Status(const QString &str);

public:
	Status curStatus() const;
	void setStatus(int status);
	void setStatus(Status eStatus);

public slots:
	void onPresenceReceived(const QString &id);
	void onLoginMgrLogined();

Q_SIGNALS:
	void statusChanged(int status);

private slots:
	void onLoginMgrLogouted();
	void onLoginMgrLoginError(const QString& err);

private:
	void startLogin();
	void sendPresence(Status eStatus);

private:
	/*
	m_bStartChanged 用来处理在用户离线的状态下，然后选择在线，这会引起一个登录过程。
	这过程中如果改变状态会引起不恰当的presence发送。
	*/
	bool                m_bStartChanged;

	Status              m_eCurStatus;
	Status              m_ePreStatus;
};

#endif // STATUSCHANGER_H
