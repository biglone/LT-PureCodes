#ifndef ATMSGWIDGET_H
#define ATMSGWIDGET_H

#include <QWidget>
#include <QList>
namespace bean {class MessageBody;};
namespace Ui {class AtMsgWidget;};

class AtMsgWidget : public QWidget
{
	Q_OBJECT

	struct AtMsgData
	{
		QString atUid;
		QString fromId;
		QString atText;
	};

public:
	AtMsgWidget(QWidget *parent = 0);
	~AtMsgWidget();

	void setTopAtMsg(const bean::MessageBody &atMsg);
	void setBottomAtMsg(const bean::MessageBody &atMsg);
	void clearAtMsg();
	void removeAtMsg(const QString &atUid);

signals:
	void anchorAtMsg(const QString &atId);

protected:
	void paintEvent(QPaintEvent *e);

private slots:
	void takeAtMsg();

private:
	void setAtMsg(const AtMsgData &atMsg);

private:
	Ui::AtMsgWidget *ui;
	QList<AtMsgData> m_atMsgs;
};

#endif // ATMSGWIDGET_H
