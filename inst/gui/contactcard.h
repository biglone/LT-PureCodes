#ifndef CONTACTCARD_H
#define CONTACTCARD_H

#include "framelessdialog.h"
namespace Ui {class ContactCard;};
class QPropertyAnimation;
class QTimer;

class ContactCard : public FramelessDialog
{
	Q_OBJECT

public:
	ContactCard(QWidget *parent = 0);
	~ContactCard();

	void animateShow(bool visible);
	bool isCardShowing(const QString &id);

	void preHide();
	void preShowCard(const QString &id, const QPoint &topLeft = QPoint());

signals:
	void viewMaterial(const QString &id);
	void sendMail(const QString &id);

public slots:
	void setSkin();

protected:
	void enterEvent(QEvent *e);
	void leaveEvent(QEvent *e);

private slots:
	void preHideTimeout();
	void preShowTimeout();
	void on_avatarLabel_clicked();
	void on_nameLabel_clicked();
	void onDetailChanged(const QString &id);
	void on_emailLabel_clicked();

private:
	void setDetail();

private:
	Ui::ContactCard *ui;
	QPropertyAnimation *m_animationShow;
	QPropertyAnimation *m_animationHide;
	QTimer *m_preHideTimer;
	QTimer *m_preShowTimer;
	QString m_currentId;
};

#endif // CONTACTCARD_H
