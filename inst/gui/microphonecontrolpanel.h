#ifndef MICROPHONECONTROLPANEL_H
#define MICROPHONECONTROLPANEL_H

#include "framelessdialog.h"
#include <QTimer>
namespace Ui {class MicrophoneControlPanel;};

class MicrophoneControlPanel : public FramelessDialog
{
	Q_OBJECT

public:
	MicrophoneControlPanel(QWidget *parent = 0);
	~MicrophoneControlPanel();

	void preHide();

public slots:
	void setSkin();

protected:
	void showEvent(QShowEvent *e);
	void enterEvent(QEvent *e);
	void leaveEvent(QEvent *e);

private slots:
	void onHideTimeout();
	void onBoostToggled(bool checked);
	void onVolumeChanged(int volume);

private:
	void setVolumeAndBoostInfo();

private:
	Ui::MicrophoneControlPanel *ui;
	QTimer m_hideTimer;
};

#endif // MICROPHONECONTROLPANEL_H
