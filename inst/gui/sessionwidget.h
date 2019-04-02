#ifndef SESSIONWIDGET_H
#define SESSIONWIDGET_H

#include <QWidget>
#include <QPointer>
#include <QTimer>
#include <QScopedPointer>

namespace Ui {class SessionWidget;};

namespace rtcsession {class Session;};
class CChatDialog;
class VideoWidget;
// class VolumeControlPanel;

class SessionWidget : public QWidget
{
    Q_OBJECT
public:
    enum SessionState
    {
        SS_Invite = 0,
        SS_OnInvite,
        SS_Audio
    };

public:
    explicit SessionWidget(QWidget *parent = 0);
    ~SessionWidget();

public:
	void setChatDialog(CChatDialog *pChatDialog);

    bool inviteAudio(const QString &uid);
    bool inviteVideo(const QString &uid);
    bool onInvite(const QString &sid);

    QString title() const;

    int state() const;

    void sessionClose();

public:
    void setSkin();

Q_SIGNALS:
	void sessionVideoSetup();
    void sessionClosed();
	
protected:
    void timerEvent(QTimerEvent *event);

private slots:
    void onSessionDestoryed(QObject *obj);
	void onSessionError(int errCode);
	void onSessionStateChanged(int ctrlPeer, int curState, int oldState);
    void onSessionOnOk();

private slots:
    void on_btnBye_clicked();
    void on_btnOk_clicked();
    void on_btnReject_clicked();
    void on_btnAudioBye_clicked();
	void onBtnMicClicked(bool checked);
	void onBtnVolumeClicked(bool checked);
	// void onEnterVolumeBtn();
	// void onLeaveVolumeBtn();
	void onAudioSendChanged(bool send);
	void onVolumeChanged(int vol);

private:
    void startElapsedTimer();
    void stopElapsedTimer();
    void initUI();
	QString otherId() const;
	// VolumeControlPanel *volumeControlPanel();

private:
    QPointer<rtcsession::Session> m_pSession;

	// QScopedPointer<VolumeControlPanel> m_volumeControlPanel;
	int                m_currentVolume;

	CChatDialog       *m_pChatDlg;

    Ui::SessionWidget *ui;
    int                timerId;
    int                elapsed;
};

#endif // SESSIONWIDGET_H
