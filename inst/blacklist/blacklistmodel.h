#ifndef BLACKLISTMODEL_H
#define BLACKLISTMODEL_H

#include <QStandardItemModel>
#include <QStringList>

class BlackListModel : public QStandardItemModel
{
	Q_OBJECT

public:
	BlackListModel(QObject *parent = 0);
	~BlackListModel();

	void setBlackList(const QStringList &ids);
	void release();
	QStringList allIds() const;

Q_SIGNALS:
	void blackListChanged();

private:
	QStringList m_ids;
};

#endif // BLACKLISTMODEL_H
