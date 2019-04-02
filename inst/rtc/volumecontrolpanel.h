#ifndef VOLUMECONTROLPANEL_H
#define VOLUMECONTROLPANEL_H

#include "framelessdialog.h"
#include <QTimer>
namespace Ui {class VolumeControlPanel;};

class VolumeControlPanel : public FramelessDialog
{
	Q_OBJECT

public:
	VolumeControlPanel(int minVolume, int maxVolume, QWidget *parent = 0);
	~VolumeControlPanel();

	void preHide();

	void setEnable(bool enable);
	void setVolume(int vol);

Q_SIGNALS:
	void volumeChanged(int vol);

public Q_SLOTS:
	void setSkin();

protected:
	void showEvent(QShowEvent *e);
	void enterEvent(QEvent *e);
	void leaveEvent(QEvent *e);

private Q_SLOTS:
	void onHideTimeout();
	void onEnterTimeout();

private:
	Ui::VolumeControlPanel *ui;
	QTimer                  m_hideTimer;
	QTimer                  m_enterTimer;
};

#endif // VOLUMECONTROLPANEL_H
