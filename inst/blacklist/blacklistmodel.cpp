#include "blacklistmodel.h"

BlackListModel::BlackListModel(QObject *parent /*= 0*/)
	: QStandardItemModel(parent)
{

}

BlackListModel::~BlackListModel()
{

}

void BlackListModel::setBlackList(const QStringList &ids)
{
	release();

	m_ids = ids;
	foreach (QString id, ids)
	{
		QStandardItem *item = new QStandardItem();
		item->setData(id);
		appendRow(item);
	}

	emit blackListChanged();
}

void BlackListModel::release()
{
	clear();
	m_ids.clear();
}

QStringList BlackListModel::allIds() const
{
	return m_ids;
}