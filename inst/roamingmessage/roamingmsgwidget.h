#ifndef ROAMINGMSGWIDGET_H
#define ROAMINGMSGWIDGET_H

#include <QWidget>
#include <QDate>
#include <QList>
#include "bean/bean.h"
#include "bean/MessageBody.h"

namespace Ui {class RoamingMsgWidget;};
class CMessage4Js;

class RoamingMsgWidget : public QWidget
{
	Q_OBJECT

public:
	enum PageDirection
	{
		PagePrev,
		PageNext
	};

public:
	RoamingMsgWidget(QWidget *parent = 0);
	~RoamingMsgWidget();

	bool init(const QString &id, bean::MessageType type);
	void setSkin();

private Q_SLOTS:
	void on_pBtnBegin_clicked();
	void on_pBtnPrevious_clicked();
	void on_pBtnNext_clicked();
	void on_pBtnEnd_clicked();
	void changeCurPage();
	void onComboBoxDateActivated(int index);
	void on_tBtnSearch_clicked();
	void on_tBtnRefresh_clicked();
	void on_tBtnSearchBar_toggled(bool checked);

	void gotChatRoamingMessageOK(const QString &uid, int pageSize, int currentPage, int totalPage,
		const bean::MessageBodyList &messages);
	void gotChatRoamingMessageFailed(const QString &uid);

	void gotGroupRoamingMessageOK(const QString &groupId, int pageSize, int currentPage, int totalPage,
		const bean::MessageBodyList &messages);
	void gotGroupRoamingMessageFailed(const QString &groupId);

	void gotDiscussRoamingMessageOK(const QString &discussId, int pageSize, int currentPage, int totalPage,
		const bean::MessageBodyList &messages);
	void gotDiscussRoamingMessageFailed(const QString &discussId);

private:
	void initUI();
	void startOperation();
	void endOperation();
	void configBottomBar(int curPage, int maxPage);
	void resetRoaming();
	void onGotMessageOK(int pageSize, int currentPage, int totalPage, const bean::MessageBodyList &messages);
	void onGotMessageFailed();

private:
	Ui::RoamingMsgWidget          *ui;
	CMessage4Js                   *m_pMessage4js;
	PageDirection                  m_pageDirection;
	QString                        m_qsId;
	bean::MessageType              m_eType;
	int                            m_nMaxPage;
	int                            m_nCurPage;
	QDate                          m_dateFrom;
	QDate                          m_dateTo;
	QString                        m_beginDate;
	QString                        m_endDate;
	bool                           m_inited;
};

#endif // ROAMINGMSGWIDGET_H
