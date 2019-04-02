#include "VolumeControlPanel.h"
#include "ui_VolumeControlPanel.h"
#include "microphonecontrol.h"
#include <QAudioDeviceInfo>
#include <QDebug>

VolumeControlPanel::VolumeControlPanel(int minVolume, int maxVolume, QWidget *parent)
	: FramelessDialog(parent)
{
	ui = new Ui::VolumeControlPanel();
	ui->setupUi(this);

	setMouseTracking(true);
	setAttribute(Qt::WA_Hover, true);
	setWindowFlags(windowFlags() | Qt::Dialog | Qt::WindowStaysOnTopHint);	
	setFixedSize(36, 116);
	setResizeable(false);
	setMoveAble(false);

	ui->verticalSliderVolume->setMinimum(minVolume);
	ui->verticalSliderVolume->setMaximum(maxVolume);
	ui->verticalSliderVolume->setSingleStep(1);
	ui->verticalSliderVolume->setValue(maxVolume);

	m_hideTimer.setInterval(800);
	m_hideTimer.setSingleShot(true);

	m_enterTimer.setInterval(3000);
	m_enterTimer.setSingleShot(true);

	connect(&m_hideTimer, SIGNAL(timeout()), this, SLOT(onHideTimeout()));
	connect(&m_enterTimer, SIGNAL(timeout()), this, SLOT(onEnterTimeout()));
	connect(ui->verticalSliderVolume, SIGNAL(valueChanged(int)), this, SIGNAL(volumeChanged(int)));

	setSkin();
}

VolumeControlPanel::~VolumeControlPanel()
{
	delete ui;
}

void VolumeControlPanel::preHide()
{
	if (m_hideTimer.isActive())
		m_hideTimer.stop();
	m_hideTimer.start();
}

void VolumeControlPanel::setEnable(bool enable)
{
	ui->verticalSliderVolume->setEnabled(enable);
}

void VolumeControlPanel::setVolume(int vol)
{
	ui->verticalSliderVolume->setValue(vol);
}

void VolumeControlPanel::setSkin()
{
	// set background image
	QPixmap bgPixmap(":/images/volume_control_bg.png");
	FramelessDialog::BGSizes bgSizes = {0};
	bgSizes.borderwidth = 5;
	setBG(bgPixmap, bgSizes);

	setStyleSheet("font-size: 9pt;");

	ui->verticalSliderVolume->setStyleSheet(
		"QSlider::groove:vertical {"
		"	background: rgb(0, 120, 216);"
		"	width: 4px;"
		"}"
		"QSlider::handle:vertical {"
		"	height: 10px;"
		"	border: 1px solid rgb(223, 223, 223);"
		"	background: white;"
		"	margin: 0 -5px;"
		"	border-radius: 6px;"
		"}"
		"QSlider::add-page:vertical {"
		"	background: rgb(0, 120, 216);"
		"}"
		"QSlider::sub-page:vertical {"
		"	background: rgb(196, 196, 196);"
		"}"
		"QSlider::add-page:vertical:disabled {"
		"	background: rgb(196, 196, 196);"
		"}"
		);
}

void VolumeControlPanel::showEvent(QShowEvent *e)
{
	m_enterTimer.start();
	FramelessDialog::showEvent(e);
}

void VolumeControlPanel::enterEvent(QEvent *e)
{
	m_enterTimer.stop();
	m_hideTimer.stop();

	FramelessDialog::enterEvent(e);
}

void VolumeControlPanel::leaveEvent(QEvent *e)
{
	FramelessDialog::leaveEvent(e);

	preHide();
}

void VolumeControlPanel::onHideTimeout()
{
	m_hideTimer.stop();
	hide();
}

void VolumeControlPanel::onEnterTimeout()
{
	m_enterTimer.stop();
	hide();
}

