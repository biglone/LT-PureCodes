#ifndef INTERPHONEMANAGER_H
#define INTERPHONEMANAGER_H

#include <QObject>
#include <QList>
#include <QMap>
#include <QScopedPointer>
#include "interphoneinfo.h"

class InterphoneResHandler;
class InterphoneNtfHandler;

class InterphoneManager : public QObject
{
	Q_OBJECT

public:
	InterphoneManager(QObject *parent = 0);
	~InterphoneManager();

	bool initObject();
	void removeObject();

public:
	void syncInterphones(const QStringList &groupIds, const QStringList &discussIds);
	void syncInterphoneMember(const QString &interphoneId);
	void addInterphone(const QString &interphoneId, const QString &uid);
	void quitInterphone(const QString &interphoneId, const QString &uid);
	void prepareSpeak(const QString &interphoneId);
	void stopSpeak(const QString &interphoneId);
	void clearInterphones();
	void quitCurrentInterphone(const QString &uid);
	void removeInterphone(const QString &interphoneId);

	bool isCurrentInInterphone() const;
	QString currentInterphone() const;
	bool hasInterphone(bean::MessageType attachType, const QString &attachId);
	bool isInInterphone(bean::MessageType attachType, const QString &attachId);
	QMap<QString, InterphoneInfo> allInterphones() const;
	InterphoneInfo interphone(const QString &id) const;
	QString interphoneAudioAddr() const;

	static QString attachTypeId2InterphoneId(bean::MessageType attachType, const QString &attachId);
	// attachId: if this is chat type, always return other's id
	static void interphoneId2AttachTypeId(const QString &interphoneId, bean::MessageType &attachType, QString &attachId);
	static bean::MessageType string2AttachType(const QString &typeStr);

Q_SIGNALS:
	void syncInterphonesFinished(bool OK);
	void syncInterphoneMemberFinished(bool OK, const QString &interphoneId);
	void addInterphoneFinished(bool OK, const QString &interphoneId);
	void quitInterphoneFinished(bool OK, const QString &interphoneId);
	void prepareSpeakOK(const QString &interphoneId);
	void prepareSpeakFailed(const QString &interphoneId);
	void stopSpeakOK(const QString &interphoneId);
	void stopSpeakFailed(const QString &interphoneId);
	
	void interphoneChanged(const QString &interphoneId, int attachType, const QString &attachId);
	void interphoneStarted(const QString &interphoneId, int attachType, const QString &attachId);
	void interphoneFinished(const QString &interphoneId);

	void interphonesCleared();

private slots:
	void setError(int type, const QString &interphoneId, const QString &param, const QString &errMsg);
	void processInterphones(const QList<InterphoneInfo> &interphones);
	void processInterphoneMember(const InterphoneInfo &interphone);
	void processInterphoneAdd(const InterphoneInfo &interphone, const QString &audioAddr);
	void processInterphoneQuit(const InterphoneInfo &interphone);
	void processSpeak(const QString &interphoneId, const QString &speakState);
	void processInterphoneChanged(const InterphoneInfo &interphone);

private:
	QScopedPointer<InterphoneResHandler> m_resHandler;
	QScopedPointer<InterphoneNtfHandler> m_ntfHandler;

	QMap<QString, InterphoneInfo>        m_interphones;
	QString                              m_currentInterphone;

	QString                              m_audioAddr;
};

#endif // INTERPHONEMANAGER_H
