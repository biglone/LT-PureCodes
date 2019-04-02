#include "webview.h"
#include <QWebPage>
#include <QWebFrame>
#include <QWebInspector>
#include <QWebSettings>
#include <QKeyEvent>
#include <QMenu>
#include <QWebElement>
#include <QFileInfo>
#include <QDebug>
#include <QDragMoveEvent>
#include <QDrag>
#include <QApplication>
#include <QCursor>
#include <QMimeData>
#include <QtWin>
#include "PmApp.h"
#include "logger.h"

const char *kMessageImageMimeType = "application/pm.message.image.path";

//////////////////////////////////////////////////////////////////////////
// MEMBER FUNCTIONS OF CLASS CWebView
CWebView::CWebView(QWidget *parent)
	: QWebView(parent), m_pInspector(0), m_menuDelegate(0), m_imageDragDelegate(0)
{
	CWebPage *webPage = new CWebPage(this);
	setPage(webPage);

	QWebSettings* pWebSettings = this->settings();
	pWebSettings->setAttribute(QWebSettings::PrivateBrowsingEnabled, true);
	pWebSettings->setAttribute(QWebSettings::DeveloperExtrasEnabled, true);
	this->setAcceptDrops(false);
	this->setContextMenuPolicy(Qt::CustomContextMenu);
	this->setRenderHints(QPainter::SmoothPixmapTransform|QPainter::TextAntialiasing);
	this->page()->mainFrame()->setScrollBarPolicy(Qt::Horizontal, Qt::ScrollBarAlwaysOff);

	connect(this, SIGNAL(customContextMenuRequested(QPoint)), SLOT(slot_customContextMenuRequested(QPoint)));
}

CWebView::~CWebView()
{
	if (m_pInspector)
		m_pInspector->deleteLater();
}

void CWebView::setMenuDelegate(WebViewMenuDelegate *menuDelegate)
{
	m_menuDelegate = menuDelegate;
}

void CWebView::setImageDragDelegate(WebViewImageDragDelegate *imageDragDelegate)
{
	m_imageDragDelegate = imageDragDelegate;
}

void CWebView::keyPressEvent(QKeyEvent *e)
{
	if((e->key() == Qt::Key_C)
		&& (e->modifiers().testFlag(Qt::ControlModifier)))
	{
		triggerPageAction(QWebPage::Copy, true);
	}
	else if ((e->key() == Qt::Key_A)
		&& (e->modifiers().testFlag(Qt::ControlModifier)))
	{
		triggerPageAction(QWebPage::SelectAll);
	}
	else if (e->modifiers() == (Qt::ShiftModifier | Qt::ControlModifier) && e->key() == Qt::Key_Q)
	{
		if (!m_pInspector)
		{
			m_pInspector = new QWebInspector();
			m_pInspector->setAttribute(Qt::WA_DeleteOnClose, false);
			m_pInspector->setPage(page());
		}
		m_pInspector->show();
		m_pInspector->activateWindow();
	}
	else
	{
		QWebView::keyPressEvent(e);
	}
}

void CWebView::mousePressEvent(QMouseEvent *event)
{
	if (event->button() == Qt::LeftButton)
		m_dragStartPosition = event->pos();

	QWebView::mousePressEvent(event);
}

void CWebView::mouseMoveEvent(QMouseEvent *event)
{
	do {
		if (!(event->buttons() & Qt::LeftButton))
			break;

		if ((event->pos() - m_dragStartPosition).manhattanLength() < 2/*QApplication::startDragDistance()*/)
			break;

		QWebHitTestResult testResult = this->page()->mainFrame()->hitTestContent(m_dragStartPosition);
		if (testResult.isNull())
			break;

		QWebElement webElement = testResult.element();
		QString tagName = webElement.tagName();
		if (0 == tagName.compare("IMG", Qt::CaseInsensitive))
		{
			QStringList attNames = webElement.attributeNames();
			QString srcName;
			foreach (QString attName, attNames)
			{
				if (0 == attName.compare("ORIGSRC", Qt::CaseInsensitive))
				{
					srcName = attName;
					break;
				}
				else if (0 == attName.compare("SRC", Qt::CaseInsensitive))
				{
					srcName = attName;
					break;
				}
			}

			QString imagePath;
			if (!srcName.isEmpty())
				imagePath = webElement.attribute(srcName, "");

			if (!imagePath.isEmpty())
			{
				QUrl url = QUrl::fromPercentEncoding(imagePath.toUtf8());
				if (!url.isLocalFile())
					break;

				if (m_imageDragDelegate)
				{
					if (!m_imageDragDelegate->canDragImage(webElement))
						return;
				}

				imagePath = url.toLocalFile();
				QMimeData *mimeData = new QMimeData();
				mimeData->setUrls(QList<QUrl>() << url);
				mimeData->setData(kMessageImageMimeType, imagePath.toUtf8());
				
				QDrag *drag = new QDrag(this);
				drag->setMimeData(mimeData);
				QPixmap dragPixmap(":/images/Icon_130.png");
				drag->setPixmap(dragPixmap);
				drag->setHotSpot(QPoint(-26, -14));

				Qt::DropAction dropAction = drag->exec(Qt::CopyAction);
				qDebug() << "web view drop action finished: " << dropAction;
				return;
			}
		}
	} while(0);
	
	QWebView::mouseMoveEvent(event);
}

void CWebView::slot_customContextMenuRequested(const QPoint &pos)
{
	QWebHitTestResult testResult = this->page()->mainFrame()->hitTestContent(pos);
	if (testResult.isNull())
	{
		return;
	}

	QMenu* menu = createMenu();
	if (menu)
	{
		if (m_menuDelegate)
		{
			QWebElement element = testResult.element();
			m_menuDelegate->addMenuAction(menu, element);
		}

		menu->exec(this->mapToGlobal(pos));
		delete menu;
		menu = 0;
	}
}

void CWebView::slot_copy_action_trigger()
{
	triggerPageAction(QWebPage::Copy);
}

void CWebView::slot_selectall_action_trigger()
{
	triggerPageAction(QWebPage::SelectAll);
}

QMenu* CWebView::createMenu()
{
	QMenu* menu = new QMenu();
	QAction* act = menu->addAction(tr("Copy"), this, SLOT(slot_copy_action_trigger()));
	menu->addAction(tr("Select all"), this, SLOT(slot_selectall_action_trigger()));

	if (selectedText().isEmpty())
	{
		act->setEnabled(false);
	}
	else
	{
		act->setEnabled(true);
	}

	return menu;
}

//////////////////////////////////////////////////////////////////////////
// MEMBER FUNCTIONS OF CLASS CWebPage
CWebPage::CWebPage(QObject *parent /*= 0*/)
: QWebPage(parent)
{

}

bool CWebPage::shouldInterruptJavaScript()
{
	qPmApp->getLogger()->debug(QString::fromLatin1("web view rose interrupt javescript error"));
	qDebug() << Q_FUNC_INFO << "rose interrupt javescript error";
	return false;
}