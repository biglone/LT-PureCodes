#include "contactcard.h"
#include "ui_contactcard.h"
#include "PmApp.h"
#include "model/ModelManager.h"
#include "bean/DetailItem.h"
#include "model/orgstructmodeldef.h"
#include <QPropertyAnimation>
#include <QTimer>
#include "QRCodeUtil.h"
#include "settings/GlobalSettings.h"

ContactCard::ContactCard(QWidget *parent)
	: FramelessDialog(parent)
{
	ui = new Ui::ContactCard();
	ui->setupUi(this);

	setMouseTracking(true);
	setAttribute(Qt::WA_Hover, true);
	setWindowFlags(windowFlags()|Qt::Dialog|Qt::Popup);
	setWindowOpacity(0.0);
	hide();

	if (GlobalSettings::introductionViewType() == 0)
	{
		ui->signatureBar->setVisible(true);
	}
	else
	{
		ui->signatureBar->setVisible(false);
	}

	m_animationShow = new QPropertyAnimation(this, "windowOpacity");
	m_animationShow->setDuration(500);
	m_animationShow->setEndValue(1.0);

	m_animationHide = new QPropertyAnimation(this, "windowOpacity");
	m_animationHide->setDuration(500);
	m_animationHide->setEndValue(0.0);
	connect(m_animationHide, SIGNAL(finished()), SLOT(hide()));

	m_preShowTimer = new QTimer(this);
	m_preShowTimer->setInterval(800);

	m_preHideTimer = new QTimer(this);
	m_preHideTimer->setInterval(800);

	connect(m_preShowTimer, SIGNAL(timeout()), SLOT(preShowTimeout()));
	connect(m_preHideTimer, SIGNAL(timeout()), SLOT(preHideTimeout()));

	if (GlobalSettings::introductionViewType() == 0)
	{
		setFixedSize(312, 199);
	}
	else
	{
		setFixedSize(312, 179);
	}
	
	setResizeable(false);
	setMoveAble(false);

	ModelManager *modelManager = qPmApp->getModelManager();
	connect(modelManager, SIGNAL(detailChanged(QString)), this, SLOT(onDetailChanged(QString)));

	setSkin();
}

ContactCard::~ContactCard()
{
	delete ui;
}

void ContactCard::animateShow(bool visible)
{
	if (visible) 
	{
		m_animationHide->stop();
		setWindowOpacity(0.0);
		if (!isVisible())
		{
			show();
		}
		m_animationShow->start(QAbstractAnimation::KeepWhenStopped);
	} 
	else 
	{
		setWindowOpacity(1.0);
		m_animationHide->setEndValue(0.0);
		m_animationHide->start(QAbstractAnimation::KeepWhenStopped);
	}
}

bool ContactCard::isCardShowing(const QString &id)
{
	if (!isVisible())
		return false;
	if (m_currentId == id
		&& (windowOpacity() == 1.0 || m_animationShow->state() == QAbstractAnimation::Running))
	{
		return true;
	}
	return false;
}

void ContactCard::preHide()
{
	m_preShowTimer->stop();
	if (!isVisible() || windowOpacity() == 0.0 || m_animationHide->state() == QAbstractAnimation::Running
		|| m_preHideTimer->isActive())
		return;

	if (m_animationShow->state() == QAbstractAnimation::Running)
	{
		m_animationHide->start(QAbstractAnimation::KeepWhenStopped);
		return;
	}

	m_preHideTimer->start();
}

void ContactCard::preShowCard(const QString &id, const QPoint &topLeft /*= QPoint()*/)
{
	m_preHideTimer->stop();
	if (isVisible() && m_currentId == id && this->pos() == topLeft
		&& (windowOpacity() == 1.0 || m_animationShow->state() == QAbstractAnimation::Running
		|| m_preShowTimer->isActive()))
		return;

	move(topLeft);
	m_currentId = id;

	setDetail();
	
	if ((isVisible() && windowOpacity() == 1.0) || m_animationShow->state() == QAbstractAnimation::Running)
	{
		return;
	}

	if (m_animationHide->state() == QAbstractAnimation::Running)
	{
		m_animationShow->start(QAbstractAnimation::KeepWhenStopped);
		return;
	}

	m_preShowTimer->start();
}

