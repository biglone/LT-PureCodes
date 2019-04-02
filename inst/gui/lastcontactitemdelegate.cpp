#include "lastcontactitemdelegate.h"
#include "model/lastcontactmodeldef.h"
#include "model/lastcontactitemdef.h"
#include <QPainter>
#include "PmApp.h"
#include "ModelManager.h"
#include "flickerhelper.h"
#include "presencemanager.h"
#include <QDateTime>
#include "common/datetime.h"
#include <QFont>
#include "login/Account.h"
#include "bean/bean.h"
#include <QBitmap>
#include "util/TextUtil.h"
#include "interphonemanager.h"
#include "emotion/EmotionUtil.h"
#include <QPixmap>
#include "DetailItem.h"
#include "Constants.h"

static const QSize kAvatarSize = QSize(42, 42);

LastContactItemDelegate::LastContactItemDelegate(QObject *parent)
: QItemDelegate(parent), m_lastContactModel(0), m_flickerHelper(0)
{
}

QSize LastContactItemDelegate::sizeHint(const QStyleOptionViewItem &/*option*/, const QModelIndex &/*index*/) const
{
	return QSize(58, 58);
}

void LastContactItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	if (m_lastContactModel == 0)
		return;

	LastContactItem *lastContactItem = m_lastContactModel->nodeFromProxyIndex(index);
	if (!lastContactItem)
		return;

	painter->save();
	painter->setRenderHints(QPainter::TextAntialiasing|QPainter::SmoothPixmapTransform|QPainter::HighQualityAntialiasing);

	// draw background
	QColor selectedBg("#fff5cc");
	QColor hoveredBg("#dfeefa");
	QRect paintRect = option.rect;
	paintRect.setHeight(paintRect.height() - 1);
	if (option.state & QStyle::State_Selected)
	{
		painter->fillRect(paintRect, selectedBg);
	}
	else if(option.state & QStyle::State_MouseOver)
	{
		painter->fillRect(paintRect, hoveredBg);
	}

	// draw avatar
	ModelManager *modelManager = qPmApp->getModelManager();
	QString id = lastContactItem->itemId();
	QString interphoneId;
	QPixmap avatar;
	if (lastContactItem->itemType() == LastContactItem::LastContactTypeGroupMuc)
	{
		interphoneId = InterphoneManager::attachTypeId2InterphoneId(bean::Message_GroupChat, id);

		// group default avatar
		avatar = modelManager->getGroupLogo(id);
	}
	else if (lastContactItem->itemType() == LastContactItem::LastContactTypeDiscuss)
	{
		interphoneId = InterphoneManager::attachTypeId2InterphoneId(bean::Message_DiscussChat, id);

		// discuss default avatar
		avatar = modelManager->discussLogo(id);
	}
	else if (lastContactItem->itemType() == LastContactItem::LastContactTypeMultiSend)
	{
		// multi send avatar
		QStringList members = lastContactItem->multiSendMemebers();
		avatar = modelManager->getMultiAvatar(110, members);
	}
	else
	{
		interphoneId = InterphoneManager::attachTypeId2InterphoneId(bean::Message_Chat, id);

		// contact status avatar
		avatar = modelManager->getUserAvatar(id);
	}

	QSize avatarSize(kAvatarSize);
	QRect rect = paintRect;
	rect.setWidth(avatarSize.width());
	rect.setHeight(avatarSize.height());
	rect.moveTop(rect.y() + (paintRect.height() - avatarSize.height())/2);
	rect.moveLeft(rect.x() + 5);
	if (m_flickerHelper)
	{
		bool needFlicker = false;
		if (lastContactItem->itemType() == LastContactItem::LastContactTypeContact)
		{
			needFlicker = m_flickerHelper->containsFlickerItem(lastContactItem->itemId(), bean::Message_Chat);
		}
		else if (lastContactItem->itemType() == LastContactItem::LastContactTypeGroupMuc)
		{
			needFlicker = m_flickerHelper->containsFlickerItem(lastContactItem->itemId(), bean::Message_GroupChat);
		}
		else if (lastContactItem->itemType() == LastContactItem::LastContactTypeDiscuss)
		{
			needFlicker = m_flickerHelper->containsFlickerItem(lastContactItem->itemId(), bean::Message_DiscussChat);
		}
		
		if (needFlicker)
		{
			int fIndex = m_flickerHelper->flickerIndex();
			if ((fIndex%4) == 1)
				rect.translate(QPoint(2, 2));
			else if ((fIndex%4) == 3)
				rect.translate(QPoint(-2, 2));
		}
	}
	avatar = avatar.scaled(rect.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
	QBitmap avatarMask(":/images/Icon_60_mask42.png");
	avatar.setMask(avatarMask);
	painter->drawPixmap(rect.topLeft(), avatar);

	if (lastContactItem->itemType() == LastContactItem::LastContactTypeContact)
	{
		if (id == QString(SUBSCRIPTION_ROSTER_ID))
		{
			int unreadMsgCount = lastContactItem->unreadMsgCount();
			if (unreadMsgCount > 0)
			{
				QPixmap msgStatus(":/images/Icon_19.png");
				QRect msgStatusRect = rect;
				msgStatusRect.setLeft(rect.right()-msgStatus.width()/2-2);
				msgStatusRect.setTop(rect.top()-msgStatus.height()/2+2);
				painter->drawPixmap(msgStatusRect.topLeft(), msgStatus);
			}
		}
	}
	else if (lastContactItem->itemType() == LastContactItem::LastContactTypeGroupMuc)
	{
		// draw message setting status
		AccountSettings::GroupMsgSettingType groupSetting = Account::settings()->groupMsgSetting(lastContactItem->itemId());
		if (groupSetting == AccountSettings::UnTip)
		{
			QPixmap msgStatus(":/images/Icon_108.png");
			QRect msgStatusRect = rect;
			msgStatusRect.setLeft(rect.right()-msgStatus.width());
			msgStatusRect.setTop(rect.bottom()-msgStatus.height());
			msgStatusRect.translate(3, 3);
			painter->drawPixmap(msgStatusRect.topLeft(), msgStatus);
		}
	}
	else if (lastContactItem->itemType() == LastContactItem::LastContactTypeDiscuss)
	{
		// draw message setting status
		AccountSettings::GroupMsgSettingType groupSetting = Account::settings()->discussMsgSetting(lastContactItem->itemId());
		if (groupSetting == AccountSettings::UnTip)
		{
			QPixmap msgStatus(":/images/Icon_108.png");
			QRect msgStatusRect = rect;
			msgStatusRect.setLeft(rect.right()-msgStatus.width());
			msgStatusRect.setTop(rect.bottom()-msgStatus.height());
			msgStatusRect.translate(3, 3);
			painter->drawPixmap(msgStatusRect.topLeft(), msgStatus);
		}
	}

	// draw time
	QDateTime msgDateTime = CDateTime::localDateTimeFromUtcString(lastContactItem->lastTime());
	QDateTime nowDateTime = CDateTime::currentDateTime();
	QString timeText;
	int days = msgDateTime.daysTo(nowDateTime);
	if (days == 0)
		timeText = msgDateTime.toString("hh:mm");
	else if (days == 1)
		timeText = tr("yesterday");
	else
		timeText = msgDateTime.toString("M-d");
	rect = paintRect;
	rect.setLeft(rect.left() + 5 + avatarSize.width() + 5);
	rect.setRight(rect.right() - 5);
	rect.setTop(rect.top() + 3);
	rect.setBottom(rect.bottom() - 3);
	rect.setHeight(rect.height()/2);
	QColor timeColor = QColor(136, 136, 136);
	QFont originalFont = painter->font();
	QFont smallFont = originalFont;
	smallFont.setPointSize(originalFont.pointSize() - originalFont.pointSize()/4);
	painter->setFont(smallFont);
	painter->setPen(timeColor);
	painter->drawText(rect, Qt::AlignRight|Qt::AlignVCenter, timeText);
	painter->setFont(originalFont);
	int timeWidth = option.fontMetrics.width(timeText);

	// draw interphone flag
	int interphoneWidth = 0;
	QPixmap interphoneFlag;
	QPoint ptInterphoneFlag;
	InterphoneState interphoneState = this->interphoneState(interphoneId);
	if (interphoneState == InterphoneNotIn)
	{
		interphoneFlag = QPixmap(":/interphone/listsound.png");
	}
	else if (interphoneState == InterphoneIn)
	{
		interphoneFlag = QPixmap(":/interphone/listsound2.png");
	}
	if (!interphoneFlag.isNull())
	{
		interphoneWidth = interphoneFlag.width();
		ptInterphoneFlag.setX(rect.right() - timeWidth - interphoneWidth);
		ptInterphoneFlag.setY(rect.top() + (rect.height() - interphoneFlag.height())/2);
		painter->drawPixmap(ptInterphoneFlag, interphoneFlag);
	}

	// draw name
	QFont orignalFont = painter->font();
	QFont newFont(orignalFont);
	newFont.setPointSizeF(orignalFont.pointSizeF() + 0.5);
	painter->setFont(newFont);

	QColor nameTextColor = QColor(0, 0, 0);
	QString text = lastContactItem->itemName();
	if (lastContactItem->itemType() == LastContactItem::LastContactTypeContact)
	{
		bean::DetailItem *detailItem = modelManager->detailItem(id);
		if (detailItem && detailItem->isDisabled())
		{
			text += tr("(invalid)");
			nameTextColor = QColor(128, 128, 128);
		}
	}

	painter->setPen(nameTextColor);
	rect.setRight(rect.right() - timeWidth - interphoneWidth - 5);
	text = QFontMetrics(newFont).elidedText(text, Qt::ElideRight, rect.width());
	painter->drawText(rect, Qt::AlignLeft|Qt::AlignVCenter, text);

	painter->setFont(orignalFont);

	// draw unread messages
	rect.moveTop(rect.top() + rect.height());
	int unreadMsgCount = lastContactItem->unreadMsgCount();
	QRect msgCountRect;
	if (unreadMsgCount > 0 && id != QString(SUBSCRIPTION_ROSTER_ID))
	{
		msgCountRect = rect;
		msgCountRect.setRight(paintRect.right() - 5);
		msgCountRect.setLeft(msgCountRect.right() - 20);
		msgCountRect.setTop(msgCountRect.top() + (msgCountRect.height() - 14)/2);
		msgCountRect.setHeight(14);

		// draw background
		QPixmap unreadBack(":/images/unread_back.png");
		painter->drawPixmap(msgCountRect.topLeft(), unreadBack);

		// draw count
		painter->setBrush(Qt::NoBrush);
		painter->setPen(QColor(255, 255, 255));
		painter->setFont(smallFont);
		QString unreadMsgCountText = QString::number(unreadMsgCount);
		if (unreadMsgCount > 99)
			unreadMsgCountText = QString("99+");
		painter->drawText(msgCountRect, Qt::AlignHCenter|Qt::AlignVCenter, unreadMsgCountText);
		painter->setFont(originalFont);
	}

	// draw last message
	if (msgCountRect.width() > 0)
		rect.setRight(paintRect.right() - 5 - msgCountRect.width() - 5);
	else
		rect.setRight(paintRect.right() - 5);
	QString lastMessage = lastContactItem->lastBody();
	if (!lastMessage.isEmpty())
	{
		QString sendUid = lastContactItem->sendUid();
		if (!sendUid.isEmpty())
		{
			if (lastContactItem->itemType() == LastContactItem::LastContactTypeGroupMuc)
			{
				QString sendName = modelManager->memberNameInGroup(id, sendUid);
				lastMessage.prepend(sendName+QString(": "));
			}
			else if (lastContactItem->itemType() == LastContactItem::LastContactTypeDiscuss)
			{
				QString sendName = modelManager->memberNameInDiscuss(id, sendUid);
				lastMessage.prepend(sendName+QString(": "));
			}
		}

		painter->setPen(QColor(136, 136, 136));
		// draw text with emotion
		int emotFromIndex = 0;
		QRect leftRect = rect;
		int textPartLen = 0;
		QString textPart;
		bool textFinished = false;
		QStringList faceNames = EmotionUtil::instance().emotionCodeNames();
		QStringList faceFileNames = EmotionUtil::instance().emotionFilePathes();
		QString faceName;
		QString faceFileName;
		int emotIndex = lastMessage.indexOf(QChar('['), emotFromIndex);
		while (emotIndex != -1)
		{
			textPart = lastMessage.mid(emotFromIndex, emotIndex-emotFromIndex);
			if (!textPart.isEmpty())
			{
				textPartLen = option.fontMetrics.width(textPart);
				if (textPartLen >= leftRect.width())
				{
					textPart = TextUtil::trimToLen(option.font, textPart, leftRect.width());
					painter->drawText(leftRect, Qt::AlignLeft|Qt::AlignVCenter|Qt::TextSingleLine, textPart);
					textFinished = true;
					break;
				}
				else
				{
					painter->drawText(leftRect, Qt::AlignLeft|Qt::AlignVCenter|Qt::TextSingleLine, textPart);
					leftRect.setLeft(leftRect.left() + textPartLen);
				}
			}

			bool isFace = false;
			for (int i = 0; i < faceNames.length(); ++i)
			{
				faceName = faceNames.value(i);
				faceFileName = faceFileNames.value(i);
				int checkIndex = lastMessage.indexOf(QString("[")+faceName+QString("]"), emotIndex);
				if (checkIndex == emotIndex)
				{
					isFace = true;
					break;
				}
			}

			if (isFace)
			{
				// draw face
				QPixmap face(faceFileName);
				// face = face.scaled(QSize(20, 20), Qt::KeepAspectRatio, Qt::SmoothTransformation);
				if (face.width() > leftRect.width())
				{
					textFinished = true;
					break;
				}

				QPoint facePt;
				facePt.setX(leftRect.left());
				facePt.setY(leftRect.top() + (leftRect.height()-face.height())/2);
				painter->drawPixmap(facePt, face);
				leftRect.setLeft(leftRect.left() + face.width());

				emotFromIndex = emotIndex + (2+faceName.length());
			}
			else
			{
				// draw [
				textPart = lastMessage.mid(emotIndex, 1);
				textPartLen = option.fontMetrics.width(textPart);
				if (textPartLen > leftRect.width())
				{
					textFinished = true;
					break;
				}
				else
				{
					painter->drawText(leftRect, Qt::AlignLeft|Qt::AlignVCenter|Qt::TextSingleLine, textPart);
					leftRect.setLeft(leftRect.left() + textPartLen);
				}

				emotFromIndex = emotIndex + 1; // move one character
			}

			emotIndex = lastMessage.indexOf(QChar('['), emotFromIndex);
		}

		if (!textFinished)
		{
			textPart = lastMessage.mid(emotFromIndex);
			if (!textPart.isEmpty())
			{
				textPart = TextUtil::trimToLen(option.font, textPart, leftRect.width());
				painter->drawText(leftRect, Qt::AlignLeft|Qt::AlignVCenter|Qt::TextSingleLine, textPart);
				textFinished = true;
			}
		}
	}

	painter->restore();
}

void LastContactItemDelegate::setInterphoneState(const QString &interphoneId, InterphoneState state)
{
	if (state == InterphoneNone)
	{
		if (m_interphoneStates.contains(interphoneId))
		{
			m_interphoneStates.remove(interphoneId);
		}
	}
	else
	{
		m_interphoneStates[interphoneId] = state;
	}
}

LastContactItemDelegate::InterphoneState LastContactItemDelegate::interphoneState(const QString &interphoneId) const
{
	if (interphoneId.isEmpty())
		return InterphoneNone;

	if (m_interphoneStates.contains(interphoneId))
	{
		return m_interphoneStates[interphoneId];
	}
	else
	{
		return InterphoneNone;
	}
}

void LastContactItemDelegate::clearInterphoneStates()
{
	m_interphoneStates.clear();
}