#include "audioplayingobject.h"
#include <QVariant>
#include <QImage>
#include <QPainter>
#include "chatinputdef.h"

QSizeF AudioPlayingObject::intrinsicSize(QTextDocument *doc, int posInDocument, const QTextFormat &format)
{
	Q_UNUSED(doc);
	Q_UNUSED(posInDocument);

	QVariant frameVar = format.property(PM::AudioPlayingFrame);
	QImage frame = frameVar.value<QImage>();
	return frame.size();
}

void AudioPlayingObject::drawObject(QPainter *painter, const QRectF &rect, QTextDocument *doc, int posInDocument, const QTextFormat &format)
{
	Q_UNUSED(doc);
	Q_UNUSED(posInDocument);

	painter->save();

	QVariant frameVar = format.property(PM::AudioPlayingFrame);
	QImage frame = frameVar.value<QImage>();
	if (!frame.isNull())
	{
		QRect rt = rect.toRect();
		painter->drawImage(rt.topLeft(), frame);
	}

	painter->restore();
}