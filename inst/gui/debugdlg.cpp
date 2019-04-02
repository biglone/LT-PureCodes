#include <QTextStream>
#include <QTime>
#include "QxtGlobalShortcut.h"

#include <Windows.h>

#include "debugdlg.h"
#include "ui_debugdlg.h"

#include <QDomDocument>

#include <QDir>
#include <QFileInfo>
#include <QProcess>
#include <QDebug>
#include <QDesktopServices>
#include <QStandardPaths>
#include "Constants.h"
#include "util/ExplorerUtil.h"

static const int MAX_CACHE_LOG_COUNT = 50;

CDebugDlg* CDebugDlg::s_instance = 0;

CDebugDlg::CDebugDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CDebugDlg),
	m_pDebugShortcut(0)
{
    ui->setupUi(this);

	setWindowTitle(QString("Console"));

	setAttribute(Qt::WA_DeleteOnClose, false);

	ui->tabWidget->setCurrentIndex(0);

	ui->labelInfo->setText(tr("Colors: <font color=darkgreen>send messages</font> <font color=darkblue>received messages</font> <font color=brown>debug information</font> <font color=darkCyan>application information</font> <font color=red>warning</font> <font color=black>other</font>"));

	ui->pBnSend->setEnabled(false);

	ui->pBtnSession->setEnabled(false);

	/*
#ifdef NDEBUG
	ui->cBoxStop->setChecked(true);
#elif
	ui->cBoxStop->setChecked(false);
#endif
	*/

	ui->cBoxCloseDebug->setChecked(false);

	/*
	m_pDebugShortcut = new QxtGlobalShortcut(QKeySequence("Ctrl+Shift+D"), this);
	connect(m_pDebugShortcut, SIGNAL(activated()), this, SLOT(slot_show()));
	*/

	connect(ui->pBnClear, SIGNAL(clicked()), this, SLOT(slot_clear()));
	connect(ui->tEditInput, SIGNAL(textChanged()), this, SLOT(slot_inputTextChanged()));
}

CDebugDlg::~CDebugDlg()
{
    delete ui;

	s_instance = 0;
}

CDebugDlg* CDebugDlg::getDebugDlg()
{
	if (!s_instance)
		s_instance = new CDebugDlg();
	return s_instance;
}

void CDebugDlg::logMessage(Logger::MessageType type, const QString &text)
{
	if (ui->cBoxStop->isChecked() || text.isEmpty())
		return;

	bool needAppendDebug = false;
	QColor fontColor;
	switch (type)
	{
		case Logger::DebugMessage:
			fontColor = QColor("brown");
			needAppendDebug = true;
			break;
		case Logger::InformationMessage:
			fontColor = Qt::darkCyan;
			needAppendDebug = true;
			break;
		case Logger::WarningMessage:
			fontColor = Qt::red;
			needAppendDebug = true;
			break;
		case Logger::ReceivedMessage:
			fontColor = Qt::darkBlue;
			needAppendDebug = false;
			break;
		case Logger::SentMessage:
			fontColor = Qt::darkGreen;
			needAppendDebug = false;
			break;
		default:
			fontColor = Qt::black;
			needAppendDebug = false;
			break;
	}

	QString formattedText;
	if (!needAppendDebug)
	{
		QDomDocument doc;
		bool isXml = doc.setContent(text);
		if (isXml)
		{
			QTextStream stream(&formattedText);
			doc.save(stream, 2);
		}
		else
		{
			formattedText = text;
		}
	}
	else
	{
		formattedText = text;
	}

	if (!isVisible())
	{
		// cache the log items
		LogItem logItem;
		logItem.color = fontColor;
		logItem.log = formattedText;
		m_cacheLogs.append(logItem);
		if (m_cacheLogs.count() > MAX_CACHE_LOG_COUNT)
		{
			// clear the previous log window first
			ui->tBrwsView->clear();

			// remove extra cache logs
			while (m_cacheLogs.count() > MAX_CACHE_LOG_COUNT)
				m_cacheLogs.removeAt(0);
		}
	}
	else
	{
		// append to log window
		appendLog(fontColor, formattedText);
	}

	// debug information need to be added always
	if (needAppendDebug && !ui->cBoxCloseDebug->isChecked())
	{
		appendDebug(fontColor, formattedText);
	}
}

