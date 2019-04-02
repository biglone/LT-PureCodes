#ifndef UPDATEDIALOG_H
#define UPDATEDIALOG_H

#include "framelessdialog.h"

namespace Ui {
	class CUpdateDialog;
}

class CUpdateDialog : public FramelessDialog
{
	Q_OBJECT

public:
	static CUpdateDialog *instance();

private:
	explicit CUpdateDialog(QWidget *parent = 0);

public:
	~CUpdateDialog();

	void setLastestVersion(const QString &toVer, const QString &toMsg, const QString &downloadUrl);

public slots:
	void setSkin();
	void startDownload();
	void slot_downloadProgress(qint64 nBytesReceived, qint64 nBytesTotal);
	void slot_downloadFinish(const QString& rsFilename);
	void checkIfUpdating();

protected:
	void closeEvent(QCloseEvent *e);
	void keyPressEvent(QKeyEvent *e);

private slots:
	void installUpdate();

	void onError(const QString& rsError);

	void onUpdatingChecked(bool update);

	void onProgressValueChanged(int v);

	void closeUpdate();

private:
	Ui::CUpdateDialog *ui;
	QString m_sFilename;
	QString m_toVersion;
	QString m_downloadUrl;
	bool    m_downloading;

	bool m_bSettedLastVersion;
	static CUpdateDialog *s_pIns;
};

#endif // UPDATEDIALOG_H
