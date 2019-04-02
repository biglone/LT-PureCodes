#ifndef MSGMULTISENDDLG_H
#define MSGMULTISENDDLG_H

#include "chatbasedialog.h"
#include <QStringList>
#include <QPixmap>
#include "MessagePannel.h"

class GroupItemListModel;
namespace Ui {class MsgMultiSendDlg;};

class MsgMultiSendDlg : public ChatBaseDialog, public MessageSendDelegate
{
	Q_OBJECT

public:
	MsgMultiSendDlg(const QStringList &members, const QString &id = QString(), QWidget *parent = 0);
	~MsgMultiSendDlg();

	QString id() const;
	bool hasSameMembers(const QStringList &members) const;
	static bool isSameMember(const QStringList &left, const QStringList &right);
	void checkToAddMember();

public: // from MessagePannel
	virtual void sendMessage(const bean::MessageBody &msgBody);

public slots:
	virtual void setSkin();
	void slot_screenshot_ok(const QString &imagePath);
	void slot_screenshot_cancel();

	virtual void insertMimeData(const QMimeData *source);

Q_SIGNALS:
	void doScreenshot();
	void cleanup();
	void chat(const QString &uid);
	void sendMail(const QString &uid);
	void viewMaterial(const QString &uid);
	void memberChanged(const QString &id, const QString &name, const QStringList &members);
	void addFriendRequest(const QString &uid, const QString &name);
	void removeMultiSend(const QString &id);

private: // functions from ChatBaseDialog
	virtual void setMaximizeState(bool maximizeState);
	virtual bool isExpanded() const;
	virtual int unExpandedWidth() const;
	virtual void onUnionStateChanged();
	virtual void focusToEdit();

private Q_SLOTS:
	void onMaximizeStateChanged(bool isMaximized);

	void setMemberCount();

	void on_btnMember_clicked();

	void on_btnLeave_clicked();

	void onMemberChanged();

	void doCheckToAddMember();

private:
	void initUI();
	void initMemberModel();
	void updateIcon();
	void updateName();
	QPixmap generateIcon() const;
	QString generateName() const;

private:
	Ui::MsgMultiSendDlg   *ui;
	QString               m_id;
	QStringList           m_members;
	GroupItemListModel    *m_memberModel;
};

#endif // MSGMULTISENDDLG_H
