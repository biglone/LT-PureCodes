#ifndef _PLAYBEEP_H_
#define _PLAYBEEP_H_
#include <QString>

class PlayBeep
{
public:
	static void playSendBeep();

	static void playRecvBuddyMsgBeep();
	static void playRecvSubscriptionMsgBeep();

    static void playAudioBeep();

	static void play(const QString &file);
	static void playBuddy(const QString &file);
	static void playSubscription(const QString &file);

	static void setMute(bool mute = true);
	static void setBuddyMute(bool mute = true);
	static void setSubscriptionMute(bool mute = true);
	static void setBuddyFilePath(const QString &path);
	static void setSubscriptionFilePath(const QString &path);

private:
	PlayBeep() {}
	~PlayBeep() {}

	static QString getBuddyFilePath();
	static QString getSubscriptionFilePath();

	static bool    m_mute;
	static bool    m_buddyMute;
	static bool    m_subscriptionMute;
	static QString m_buddyFilePath;
	static QString m_subscriptionFilePath;
};
#endif //_PLAYBEEP_H_