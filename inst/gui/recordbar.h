#ifndef RECORDBAR_H
#define RECORDBAR_H

#include <QWidget>
namespace Ui {class RecordBar;};

class RecordBar : public QWidget
{
	Q_OBJECT

public:
	RecordBar(QWidget *parent = 0);
	~RecordBar();

	void setRecordTime(int ms);
	int recordTime() const;

	void setRunning(bool running);
	bool isRunning() const;

signals:
	void sendRecord();
	void cancelRecord();

protected:
	void paintEvent(QPaintEvent *e);

private slots:
	void on_pushButtonSend_clicked();
	void on_pushButtonCancel_clicked();

private:
	Ui::RecordBar *ui;
	int  m_recordTimeInMs;
	bool m_running;
};

#endif // RECORDBAR_H