void ContactCard::setSkin()
{
	// set background image
	QPixmap bgPixmap(":/theme/dialog_bgs/dialog_bg_6.png");
	FramelessDialog::BGSizes bgSizes = {0};
	bgSizes.borderwidth = 1;
	bgSizes.bottomBarHeight = 68;
	setBG(bgPixmap, bgSizes);

	setStyleSheet("font-size: 9pt;");

	ui->telTag->setPixmap(QPixmap(":/images/Icon_50_2.png"));
	ui->emailTag->setPixmap(QPixmap(":/images/Icon_133.png"));

	ui->labelVCard->setStyleSheet("background-color: white;");
	ui->signatureTag->setStyleSheet("color: rgb(128, 128, 128);");
	ui->signatureLabel->setStyleSheet("color:rgb(128, 128, 128);");

	ui->avatarLabel->setClickable(true);
	ui->nameLabel->setFontAtt(QColor("#000000"), 11, false);
	ui->emailLabel->setFontAtt(QColor(0, 120, 216), 9, false);
}

void ContactCard::enterEvent(QEvent *e)
{
	m_preHideTimer->stop();
	if(m_animationHide->state() == QAbstractAnimation::Running)
	{
		m_animationShow->start(QAbstractAnimation::KeepWhenStopped);
		return;
	}
	QDialog::enterEvent(e);
}

void ContactCard::leaveEvent(QEvent *e)
{
	preHide();
	QDialog::leaveEvent(e);
}

void ContactCard::preHideTimeout()
{
	m_preHideTimer->stop();
	animateShow(false);
}

void ContactCard::preShowTimeout()
{
	m_preShowTimer->stop();
	animateShow(true);
}

void ContactCard::on_avatarLabel_clicked()
{
	emit viewMaterial(m_currentId);
}

void ContactCard::on_nameLabel_clicked()
{
	emit viewMaterial(m_currentId);
}

void ContactCard::onDetailChanged(const QString &id)
{
	if (id == m_currentId)
	{
		setDetail();
	}
}

void ContactCard::on_emailLabel_clicked()
{
	if (!ui->emailLabel->text().isEmpty())
	{
		emit sendMail(m_currentId);
	}
}

void ContactCard::setDetail()
{
	// set all the content
	ModelManager *modelManager = qPmApp->getModelManager();
	QPixmap avatar = modelManager->getUserAvatar(m_currentId);
	avatar = avatar.scaled(ui->avatarLabel->size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
	ui->avatarLabel->setPixmap(avatar);

	ui->nameLabel->setText(modelManager->userName(m_currentId));

	bean::DetailItem *item = modelManager->detailItem(m_currentId);
	if (item)
	{
		int sex = item->sex();
		if (sex < 0 && sex > 1)
		{
			sex = 9;
		}
		if (sex == 0)
		{
			// female
			ui->genderLabel->setPixmap(QPixmap(":/images/Icon_95.png"));
		}
		else if (sex == 1)
		{
			// male
			ui->genderLabel->setPixmap(QPixmap(":/images/Icon_94.png"));
		}
		else
		{
			// secret
			ui->genderLabel->setPixmap(QPixmap());
		}

		QString posName = item->duty();
		QString deptName = item->depart();
		ui->deptLabel->setText(deptName);
		ui->dutyLabel->setText(posName);

		ui->telLabel->setText(item->phone1());

		ui->emailLabel->setText(item->email());

		QString signature = item->message();
		if (signature.isEmpty())
		{
			ui->signatureLabel->setText(tr("No what's up"));
		}
		else
		{
			ui->signatureLabel->setText(item->message());
		}

		QString name = ui->nameLabel->text();
		QString tel = ui->telLabel->text();
		QString mail = ui->emailLabel->text();
		if (!name.isEmpty() && (!tel.isEmpty() || !mail.isEmpty()))
		{
			ui->labelVCard->setVisible(true);
			QString vCard = QString("BEGIN:VCARD\n"
				"FN:%1\n"
				"TEL:%2\n"
				"EMAIL:%3\n"
				"END:VCARD").arg(name).arg(tel).arg(mail);
			QByteArray vCardData = vCard.toUtf8();
			QImage qrImage;
			if (QRCodeUtil::getQRImage(qrImage, vCardData, 2, QColor("#000000")))
			{
				if (qrImage.height() > ui->labelVCard->height())
					qrImage = qrImage.scaled(ui->labelVCard->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
				ui->labelVCard->setPixmap(QPixmap::fromImage(qrImage));
			}
		}
		else
		{
			ui->labelVCard->setVisible(false);
		}
	}
}

