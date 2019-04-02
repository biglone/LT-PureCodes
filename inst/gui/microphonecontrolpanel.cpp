#include "microphonecontrolpanel.h"
#include "ui_microphonecontrolpanel.h"
#include "microphonecontrol.h"
#include <QAudioDeviceInfo>
#include <QDebug>

MicrophoneControlPanel::MicrophoneControlPanel(QWidget *parent)
	: FramelessDialog(parent)
{
	ui = new Ui::MicrophoneControlPanel();
	ui->setupUi(this);

	setMouseTracking(true);
	setAttribute(Qt::WA_Hover, true);
	setWindowFlags(windowFlags() |Qt::Dialog | Qt::Popup);	
	setFixedSize(64, 136);
	setResizeable(false);
	setMoveAble(false);

	m_hideTimer.setInterval(800);
	connect(&m_hideTimer, SIGNAL(timeout()), this, SLOT(onHideTimeout()));
	connect(ui->checkBoxBoost, SIGNAL(toggled(bool)), this, SLOT(onBoostToggled(bool)));
	connect(ui->verticalSliderVolume, SIGNAL(valueChanged(int)), this, SLOT(onVolumeChanged(int)));

	setSkin();
}

MicrophoneControlPanel::~MicrophoneControlPanel()
{
	delete ui;
}

void MicrophoneControlPanel::preHide()
{
	if (m_hideTimer.isActive())
		m_hideTimer.stop();
	m_hideTimer.start();
}

void MicrophoneControlPanel::setSkin()
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
	);
}

void MicrophoneControlPanel::showEvent(QShowEvent *e)
{
	setVolumeAndBoostInfo();

	FramelessDialog::showEvent(e);
}

void MicrophoneControlPanel::enterEvent(QEvent *e)
{
	m_hideTimer.stop();

	FramelessDialog::enterEvent(e);
}

void MicrophoneControlPanel::leaveEvent(QEvent *e)
{
	FramelessDialog::leaveEvent(e);

	preHide();
}

void MicrophoneControlPanel::onHideTimeout()
{
	m_hideTimer.stop();
	hide();
}

void MicrophoneControlPanel::onBoostToggled(bool checked)
{
	if (!setMicrophoneBoostInfo(QAudioDeviceInfo::defaultInputDevice().deviceName(), checked))
	{
		qWarning() << Q_FUNC_INFO << "set microphone boost failed.";
	}
}

void MicrophoneControlPanel::onVolumeChanged(int volume)
{
	if (!setMicrophoneVolumeInfo(QAudioDeviceInfo::defaultInputDevice().deviceName(), 
		ui->verticalSliderVolume->minimum(), ui->verticalSliderVolume->maximum(), volume))
	{
		qWarning() << Q_FUNC_INFO << "set microphone volume failed.";
	}
}

void MicrophoneControlPanel::setVolumeAndBoostInfo()
{
	QString microphoneName = QAudioDeviceInfo::defaultInputDevice().deviceName();
	if (microphoneName.isEmpty())
	{
		ui->verticalSliderVolume->setEnabled(false);
		ui->checkBoxBoost->setEnabled(false);
		return;
	}

	disconnect(ui->checkBoxBoost, SIGNAL(toggled(bool)), this, SLOT(onBoostToggled(bool)));
	disconnect(ui->verticalSliderVolume, SIGNAL(valueChanged(int)), this, SLOT(onVolumeChanged(int)));

	int minimum = 0;
	int maximum = 0;
	int step = 0;
	int volume = 0;
	bool boost = false;

	if (!isMicrophoneVolumeControlAvailable(microphoneName))
	{
		ui->verticalSliderVolume->setEnabled(false);
	}
	else
	{
		if (!getMicrophoneVolumeInfo(microphoneName, minimum, maximum, step, volume))
		{
			ui->verticalSliderVolume->setEnabled(false);
		}
		else
		{
			ui->verticalSliderVolume->setEnabled(true);
			ui->verticalSliderVolume->setMinimum(minimum);
			ui->verticalSliderVolume->setMaximum(maximum);
			if (step > 0)
			{
				int sliderStep = (maximum - minimum)/step;
				ui->verticalSliderVolume->setSingleStep(sliderStep);
				ui->verticalSliderVolume->setPageStep(sliderStep);
				ui->verticalSliderVolume->setValue(volume);
			}
		}
	}

	if (!isMicrophoneBoostControlAvailable(microphoneName))
	{
		ui->checkBoxBoost->setEnabled(false);
	}
	else
	{
		if (!getMicrophoneBoostInfo(microphoneName, boost))
		{
			ui->checkBoxBoost->setEnabled(false);
		}
		else
		{
			ui->checkBoxBoost->setEnabled(true);
			ui->checkBoxBoost->setChecked(boost);
		}
	}

	connect(ui->checkBoxBoost, SIGNAL(toggled(bool)), this, SLOT(onBoostToggled(bool)));
	connect(ui->verticalSliderVolume, SIGNAL(valueChanged(int)), this, SLOT(onVolumeChanged(int)));
}

