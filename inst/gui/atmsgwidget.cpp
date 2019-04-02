#include "atmsgwidget.h"
#include "ui_atmsgwidget.h"
#include "bean/MessageBody.h"
#include "PmApp.h"
#include "ModelManager.h"
#include <QPainter>

AtMsgWidget::AtMsgWidget(QWidget *parent)
	: QWidget(parent)
{
	ui = new Ui::AtMsgWidget();
	ui->setupUi(this);

	ui->labelText->setEnterUnderline(false);
	ui->labelText->setStyleSheet("color: #000000;");

	StyleToolButton::Info info;
	info.urlNormal = ":/images/close_normal.png";
	info.urlHover = ":/images/close_pressed.png";
	info.urlPressed = ":/images/close_pressed.png";
	info.tooltip = tr("Close");
	ui->toolButtonClose->setInfo(info);

	connect(ui->toolButtonClose, SIGNAL(clicked()), this, SLOT(hide()));
	connect(ui->labelText, SIGNAL(clicked()), this, SLOT(takeAtMsg()));
}

AtMsgWidget::~AtMsgWidget()
{
	delete ui;
}

void AtMsgWidget::setTopAtMsg(const bean::MessageBody &atMsg)
{
	AtMsgData msgData;
	msgData.fromId = atMsg.from();
	msgData.atUid = atMsg.ext().data("atid").toString();
	msgData.atText = atMsg.toMessageText();
	m_atMsgs.prepend(msgData);

	setAtMsg(msgData);
}

void AtMsgWidget::setBottomAtMsg(const bean::MessageBody &atMsg)
{
	AtMsgData msgData;
	msgData.fromId = atMsg.from();
	msgData.atUid = atMsg.ext().data("atid").toString();
	msgData.atText = atMsg.toMessageText();
	m_atMsgs.append(msgData);

	if (m_atMsgs.count() == 1)
	{
		setAtMsg(msgData);
	}
}

void AtMsgWidget::clearAtMsg()
{
	m_atMsgs.clear();
	hide();
}

void AtMsgWidget::removeAtMsg(const QString &atUid)
{
	if (m_atMsgs.isEmpty())
		return;
	
	int i = 0;
	for (i = 0; i < m_atMsgs.count(); ++i)
	{
		AtMsgData atMsgData =  m_atMsgs[i];
		if (atMsgData.atUid == atUid)
			break;
	}

	if (i < m_atMsgs.count())
	{
		m_atMsgs.removeAt(i);
		if (m_atMsgs.isEmpty())
		{
			hide();
			return;
		}

		setAtMsg(m_atMsgs[0]);
	}
}

void AtMsgWidget::paintEvent(QPaintEvent * /*e*/)
{
	QPainter painter(this);
	
	QRect rt = rect();
	painter.fillRect(rt, QColor(155, 224, 179));
}

void AtMsgWidget::takeAtMsg()
{
	if (m_atMsgs.isEmpty())
		return;

	AtMsgData atMsgData =  m_atMsgs.takeFirst();
	emit anchorAtMsg(atMsgData.atUid);

	if (m_atMsgs.isEmpty())
	{
		hide();
		return;
	}

	setAtMsg(m_atMsgs[0]);
}

void AtMsgWidget::setAtMsg(const AtMsgData &atMsg)
{
	QString fromId = atMsg.fromId;
	ModelManager *modelManager = qPmApp->getModelManager();
	QPixmap fromPixmap = modelManager->getUserAvatar(fromId);
	ui->labelIcon->setPixmap(fromPixmap);

	QString msgText = atMsg.atText;
	msgText.replace("\r\n", " ").replace("\r", " ").replace("\n", " ");
	ui->labelText->setText(msgText);
}
