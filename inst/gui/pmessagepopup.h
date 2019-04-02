#ifndef PMESSAGEPOPUP_H
#define PMESSAGEPOPUP_H

#include "framelessdialog.h"
#include <QTimer>
#include <QPoint>
namespace Ui {class PMessagePopup;};
class QFocusEvent;

class PMessagePopup : public FramelessDialog
{
	Q_OBJECT

public:
	enum Type{
		Success,
		Failed,
		Question,
		Information,
		Warning
	};

public:
	static void critical(const QString &text, QPoint contextPt, QWidget *parent = 0);
	static void information(const QString &text, QPoint contextPt, QWidget *parent = 0);
	static void question(const QString &text, QPoint contextPt, QWidget *parent = 0);
	static void warning(const QString &text, QPoint contextPt, QWidget *parent = 0);
	static void success(const QString &text, QPoint contextPt, QWidget *parent = 0);

	PMessagePopup(Type popupType, const QString &text, QWidget *parent = 0);
	~PMessagePopup();

public slots:
	void setSkin();

protected:
	void focusOutEvent(QFocusEvent *ev);

private slots:
	void onHideTimeout();

private:
	Ui::PMessagePopup *ui;
	QTimer m_hideTimer;
};

#endif // PMESSAGEPOPUP_H
