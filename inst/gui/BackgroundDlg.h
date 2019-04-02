#ifndef BACKGROUNDDLG_H
#define BACKGROUNDDLG_H

#include <QPointer>
#include <QDialog>

class BackgroundDlg : public QWidget
{
	Q_OBJECT

public:
	explicit BackgroundDlg(QWidget *parent = 0);
	virtual ~BackgroundDlg();

	void loginWithId(const QString &uid);

	void setVisible(bool visible);

public slots:
	void onLogined();
	void showUpdateDlg(const QString &toVer, const QString &toMsg, const QString &downloadUrl);
	void onCompanyLoginFailed(const QString &desc);

private slots:
	void createLoginDlg();
	void createPscDlg();

private:
	QPointer<QDialog> m_pCurDlg;
};

#endif // BACKGROUNDDLG_H
