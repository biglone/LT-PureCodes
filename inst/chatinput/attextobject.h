#ifndef ATTEXTOBJECT_H
#define ATTEXTOBJECT_H

#include <QTextObjectInterface>

QT_BEGIN_NAMESPACE
class QTextDocument;
class QTextFormat;
class QPainter;
class QRectF;
class QSizeF;
QT_END_NAMESPACE

class CAtTextObject : public QObject, public QTextObjectInterface
{
Q_OBJECT
	Q_INTERFACES(QTextObjectInterface)

public:
	QSizeF intrinsicSize(QTextDocument *doc, int posInDocument, const QTextFormat &format);

	void drawObject(QPainter *painter, const QRectF &rect, QTextDocument *doc, int posInDocument, const QTextFormat &format);
};


#endif // ATTEXTOBJECT_H
