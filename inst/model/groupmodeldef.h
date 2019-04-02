#ifndef GROUPMODELDEF_H
#define GROUPMODELDEF_H

#include <QStandardItemModel>
#include <QMap>
#include <QStringList>

class ModelManager;
class MucGroupItem;
class CSortFilterProxyModel;

class GroupModel : public QStandardItemModel
{
	Q_OBJECT

public:
	GroupModel(ModelManager *modelManager);
	~GroupModel();

	void setGroup(const QStringList &ids, const QStringList &names, const QList<int> &indice, 
		const QStringList &logoPathes, const QStringList &annts);

	void setGroupLogo(const QString &id, const QImage &logo);

	bool delGroup(const QString &id);
	
	void setDesc(const QString &id, const QString &desc);

	void setGroupVersion(const QString &id, const QString &version);
	QString groupVersion(const QString &id) const;
	
	void release();

	QStringList allGroupIds() const;

	MucGroupItem *getGroup(const QString &id) const;

	bool containsGroup(const QString &id) const;

	CSortFilterProxyModel *proxyModel() const;

	MucGroupItem *nodeFromProxyIndex(const QModelIndex &proxyIndex);

	static QByteArray makeGroupMemberData(const QString& desc, const QStringList &memberIds,
		const QStringList &memberNames, const QList<int> &indice, const QStringList &cardNames);
	static bool parseGroupMemberData(const QByteArray &rawData, QString &desc, QStringList &memberIds, 
		QStringList &memberNames, QList<int> &indice, QStringList &cardNames);

private:
	void appendGroup(MucGroupItem *group);

private:
	ModelManager                 *m_pModelManager;
	QMap<QString, MucGroupItem *> m_groupItems;    // <group id = group item>
	QMap<QString, QString>        m_groupVersions; // <group id = group version>
	CSortFilterProxyModel        *m_pProxyModel;
};

#endif // GROUPMODELDEF_H
