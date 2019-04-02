#ifndef INTERPHONEDIALOG_H
#define INTERPHONEDIALOG_H

#include "FramelessDialog.h"
#include <QList>
#include <QMap>
#include <QPixmap>
#include <QTimer>
#include <QPointer>
#include "interphoneinfo.h"
#include <QScopedPointer>

namespace interphone {class InterphoneSession;};

namespace Ui {class InterphoneDialog;};

class MicrophoneControlPanel;
class InterphoneMemberItemWidget;

class InterphoneDialog : public FramelessDialog
{
	Q_OBJECT

public:
	enum SpeakState
	{
		SpeakStop,
		SpeakPrepareing,
		SpeakPrepared,
		Speaking,
		SpeakConflict
	};

public:
	InterphoneDialog(QWidget *parent = 0);
	~InterphoneDialog();

	static bool hasInterphoneDialog();
	static InterphoneDialog * getInterphoneDialog();

	void setInterphoneId(const QString &id);
	QString interphoneId() const;

	bool openChannel();
	void closeChannel();

	void quitAndClose();

public slots:
	virtual void setSkin();

protected:
	void paintEvent(QPaintEvent *ev);
	void closeEvent(QCloseEvent *ev);
	void keyPressEvent(QKeyEvent *ev);
	void keyReleaseEvent(QKeyEvent *ev);
	void changeEvent(QEvent *ev);

private slots:
	void onSoundWaveTimeout();
	void onSpeakButtonToogled(bool pressed);

	void onInterphoneChanged(const QString &interphoneId, int attchType, const QString &attachId);
	void onSyncInterphoneMemberFinished(bool OK, const QString &interphoneId);
	void onPrepareSpeakOK(const QString &interphoneId);
	void onPrepareSpeakFailed(const QString &interphoneId);
	void onStopSpeakOK(const QString &interphoneId);
	void onStopSpeakFailed(const QString &interphoneId);
	/*
	void onQuitInterphoneFinished(bool ok, const QString &interphoneId);
	*/
	void onInterphoneCleared();

	void onSpeakPrepareTimeout();

	void onRecvChannelFailed();

	void onTickTimeout();

	void onEnterBtnStopAudio();
	void onLeaveBtnStopAudio();

	void chat(const QString &uid);

	void on_btnOpenChatDialog_clicked();

	void onDetailChanged(const QString &id);

private:
	enum SoundType
	{
		Press,
		Release,
		Ready,
		OtherReady,
		Conflict
	};

	void startPlaySoundWave();
	void stopPlaySoundWave();
	void initSoundWave();
	void initUI();
	void setSpeaker(const QString &id);
	void setTip(const QString &tip, bool warning = false);
	void setMemberCount(int count);
	void setMembers(const QStringList &memberIds);
	QString configTipText(const QString &speaker, int memberCount);
	void prepareSpeak();
	void stopSpeak();
	void setSpeakState(SpeakState state);
	void onSpeakStateChanged(SpeakState oldState, SpeakState newState);
	void playSound(SoundType soundType);
	void startTalk();
	void stopTalk();
	void startTick();
	void stopTick();
	void setTickText();
	void quitInterphone();
	MicrophoneControlPanel *microphoneControlPanel();
	void showMemberInOutTip(const QString &tip);
	void configMemberInOut(const QStringList &origMembers, const QStringList &newMembers);

private:
	Ui::InterphoneDialog *ui;

	QString        m_interphoneId;
	InterphoneInfo m_interphone;
	QTimer         m_speakPrepareTimer;

	QMap<QString, InterphoneMemberItemWidget *> m_memberWidgets;

	bool           m_isPlayingSoundWave;
	int            m_soundWaveIndex;
	QList<QPixmap> m_soundWavePixmaps;
	QTimer         m_soundWaveTimer;

	SpeakState     m_speakState;

	bool           m_closing;

	QScopedPointer<interphone::InterphoneSession> m_session;

	QTimer      m_tickTimer;
	int         m_tickSeconds;

	QTimer      m_memberInOutTipTimer;

	QScopedPointer<MicrophoneControlPanel> m_pMicrophoneControlPanel;

	static QPointer<InterphoneDialog> s_interphoneDialog;
};

class SpeakerAvatar : public QWidget
{
public:
	SpeakerAvatar(QWidget *parent = 0);
	virtual ~SpeakerAvatar();

	void setAvatar(const QPixmap &avatar);

protected:
	void paintEvent(QPaintEvent *ev);

private:
	QPixmap m_avatar;
	QPixmap m_border;
};

class SpeakButton : public QWidget
{
	Q_OBJECT

public:
	enum SignalState
	{
		Normal,
		Prepare,
		Ready,
		Busy
	};

public:
	SpeakButton(QWidget *parent = 0);
	virtual ~SpeakButton();

	void setPressed(bool pressed);
	bool isPressed() const;

	void setEnabled(bool enabled);

	SignalState signalState() const;
	void setSignalState(SignalState state);

Q_SIGNALS:
	void toggled(bool pressed);

protected:
	void paintEvent(QPaintEvent *ev);
	void mousePressEvent(QMouseEvent *ev);
	void mouseReleaseEvent(QMouseEvent *ev);
	void mouseMoveEvent(QMouseEvent *ev);

private:
	QPixmap m_normalPixmap;
	QPixmap m_pressedPixmap;
	QPixmap m_disabledPixmap;

	QPixmap m_prepareSignalPixmap;
	QPixmap m_readySignalPixmap;
	QPixmap m_busySignalPixmap;

	bool        m_pressed;
	bool        m_enabled;

	SignalState m_signalState;
};

#endif // INTERPHONEDIALOG_H
