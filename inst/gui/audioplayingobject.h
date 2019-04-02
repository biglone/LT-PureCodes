#ifndef AUDIOPLAYINGOBJECT_H
#define AUDIOPLAYINGOBJECT_H

#include <QObject>
#include <QTextObjectInterface>
#include <QTextFormat>

QT_BEGIN_NAMESPACE
class QTextDocument;
class QTextFormat;
class QPainter;
class QRectF;
class QSizeF;
QT_END_NAMESPACE

class AudioPlayingObject : public QObject, public QTextObjectInterface
{
	Q_OBJECT
		Q_INTERFACES(QTextObjectInterface)

public:
	QSizeF intrinsicSize(QTextDocument *doc, int posInDocument, const QTextFormat &format);
	void drawObject(QPainter *painter, const QRectF &rect, QTextDocument *doc, int posInDocument, const QTextFormat &format);
};

#endif // AUDIOPLAYINGOBJECT_H
