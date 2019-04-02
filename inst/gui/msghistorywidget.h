#ifndef MSGHISTORYWIDGET_H
#define MSGHISTORYWIDGET_H

#include <QList>
#include "bean/MessageBody.h"
#include <QWidget>
#include <QDate>
#include "webview.h"

namespace DB { class MessageDB; }
namespace Ui {class MsgHistoryWidget;};
class CMessage4Js;

class MsgHistoryWidget : public QWidget, public WebViewImageDragDelegate, public WebViewMenuDelegate
{
	Q_OBJECT

	enum PageDirection
	{
		PagePrev,
		PageNext
	};

	enum HistoryState
	{
		StateHistory,
		StateSearch,
		StateContext
	};

public:
	MsgHistoryWidget(QWidget *parent = 0);
	~MsgHistoryWidget();

	void setShowTitle(bool show);
	bool isShowTitle() const;

	void setShowSearchButton(bool show);
	void setShowMsgManagerButton(bool show);

	void setShowSearchMsgSource(bool show);

	void hideSearchBar();

	bool init(const QString &id, bean::MessageType type);
	void search(const QString &id, bean::MessageType type, const QString &begindate, const QString &enddate, const QString &keyword);
	void setSkin();
	QString id() const { return m_qsId; }
	void setId(const QString &id) { m_qsId = id; }
	bean::MessageType msgType() const { return m_eType; }
	CMessage4Js *message4Js() const { return m_pMessage4js; }

public: // From WebViewImageDragDelegate
	bool canDragImage(const QWebElement &imageElement);

public: // From WebViewMenuDelegate
	void addMenuAction(QMenu *menu, const QWebElement &webElement);

public Q_SLOTS:
	void onSendSecretMessageRead(const QString &uid, const QString &stamp);
	void onRecvSecretMessageRead(const QString &uid, const QString &stamp);

Q_SIGNALS:
	void sendSecretMessageRead(const QString &stamp);
	void recvSecretMessageRead(const QString &stamp);

	void messageIdTypeChanged(int msgType, const QString &id);

private Q_SLOTS:
	void onMsgBrowserLoadFinished();

	void on_pBtnBegin_clicked();
	void on_pBtnPrevious_clicked();
	void on_pBtnNext_clicked();
	void on_pBtnEnd_clicked();
	void on_leditPage_returnPressed();

	void slot_removeOneFromDB(int nMsgID, int msgType);
	void slot_removeMoreFromDB(int msgType);

	void onGotMessages(qint64 seq, int curPage, int maxPage, const bean::MessageBodyList &msgs, int sum);
	void onGotContextMessages(qint64 seq, int curPage, int maxPage, const bean::MessageBodyList &msgs, int sum);
	void onMsgRemoved(qint64 seq);
	void onGotMessageOfMsgId(qint64 seq, int msgId, const bean::MessageBody &msg);

	void changeCurPage();

	void openMsgContext(const QString &msgId, const QString &strMsgType, const QString &otherId);
	void contextMsgBack();
	void on_tBtnSearchBar_toggled(bool checked);
	void on_tBtnSearch_clicked();
	void onComboBoxDateActivated(int index);
	void on_tBtnSearchBack_clicked();
	void on_tBtnMsgManager_clicked();
	void on_lineEditWord_returnPressed();

	void onCopyImage();
	void onSaveImage();
	void onCopyMessage();
	void onRemoveOne();
	void onRemoveMore();

private:
	void initUI();
	void checkSecretState(const bean::MessageBodyList &msgs);
	void switchHistoryState(HistoryState state);
	void resetHistory();
	void configBottomBar(int curPage, int maxPage);
	void configContextLabel(bean::MessageType msgType, const QString &id);
	void startOperation();
	void endOperation();
	void doMessageCopy(const bean::MessageBody &msg);

private:
	Ui::MsgHistoryWidget*          ui;
	CMessage4Js                   *m_pMessage4js;
	PageDirection                  m_pageDirection;
	qint64                         m_nSearchId;
	qint64                         m_nDeleteId;
	HistoryState                   m_historyState;
	bool                           m_titleVisibility;
	qint64                         m_nCopyId;

	//////////////////////////////////////////////////////////////////////////
	// history part
	QString                        m_qsId;
	bean::MessageType              m_eType;
	int                            m_nMaxPage;
	int                            m_nCurPage;
	//////////////////////////////////////////////////////////////////////////
	
	//////////////////////////////////////////////////////////////////////////
	// search part
	CMessage4Js                   *m_pSearchMessage4js;
	QString                        m_searchId;
	bean::MessageType              m_searchType;
	QString                        m_searchBeginDate;
	QString                        m_searchEndDate;
	QString                        m_searchKeyword;
	int                            m_nSearchMaxPage;
	int                            m_nSearchCurPage;
	QDate                          m_dateFrom;
	QDate                          m_dateTo;
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	// context part
	QString                        m_contextMsgId;
	QString                        m_contextId;
	bean::MessageType              m_contextType;
	int                            m_nContextMaxPage;
	int                            m_nContextCurPage;
	//////////////////////////////////////////////////////////////////////////

	QAction                       *m_imageCopyAction;
	QAction                       *m_imageSaveAction;
	QAction                       *m_messageCopyAction;
	QAction                       *m_removeOneAction;
	QAction                       *m_removeMoreAction;
};

#endif // MSGHISTORYWIDGET_H
