#include "chatinfopanel.h"
#include "ui_chatinfopanel.h"
#include "PmApp.h"
#include "ModelManager.h"
#include "bean/DetailItem.h"
#include "util/MaskUtil.h"
#include "QRCodeUtil.h"

ChatInfoPanel::ChatInfoPanel(QWidget *parent)
	: QWidget(parent)
{
	ui = new Ui::ChatInfoPanel();
	ui->setupUi(this);
}

ChatInfoPanel::~ChatInfoPanel()
{
	delete ui;
}

void ChatInfoPanel::setSkin()
{
	ui->contentWidget->setStyleSheet("QWidget#contentWidget {border-left: 1px solid rgb(219, 219, 219;)}");

	ui->iconPhone->setPixmap(QPixmap(":/images/Icon_50_2.png"));
	ui->iconMail->setPixmap(QPixmap(":/images/Icon_133.png"));

	ui->labelName->setStyleSheet("color: black;");
	ui->labelDepart->setStyleSheet("color: black;");
	ui->labelPhone->setStyleSheet("color: rgb(128, 128, 128); font-size: 9pt;");
	ui->labelMail->setStyleSheet("color: rgb(128, 128, 128); font-size: 9pt;");

	ui->hSep->setStyleSheet("color: rgb(216, 220, 223);");
}

void ChatInfoPanel::updateInfo(const QString &uid)
{
	if (uid.isEmpty())
		return;

	ModelManager *modelManager = qPmApp->getModelManager();
	bean::DetailItem *detail = modelManager->detailItem(uid);
	if (!detail)
		return;

	const int kAvatarSize = 72;
	QPixmap avatar = modelManager->getUserAvatar(uid);
	avatar = avatar.scaled(QSize(kAvatarSize, kAvatarSize), Qt::KeepAspectRatio, Qt::SmoothTransformation);
	QPixmap rawMask(":/images/Icon_60_mask.png");
	WidgetBorder border;
	border.top = border.bottom = border.left = border.right = 4;
	QBitmap mask = MaskUtil::generateMask(rawMask, border, QSize(kAvatarSize, kAvatarSize));
	avatar.setMask(mask);
	ui->iconAvatar->setPixmap(avatar);

	QString name = modelManager->userName(uid);
	ui->labelName->setText(name);

	int sex = detail->sex();
	if (sex < 0 && sex > 1)
	{
		sex = 9;
	}
	if (sex == 0)
	{
		// female
		ui->labelGender->setPixmap(QPixmap(":/images/Icon_95.png"));
	}
	else if (sex == 1)
	{
		// male
		ui->labelGender->setPixmap(QPixmap(":/images/Icon_94.png"));
	}
	else
	{
		// secret
		ui->labelGender->setPixmap(QPixmap());
		ui->labelGender->setVisible(false);
	}

	QString depart = detail->depart();
	ui->labelDepart->setText(depart);
	if (depart.isEmpty())
	{
		ui->labelDepart->setVisible(false);
	}
	else
	{
		ui->labelDepart->setVisible(true);
	}
	
	QString phone = detail->phone1();
	ui->labelPhone->setText(phone);
	if (phone.isEmpty())
	{
		ui->iconPhone->setVisible(false);
		ui->labelPhone->setVisible(false);
	}
	else
	{
		ui->iconPhone->setVisible(true);
		ui->labelPhone->setVisible(true);
	}

	QString email = detail->email();
	ui->labelMail->setText(email);
	if (email.isEmpty())
	{
		ui->iconMail->setVisible(false);
		ui->labelMail->setVisible(false);
	}
	else
	{
		ui->iconMail->setVisible(true);
		ui->labelMail->setVisible(true);
	}

	if (!name.isEmpty() && (!phone.isEmpty() || !email.isEmpty()))
	{
		QString vCard = QString("BEGIN:VCARD\n"
			"FN:%1\n"
			"TEL:%2\n"
			"EMAIL:%3\n"
			"END:VCARD").arg(name).arg(phone).arg(email);
		QByteArray vCardData = vCard.toUtf8();
		QImage qrImage;
		if (QRCodeUtil::getQRImage(qrImage, vCardData, 2, QColor("#000000")))
		{
			ui->labelVCard->setPixmap(QPixmap::fromImage(qrImage));
		}
	}
	else
	{
		ui->labelVCard->setPixmap(QPixmap());
	}
}

