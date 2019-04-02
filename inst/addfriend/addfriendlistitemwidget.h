#ifndef ADDFRIENDLISTITEMWIDGET_H
#define ADDFRIENDLISTITEMWIDGET_H

#include <QWidget>
#include "addfriendmanager.h"
namespace Ui {class AddFriendListItemWidget;};

class AddFriendListItemWidget : public QWidget
{
	Q_OBJECT

public:
	AddFriendListItemWidget(QWidget *parent = 0);
	~AddFriendListItemWidget();

	void setIndex(int index);
	void setAvatar(const QPixmap &avatar);
	void setName(const QString &name);
	void setSex(int sex);
	void setDepart(const QString &dept);
	void setPhone(const QString &phone);
	void setDate(const QString &date);
	void setAction(const QString &fromId, const QString &toId, const QString &selfId, AddFriendManager::Action action);
	void setMessage(const QString &message);
	void setDeletable(bool deletable);

protected:
	void enterEvent(QEvent *e);
	void leaveEvent(QEvent *e);

private slots:
	void on_pushButtonAccept_clicked();
	void on_pushButtonRefuse_clicked();
	void on_labelAvatar_clicked();
	void on_pushButtonDelete_clicked();

signals:
	void accept(int index);
	void refuse(int index);
	void viewMaterial(int index);
	void deleteItem(int index);

private:
	Ui::AddFriendListItemWidget *ui;
	int  m_index;
	bool m_deletable;
};

#endif // ADDFRIENDLISTITEMWIDGET_H
