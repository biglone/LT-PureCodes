#ifndef APPMANAGEMODEL_H
#define APPMANAGEMODEL_H

#include <QStandardItemModel>
#include <QByteArray>
#include <QIcon>

class AppManageModel : public QStandardItemModel
{
	Q_OBJECT

public:
	static const int kPathRole       = Qt::UserRole + 1;
	static const int kActualDataRole = Qt::UserRole + 2;

	static const int kRowCount       = 3;
	static const int kColCount       = 3;

	static const char *kMimeType;

	enum ActualData
	{
		Data, // this is a data item
		Add,  // this is a add sign
		Del   // this is a del sign
	};

public:
	AppManageModel(QObject *parent);
	~AppManageModel();

	static QIcon appSmallIcon(const QString &appPath);
	static QIcon appBigIcon(const QString &appPath);

	void setAppItem(int index, const QString &name, const QString &path);
	void setAddItem(int index);
	void setDelItem();

	static bool mimeDataToIndex(const QByteArray &data, int &row, int &col);
	
public: // drag & drop
	Qt::DropActions supportedDropActions() const;
	Qt::ItemFlags flags(const QModelIndex &index) const;
	QStringList mimeTypes() const;
	QMimeData *mimeData(const QModelIndexList &indexes) const;
	bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent);

Q_SIGNALS:
	void appItemsChanged();
};

#endif // APPMANAGEMODEL_H
