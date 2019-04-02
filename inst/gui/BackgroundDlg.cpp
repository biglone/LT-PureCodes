#include <QDebug>
#include "logindlg.h"
#include "pscdlg.h"
#include "updatedialog.h"
#include "pmessagebox.h"
#include "settings/GlobalSettings.h"
#include "filetransfer/attachtransfermgr.h"

#include "buddymgr.h"

#include "PmApp.h"
#include "BackgroundDlg.h"

BackgroundDlg::BackgroundDlg(QWidget *parent)
	: QWidget(parent), m_pCurDlg(0)
{
	createLoginDlg();
}

BackgroundDlg::~BackgroundDlg()
{
}

void BackgroundDlg::loginWithId(const QString &uid)
{
	CLoginDlg *pDlg = static_cast<CLoginDlg *>(m_pCurDlg.data());
	if (!pDlg)
		return;

	pDlg->loginWithId(uid);
}

void BackgroundDlg::setVisible(bool visible)
{
	if (m_pCurDlg)
	{
		m_pCurDlg->setVisible(visible);
		if (visible)
		{
			m_pCurDlg->raise();
			m_pCurDlg->activateWindow();
		}

		if (m_pCurDlg == CPscDlg::instance())
		{
			CPscDlg *pscDlg = static_cast<CPscDlg *>(m_pCurDlg.data());
			pscDlg->dockShow();
		}
	}
	QWidget::setVisible(false);
}

void BackgroundDlg::onLogined()
{
	if (m_pCurDlg && m_pCurDlg != CPscDlg::instance())
	{
		m_pCurDlg->hide();
		m_pCurDlg->deleteLater();
		m_pCurDlg = 0;
	}

	if (!m_pCurDlg)
	{
		createPscDlg();
	}
}

void BackgroundDlg::showUpdateDlg(const QString &toVer, const QString &toMsg, const QString &downloadUrl)
{
	if (m_pCurDlg)
	{
		m_pCurDlg->hide();
		m_pCurDlg->deleteLater();
		m_pCurDlg = 0;
	}
	qPmApp->UninitSystemTray();

	// close all dialog, but not self
	foreach (QWidget *widget, QApplication::topLevelWidgets())
	{
		if (widget != this)
		{
			widget->close();
		}
	}

	CUpdateDialog *pUpdateDialog = CUpdateDialog::instance();
	pUpdateDialog->setLastestVersion(toVer, toMsg, downloadUrl);
}

void BackgroundDlg::onCompanyLoginFailed(const QString &desc)
{
	if (m_pCurDlg && m_pCurDlg == CPscDlg::instance())
	{
		CPscDlg::instance()->onCompanyLoginFailed(desc);
	}
}

void BackgroundDlg::createLoginDlg()
{
	CLoginDlg *pLoginDlg = new CLoginDlg(this);
	pLoginDlg->setWindowFlags(pLoginDlg->windowFlags() | Qt::WindowStaysOnTopHint);
	m_pCurDlg = pLoginDlg;
}

void BackgroundDlg::createPscDlg()
{
	CPscDlg *pPscDlg = new CPscDlg(this);
	pPscDlg->init();
	pPscDlg->show();
	pPscDlg->raise();
	pPscDlg->activateWindow();
	m_pCurDlg = pPscDlg;
}

