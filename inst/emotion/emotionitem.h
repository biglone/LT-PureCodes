#ifndef EMOTIONITEM_H
#define EMOTIONITEM_H

#include <QLabel>
#include "emotionconsts.h"

enum EmotionType
{
	DefaultEmotion = 0,
	FavoriteEmotion = 1
};

class QMovie;

//////////////////////////////////////////////////////////////////////////
// CLASS EmotionItem
class EmotionItem : public QLabel
{
	Q_OBJECT

public:
	EmotionItem(QWidget *parent = 0);

public slots:
	void setEmotion(EmotionType emotionType, QMovie *movie);
		
	void setEmotionId(const QString &id);
	QString emotionId() const;

	void setSkin();

protected:
	bool event(QEvent *e);
	void paintEvent(QPaintEvent *e);

private slots:
	void onFrameChanged(int frameNum);

private:
	QMovie      *m_emotion;
	EmotionType  m_emotionType;
	QString      m_emotionId;
	bool         m_hovered;
};


#endif // EMOTIONITEM_H