void CDebugDlg::showEvent(QShowEvent *event)
{
	foreach (LogItem logItem, m_cacheLogs)
	{
		appendLog(logItem.color, logItem.log);
	}
	m_cacheLogs.clear();

	QDialog::showEvent(event);
}

void CDebugDlg::on_pBnSend_clicked()
{
	// to do...
}

void CDebugDlg::on_pBnClose_clicked()
{
	hide();
}

void CDebugDlg::on_pBtnSession_clicked()
{
	// to do...
}

void CDebugDlg::slot_show()
{
	show();

#ifdef Q_WS_WIN
	::SetWindowPos(this->effectiveWinId(), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
	::SetWindowPos(this->effectiveWinId(), HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
#endif
}

void CDebugDlg::slot_clear()
{
	ui->tBrwsView->clear();
	ui->tBrwsViewDebug->clear();
}

void CDebugDlg::slot_inputTextChanged()
{
	QString text = ui->tEditInput->toPlainText();
	if (text == QString("#systemtrayicon_status#"))
	{
		emit retriveSystemTrayIconStatus();
		ui->tEditInput->clear();
	}
	else if (text == QString("#systemtrayicon_restart#"))
	{
		emit restartSystemTrayIcon();
		ui->tEditInput->clear();
	}
	else if (text == QString("#mainwindow_open#"))
	{
		emit showMainWindow();
		ui->tEditInput->clear();
	}
}

void CDebugDlg::on_pBnOpenLog_clicked()
{
	QDir logDir = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
	logDir.mkpath("log");
	logDir.cd("log");
	int nPid = QApplication::applicationPid();
	QString logNamePattern = QString("%1_%2_*").arg(APP_NAME).arg(nPid);
	QFileInfoList fileInfoList = logDir.entryInfoList(QStringList() << logNamePattern, 
		QDir::Files|QDir::NoDotAndDotDot|QDir::NoSymLinks);
	if (!fileInfoList.isEmpty())
	{
		QFileInfo logFileInfo = fileInfoList[0];
		QString filePath = logFileInfo.absoluteFilePath();
		QDesktopServices::openUrl(filePath);
		return;
	}
}

void CDebugDlg::on_pBnLogDir_clicked()
{
	QDir logDir = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
	logDir.mkpath("log");
	logDir.cd("log");
	int nPid = QApplication::applicationPid();
	QString logNamePattern = QString("%1_%2_*").arg(APP_NAME).arg(nPid);
	QFileInfoList fileInfoList = logDir.entryInfoList(QStringList() << logNamePattern, 
		QDir::Files|QDir::NoDotAndDotDot|QDir::NoSymLinks);
	if (!fileInfoList.isEmpty())
	{
		QFileInfo logFileInfo = fileInfoList[0];
		ExplorerUtil::selectFile(logFileInfo);
	}
}

void CDebugDlg::appendLog(const QColor &color, const QString &log)
{
	ui->tBrwsView->setTextColor(color);
	ui->tBrwsView->append(log);
}

void CDebugDlg::appendDebug(const QColor &color, const QString &log)
{
	if (ui->tBrwsViewDebug->toPlainText().length() > 16*1024) // max is 16K
	{
		ui->tBrwsViewDebug->clear();
	}

	QTime now = QTime::currentTime();
	QString nowStr = now.toString("[hh:mm:ss.zzz]  ");
	QString logWithTime = nowStr + log;
	ui->tBrwsViewDebug->setTextColor(color);
	ui->tBrwsViewDebug->append(logWithTime);
}
