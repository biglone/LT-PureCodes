#include "emotionitem.h"
#include <QMovie>
#include <QEvent>
#include <QDebug>
#include <QPainter>

EmotionItem::EmotionItem(QWidget *parent /*= 0*/)
 : QLabel(parent), m_emotion(0), m_emotionType(DefaultEmotion), m_hovered(false)
{
	setAttribute(Qt::WA_Hover, true);
	setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
	setFocusPolicy(Qt::NoFocus);
	setSkin();
}

void EmotionItem::setEmotion(EmotionType emotionType, QMovie *movie)
{
	if (!movie)
		return;

	if (!movie->isValid())
		return;

	movie->setParent(this);
	movie->setCacheMode(QMovie::CacheAll);
	movie->jumpToFrame(0);

	m_emotionType = emotionType;
	m_emotion = movie;
	onFrameChanged(0);
	connect(m_emotion, SIGNAL(frameChanged(int)), this, SLOT(onFrameChanged(int)));
}

void EmotionItem::setEmotionId(const QString &id)
{
	m_emotionId = id;
}

QString EmotionItem::emotionId() const
{
	return m_emotionId;
}

void EmotionItem::setSkin()
{
	this->setStyleSheet(QString(
		"EmotionItem {"
		"	margin: 1px;"
		"}"
		));
}

bool EmotionItem::event(QEvent *e)
{
	do 
	{
		QMovie *pMovie = m_emotion;
		if (!pMovie)
			break;

		if (!pMovie->isValid())
		{
			qDebug() << Q_FUNC_INFO << " EmotionItem movie is not valid";
			break;
		}

		switch (e->type())
		{
		case QEvent::HoverEnter:
			m_hovered = true;
			if (QMovie::Running != pMovie->state())
			{
				pMovie->start();
			}
			update();
			break;
		case QEvent::HoverLeave:
			m_hovered = false;
			if (QMovie::NotRunning != pMovie->state())
			{
				pMovie->stop();
			}
			pMovie->jumpToFrame(0);
			update();
			break;
		case QEvent::HoverMove:
			break;
		}
	} while (0);

	return QLabel::event(e);
}

void EmotionItem::paintEvent(QPaintEvent * /*e*/)
{
	QPainter painter(this);
	QRect rt = rect();
	rt.adjust(1, 1, -1, -1);

	if (!m_emotion)
	{
		painter.fillRect(rt, QColor(246, 251, 254));
		return;
	}

	if (!m_hovered)
	{
		painter.fillRect(rt, QColor(246, 251, 254));
	}
	else
	{
		painter.fillRect(rt, QColor(255, 255, 255));
	}

	QPoint center = rt.center();
	QPixmap frame = m_emotion->currentPixmap();
	if (m_emotionType == FavoriteEmotion)
	{
		if (frame.width() > kFavoriteEmotionSize || frame.height() > kFavoriteEmotionSize)
		{
			frame = frame.scaled(QSize(kFavoriteEmotionSize, kFavoriteEmotionSize), Qt::KeepAspectRatio, Qt::SmoothTransformation);
		}
	}
	QPoint pt = center;
	pt += QPoint(-frame.width()/2, -frame.height()/2);
	pt.setY(pt.y()+1);
	if (m_hovered)
		pt += QPoint(1, -1);
	painter.drawPixmap(pt, frame);
}

void EmotionItem::onFrameChanged(int /*frameNum*/)
{
	QMovie *pMovie = m_emotion;
	if (!pMovie)
		return;

	if (!pMovie->isValid())
	{
		qDebug() << Q_FUNC_INFO << " EmotionItem movie is not valid";
		return;
	}

	update();
}