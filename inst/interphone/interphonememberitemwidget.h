#ifndef INTERPHONEMEMBERITEMWIDGET_H
#define INTERPHONEMEMBERITEMWIDGET_H

#include <QWidget>
namespace Ui {class InterphoneMemberItemWidget;};

class InterphoneMemberItemWidget : public QWidget
{
	Q_OBJECT

public:
	InterphoneMemberItemWidget(QWidget *parent = 0);
	~InterphoneMemberItemWidget();

	void setId(const QString &id);

	void setAvatar(const QPixmap &avatar);
	void setName(const QString &name);

Q_SIGNALS:
	void chat(const QString &id);

private slots:
	void on_labelAvatar_clicked();

private:
	Ui::InterphoneMemberItemWidget *ui;

	QString m_id;
};

#endif // INTERPHONEMEMBERITEMWIDGET_H
