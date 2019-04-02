#ifndef _DISCUSSMODELDEF_H_
#define _DISCUSSMODELDEF_H_

#include <QStandardItemModel>
#include <QMap>
#include <QStringList>
#include <QSortFilterProxyModel>
#include <QPixmap>

class ModelManager;
class DiscussItem;
class DiscussProxyModel;
class DiscussGroupItem;

//////////////////////////////////////////////////////////////////////////
// class DiscussModel
class DiscussModel : public QStandardItemModel
{
	Q_OBJECT

public:
	static QString createdGroupId();
	static QString addedGroupId();

	DiscussModel(ModelManager *modelManager);
	~DiscussModel();

	QString discussVersion(const QString &id);
	void setDiscussVersion(const QString &id, const QString &version);

	QPixmap discussLogo(const QString &id);
	void setDiscussLogo(const QString &id, const QPixmap &logo);

	QStringList discussLogoIds(const QString &id);
	void setDiscussLogoIds(const QString &id, const QStringList &logoIds);

	void setDiscussList(const QStringList &ids, const QStringList &names, const QStringList &creators, const QStringList &times);

	void setInfo(const QString &id, const QString &name, const QString &creator, const QString &time);

	void addDiscuss(const QString &id, const QString &name, const QString &creator, const QString &time);
	
	bool delDiscuss(const QString &id);

	void release();

	QStringList allDiscussIds() const;

	DiscussItem *getDiscuss(const QString &id) const;

	bool containsDiscuss(const QString &id) const;

	DiscussProxyModel *proxyModel() const;

	DiscussItem *nodeFromProxyIndex(const QModelIndex &proxyIndex);

	DiscussGroupItem *createdGroupItem() const;
	DiscussGroupItem *addedGroupItem() const;

	static bool discussCmpLessThan(const DiscussItem *left, const DiscussItem *right);

	static QByteArray makeDiscussMemberData(const QStringList &memberIds, const QStringList &memberNames, 
		const QStringList &addedIds, const QStringList &cardNames);
	static bool parseDiscussMemberData(const QByteArray &rawData, QStringList &memberIds, QStringList &memberNames,
		QStringList &addedIds, QStringList &cardNames);

Q_SIGNALS:
	void discussInfoChanged(const QString &id);
	void discussDeleted(const QString &id);
	void discussAdded(const QString &id);

private:
	void appendDiscuss(DiscussItem *item);

private:
	ModelManager                *m_pModelManager;
	QMap<QString, DiscussItem *> m_discussItems; // <discuss id = discuss item>
	DiscussProxyModel           *m_pProxyModel;
	DiscussGroupItem            *m_groupCreated;
	DiscussGroupItem            *m_groupAdded;
	QMap<QString, QString>       m_discussVersions;
};

//////////////////////////////////////////////////////////////////////////
// class DiscussProxyModel
class DiscussProxyModel : public QSortFilterProxyModel
{
	Q_OBJECT

public:
	explicit DiscussProxyModel(QObject *parent = 0);

protected:
	bool lessThan(const QModelIndex &left, const QModelIndex &right) const;
};

#endif //_DISCUSSMODELDEF_H_
