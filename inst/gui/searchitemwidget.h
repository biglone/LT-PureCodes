#ifndef SEARCHITEMWIDGET_H
#define SEARCHITEMWIDGET_H

#include <QWidget>
#include <QString>

class QPixmap;

namespace Ui {class SearchItemWidget;};

class SearchItemWidget : public QWidget
{
	Q_OBJECT

public:
	SearchItemWidget(QWidget *parent = 0);
	~SearchItemWidget();

	void setId(const QString &id);
	QString id() const;
	void setAvatar(const QPixmap &avatar);
	void setName(const QString &name, const QColor &color = QColor(0, 0, 0));
	void setSex(int sex);
	void setDepart(const QString &depart);
	void setPhone(const QString &phone);
	void setAddEnabled(bool enabled);

Q_SIGNALS:
	void addFriend(const QString &id);
	void showMaterial(const QString &id);

private slots:
	void on_pushButtonAdd_clicked();
	void on_labelAvatar_clicked();

private:
	Ui::SearchItemWidget *ui;

	QString m_id;
};

#endif // SEARCHITEMWIDGET_H
