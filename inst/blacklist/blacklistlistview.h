#ifndef BLACKLISTLISTVIEW_H
#define BLACKLISTLISTVIEW_H

#include <QListView>

class BlackListModel;

class BlackListListView : public QListView
{
	Q_OBJECT

public:
	BlackListListView(QWidget *parent = 0);
	~BlackListListView();

	void setBlackListModel(BlackListModel *model);
	BlackListModel *blackListModel() const;

Q_SIGNALS:
	void removeBlack(const QString &id);
	void viewMaterial(const QString &id);

protected:
	void paintEvent(QPaintEvent *event);

private slots:
	void onBlackListChanged();
	void onDoubleClicked(const QModelIndex &index);

private:
	BlackListModel *m_blackListModel;
};

#endif // BLACKLISTLISTVIEW_H
