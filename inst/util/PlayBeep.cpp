#include <QFile>
#include <QSound>
#include "settings/GlobalSettings.h"
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QCoreApplication>
#include <QAudioDeviceInfo>

#include "PlayBeep.h"

bool PlayBeep::m_mute = false;
bool PlayBeep::m_buddyMute = false;
bool PlayBeep::m_subscriptionMute = false;
QString PlayBeep::m_buddyFilePath;
QString PlayBeep::m_subscriptionFilePath;

void PlayBeep::play(const QString& file)
{
	if (m_mute)
		return;

	if (QAudioDeviceInfo::availableDevices(QAudio::AudioOutput).isEmpty())
	{
		qWarning() << Q_FUNC_INFO << "QSound is not available";
		return;
	}

	if (file.isEmpty() || !QFile::exists(file))
	{
		qWarning() << Q_FUNC_INFO << "QSound play file is not exists: " << file;
		return;
	}
	
	QSound::play(file);
}

void PlayBeep::playBuddy(const QString &file)
{
	if (m_buddyMute)
		return;

	play(file);
}

void PlayBeep::playSubscription(const QString &file)
{
	if (m_subscriptionMute)
		return;

	play(file);
}

void PlayBeep::playRecvBuddyMsgBeep()
{
	playBuddy(getBuddyFilePath());
}

void PlayBeep::playRecvSubscriptionMsgBeep()
{
	playSubscription(getSubscriptionFilePath());
}

void PlayBeep::playSendBeep()
{
	QString file = GlobalSettings::getBeepSend();
	QDir appDir(QCoreApplication::applicationDirPath());
	QFileInfo fi(appDir, file);
	file = fi.absoluteFilePath();
	play(file);
}

void PlayBeep::playAudioBeep()
{
    QString file = GlobalSettings::getBeepAudio();
	QDir appDir(QCoreApplication::applicationDirPath());
	QFileInfo fi(appDir, file);
	file = fi.absoluteFilePath();
    play(file);
}

void PlayBeep::setMute(bool mute /* = true */)
{
	m_mute = mute;
}

 void PlayBeep::setBuddyMute(bool mute/* = true*/)
{
	m_buddyMute = mute;
}

void PlayBeep::setSubscriptionMute(bool mute/* = true*/)
{
	m_subscriptionMute = mute;
}

void PlayBeep::setBuddyFilePath(const QString &path)
{
	m_buddyFilePath = path;
}

void PlayBeep::setSubscriptionFilePath(const QString &path)
{
	m_subscriptionFilePath = path;
}

QString PlayBeep::getBuddyFilePath()
{
	if (m_buddyFilePath.isEmpty())
	{
		QString file = GlobalSettings::getBeepRecv();
		QDir appDir(QCoreApplication::applicationDirPath());
		QFileInfo fi(appDir, file);
		m_buddyFilePath = fi.absoluteFilePath();
	}
	return m_buddyFilePath;
}

QString PlayBeep::getSubscriptionFilePath()
{
	if (m_subscriptionFilePath.isEmpty())
	{
		QString file = GlobalSettings::getBeepRecv();
		QDir appDir(QCoreApplication::applicationDirPath());
		QFileInfo fi(appDir, file);
		m_subscriptionFilePath = fi.absoluteFilePath();
	}
	return m_subscriptionFilePath;
}
