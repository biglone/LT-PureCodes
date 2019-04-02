#ifndef SESSIONVIDEODIALOG_H
#define SESSIONVIDEODIALOG_H

#include "framelessdialog.h"
#include <QPointer>
#include <QScopedPointer>
#include <QTimer>

namespace Ui {class SessionVideoDialog;};
class SimpleVideoWidget;
class VolumeControlPanel;
namespace rtcsession {class Session;}

class SessionVideoDialog : public FramelessDialog
{
	Q_OBJECT

public:
	SessionVideoDialog(const QString &id, rtcsession::Session *pSession, QWidget *parent = 0);
	~SessionVideoDialog();

	void closeSession();

public slots:
	void setSkin();

protected:
	void timerEvent(QTimerEvent *event);
	void keyPressEvent(QKeyEvent *event);
	void closeEvent(QCloseEvent *event);
	void enterEvent(QEvent *e);
	void leaveEvent(QEvent *e);

private slots:
	void onBtnVideoByeClicked();
	void onBtnStopAudioClicked(bool checked);
	void onBtnVolumeClicked(bool checked);
	void onBtnStopVideoClicked(bool checked);
	void onBtnFullScreanToggled(bool checked);
	void onFullScreenEscape();
	void onBigVideoDoubleClicked();
	void onBtnChatClicked();
	void onBtnPhotoClicked();
	void onBtnShowModeClicked(bool checked);

	void onAudioSendChanged(bool send);
	void onVideoSendChanged(bool send);

	void onSessionDestoryed(QObject *obj);
	void onSessionStateChanged(int ctrlPeer, int curState, int oldState);
	void onSessionError(int errCode);

	void onMaximizeStateChanged(bool maximized);
	/*
	void onEnterBtnVolume();
	void onLeaveBtnVolume();
	*/
	void onVolumeChanged(int vol);

	void onOtherAudioChanged(bool before, bool now);
	void onOtherVideoChanged(bool before, bool now);

	void onVideoHasEvent();

	void onUserDetailChanged(const QString &uid);

	void onPackageStatistics(int packageLost, int packageReceived);

private:
	void initUI();

	void startElapsedTimer();
	void stopElapsedTimer();

	void initVideoCanvas();

	// VolumeControlPanel *volumeControlPanel();

	void showTip(const QString &tip);

	void startSmallVideo();
	void stopSmallVideo();

private:
	Ui::SessionVideoDialog                *ui;

	QString                                m_id;
	QPointer<rtcsession::Session>          m_pSession;

	int                                    m_elapsedSeconds;
	int                                    m_elapsedTimerId;

	int                                    m_currentVolume;
	// QScopedPointer<VolumeControlPanel>     m_volumeControlPanel;

	QTimer                                 m_bottomHideTimer;

	QString                                m_otherUid;
};

#endif // SESSIONVIDEODIALOG_H
