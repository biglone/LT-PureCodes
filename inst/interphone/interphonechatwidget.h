#ifndef INTERPHONECHATWIDGET_H
#define INTERPHONECHATWIDGET_H

#include <QWidget>
namespace Ui {class InterphoneChatWidget;};

class InterphoneChatWidget : public QWidget
{
	Q_OBJECT

public:
	InterphoneChatWidget(QWidget *parent = 0);
	~InterphoneChatWidget();

	bool isInInterphone() const;
	void setInInterphone(bool in, const QString &tip);

Q_SIGNALS:
	void addInterphone();
	void openInterphone();
	void quitInterphone();

protected:
	void paintEvent(QPaintEvent *ev);

private:
	Ui::InterphoneChatWidget *ui;

	bool m_isInInterphone;
};

#endif // INTERPHONECHATWIDGET_H
