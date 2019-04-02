#ifndef DEBUGDLG_H
#define DEBUGDLG_H

#include <QDialog>
#include <QList>
#include "logger/logger.h"

namespace Ui {
    class CDebugDlg;
}

class QxtGlobalShortcut;

class CDebugDlg : public QDialog
{
    Q_OBJECT

public:
    explicit CDebugDlg(QWidget *parent = 0);
    ~CDebugDlg();

	static CDebugDlg* getDebugDlg();

public slots:
	void logMessage(Logger::MessageType type, const QString &text);
	void slot_show();

signals:
	void retriveSystemTrayIconStatus();
	void restartSystemTrayIcon();
	void showMainWindow();

protected:
	void showEvent(QShowEvent *event);

private slots:
	void on_pBnSend_clicked();
	void on_pBnClose_clicked();
	void on_pBtnSession_clicked();
	void slot_clear();
	void slot_inputTextChanged();
	void on_pBnOpenLog_clicked();
	void on_pBnLogDir_clicked();

private:
	struct LogItem {
		QColor  color;
		QString log;
	};

	void appendLog(const QColor &color, const QString &log);
	void appendDebug(const QColor &color, const QString &log);
	
private:
    Ui::CDebugDlg*     ui;
	QxtGlobalShortcut* m_pDebugShortcut;          /// 调试窗口全局快捷键
	QList<LogItem>     m_cacheLogs;

	static CDebugDlg*  s_instance;     
};

#endif // DEBUGDLG_H
