#ifndef COMBOUSERITEMWIDGET_H
#define COMBOUSERITEMWIDGET_H

#include <QWidget>
namespace Ui {class ComboUserItemWidget;};

class ComboUserItemWidget : public QWidget
{
	Q_OBJECT

public:
	ComboUserItemWidget(QWidget *parent = 0);
	~ComboUserItemWidget();

signals:
	void deleteUser(const QString &user);

public:
	void setAvatar(const QPixmap &avatar);
	void setName(const QString &name);

protected:
	void enterEvent(QEvent *e);
	void leaveEvent(QEvent *e);

private slots:
	void on_closeToolButton_clicked();

private:
	Ui::ComboUserItemWidget *ui;
};

#endif // COMBOUSERITEMWIDGET_H
