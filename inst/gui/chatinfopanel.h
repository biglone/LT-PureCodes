#ifndef CHATINFOPANEL_H
#define CHATINFOPANEL_H

#include <QWidget>
namespace Ui {class ChatInfoPanel;};

class ChatInfoPanel : public QWidget
{
	Q_OBJECT

public:
	ChatInfoPanel(QWidget *parent = 0);
	~ChatInfoPanel();

public slots:
	void setSkin();
	void updateInfo(const QString &uid);

private:
	Ui::ChatInfoPanel *ui;
};

#endif // CHATINFOPANEL_H
