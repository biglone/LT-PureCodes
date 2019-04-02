#ifndef BLACKLISTITEMWIDGET_H
#define BLACKLISTITEMWIDGET_H

#include <QWidget>
namespace Ui {class BlackListItemWidget;};

class BlackListItemWidget : public QWidget
{
	Q_OBJECT

public:
	BlackListItemWidget(QWidget *parent = 0);
	~BlackListItemWidget();
	
	void setId(const QString &id);
	void setAvatar(const QPixmap &avatar);
	void setNameText(const QString &text);

Q_SIGNALS:
	void removeBlack(const QString &id);

protected:
	void enterEvent(QEvent *e);
	void leaveEvent(QEvent *e);

private slots:
	void on_pushButtonRemove_clicked();

private:
	Ui::BlackListItemWidget *ui;

	QString m_id;
};

#endif // BLACKLISTITEMWIDGET_H
