#include "pmessagepopup.h"
#include "ui_pmessagepopup.h"
#include "widgetmanager.h"
#include <QFontMetrics>

void PMessagePopup::critical(const QString &text, QPoint contextPt, QWidget *parent /*= 0*/)
{
	PMessagePopup *popup = new PMessagePopup(PMessagePopup::Failed, text, parent);
	popup->move(QPoint(contextPt.x()-26, contextPt.y()-33));
	WidgetManager::showActivateRaiseWindow(popup);
}

void PMessagePopup::information(const QString &text, QPoint contextPt, QWidget *parent /*= 0*/)
{
	PMessagePopup *popup = new PMessagePopup(PMessagePopup::Information, text, parent);
	popup->move(QPoint(contextPt.x()-26, contextPt.y()-33));
	WidgetManager::showActivateRaiseWindow(popup);
}

void PMessagePopup::question(const QString &text, QPoint contextPt, QWidget *parent /*= 0*/)
{
	PMessagePopup *popup = new PMessagePopup(PMessagePopup::Question, text, parent);
	popup->move(QPoint(contextPt.x()-26, contextPt.y()-33));
	WidgetManager::showActivateRaiseWindow(popup);
}

void PMessagePopup::warning(const QString &text, QPoint contextPt, QWidget *parent /*= 0*/)
{
	PMessagePopup *popup = new PMessagePopup(PMessagePopup::Warning, text, parent);
	popup->move(QPoint(contextPt.x()-26, contextPt.y()-33));
	WidgetManager::showActivateRaiseWindow(popup);
}

void PMessagePopup::success(const QString &text, QPoint contextPt, QWidget *parent /*= 0*/)
{
	PMessagePopup *popup = new PMessagePopup(PMessagePopup::Success, text, parent);
	popup->move(QPoint(contextPt.x()-26, contextPt.y()-33));
	WidgetManager::showActivateRaiseWindow(popup);
}

PMessagePopup::PMessagePopup(Type popupType, const QString &text, QWidget *parent /*= 0*/)
	: FramelessDialog(parent)
{
	ui = new Ui::PMessagePopup();
	ui->setupUi(this);

	setMouseTracking(true);
	setAttribute(Qt::WA_Hover, true);
	setWindowFlags(windowFlags() |Qt::Dialog | Qt::Popup);	

	switch (popupType)
	{
	case Success:
		ui->labelIcon->setPixmap(QPixmap(":/messagebox/success.png"));
		break;
	case Failed:
		ui->labelIcon->setPixmap(QPixmap(":/messagebox/failed.png"));
		break;
	case Question:
		ui->labelIcon->setPixmap(QPixmap(":/messagebox/question.png"));
		break;
	case Information:
		ui->labelIcon->setPixmap(QPixmap(":/messagebox/info.png"));
		break;
	case Warning:
		ui->labelIcon->setPixmap(QPixmap(":/messagebox/warning.png"));
		break;
	default:
		break;
	}

	ui->labelText->setStyleSheet("QLabel#labelText {font: 9pt; color: black;}");
	ui->labelText->setText(text);
	QFontMetrics fm = ui->labelText->fontMetrics();
	int textWidth = fm.width(text);
	setFixedSize(textWidth+35, 38);
	setResizeable(false);
	setMoveAble(false);

	m_hideTimer.setSingleShot(true);
	m_hideTimer.setInterval(8000);
	connect(&m_hideTimer, SIGNAL(timeout()), this, SLOT(onHideTimeout()));
	m_hideTimer.start();

	setSkin();

	setFocus();
}

PMessagePopup::~PMessagePopup()
{
	delete ui;
}

void PMessagePopup::setSkin()
{
	// set background image
	QPixmap bgPixmap(":/images/popup_bg.png");
	FramelessDialog::BGSizes bgSizes = {0};
	bgSizes.borderwidth = 2;
	bgSizes.bottomBarHeight = 7;
	setBG(bgPixmap, bgSizes);
}

void PMessagePopup::focusOutEvent(QFocusEvent *ev)
{
	FramelessDialog::focusOutEvent(ev);

	this->hide();
	this->deleteLater();
}

void PMessagePopup::onHideTimeout()
{
	this->hide();
	this->deleteLater();
}
