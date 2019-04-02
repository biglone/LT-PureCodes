#ifndef CREATEDISCUSSDIALOG_H
#define CREATEDISCUSSDIALOG_H

#include "framelessdialog.h"

namespace Ui {class CreateDiscussDialog;};

class QStandardItemModel;
class QCompleter;
class QTreeView;
class QModelIndex;

class CreateDiscussDialog : public FramelessDialog
{
	Q_OBJECT
public:
	enum ActionType
	{
		Type_Create,
		Type_Add,
		Type_CreateInterphone
	};

public:
	CreateDiscussDialog(ActionType type = Type_Create, const QString &discussName = "", 
		const QStringList &baseUids = QStringList(), const QStringList &preAddUids = QStringList(), 
		bool modifyState = false, QWidget *parent = 0);
	~CreateDiscussDialog();

	void setDiscussId(const QString &id);
	QString discussId() const;

signals:
	void createDiscuss(const QString &name, const QStringList &uids);
	void addMembers(const QString &discussId, const QStringList &uids);
	void createInterphone(const QString &name, const QStringList &uids);
	void openDiscuss(const QString &discussId);

public slots:
	void setSkin();

private slots:
	void closeDialog();
	void addDiscussMembers();
	void onSelectionChanged();
	void onMaximizeStateChanged(bool isMaximized);
	void nameTextChanged(const QString &text);
	void completerActivated(const QModelIndex &index);
	void onDiscussListChanged();

private:
	void initUI(bool modifyState);
	void initSignals();
	void initNameCompleter();
	void addNameCompleterModel();

private:
	Ui::CreateDiscussDialog *ui;
	ActionType  m_type;
	QStringList m_lstBaseUids;
	QString     m_discussId;
	
	// name completer
	QCompleter         *m_completer;
	QStandardItemModel *m_completerModel;
	QTreeView          *m_completerView;
};

#endif // CREATEDISCUSSDIALOG_H
