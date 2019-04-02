#ifndef SHAKEDIALOG_H
#define SHAKEDIALOG_H

#include "FramelessDialog.h"
#include <QPointer>
namespace Ui {class ShakeDialog;};
class QTimer;

class ShakeDialog : public FramelessDialog
{
	Q_OBJECT

public:
	ShakeDialog(QWidget *parent = 0);
	~ShakeDialog();
	void setUser(const QString &uid);
	void startShake();

	static void shake(const QString &uid);

Q_SIGNALS:
	void openChat(const QString &uid);

public slots:
	void setSkin();

private slots:
	void onTipClicked();
	void shakingTimeout();
	void closeTimeout();

private:
	void initShake();
	void initCloseTimer();

private:
	Ui::ShakeDialog *ui;
	QString          m_uid;

	QTimer        *m_shakingTimer;
	int            m_shakingCount;
	QList<QPoint>  m_shakingPosList;
	QRect          m_shakingFrameBak;

	QTimer        *m_closeTimer;
	int            m_closeCount;

	static QPointer<ShakeDialog> s_shakeDialog;
};

#endif // SHAKEDIALOG_H
