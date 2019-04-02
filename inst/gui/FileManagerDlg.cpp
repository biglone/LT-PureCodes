#include <QStringListModel>
#include <QAbstractTableModel>
#include <QSortFilterProxyModel>
#include <QItemSelectionModel>
#include <QDate>
#include <QFileInfo>
#include <QHeaderView>
#include <QDesktopServices>
#include <QUrl>
#include <QItemDelegate>
#include <QPixmap>
#include <QPixmapCache>
#include <QProcess>
#include <QDir>
#include <QList>
#include <QMovie>
#include <QPainter>

#include <QDebug>

#include "pmessagebox.h"
#include "commonlistitemdelegate.h"

#include "removefileoptiondialog.h"

#include "clickablelabel.h"

#include "pmessagebox.h"

#include "styletoolbutton.h"

#include "util/FileUtil.h"

#include "bean/attachitem.h"

#include "PmApp.h"
#include "db/MessageDBStore.h"

#include "common/datetime.h"

#include "util/ExplorerUtil.h"

#include "FileManagerDlg.h"
#include "ui_FileManagerDlg.h"

const int SearchLastOneWeek      = 0;
const int SearchLastOneMonth     = 1;
const int SearchLastThreeMonthes = 2;
const int SearchLastOneYear      = 3;
const int SearchAll              = 4;

//////////////////////////////////////////////////////////////////////////
class OperatorPannel : public QWidget
{
	Q_OBJECT
public:
	OperatorPannel(const QString &uuid, QWidget *parent = 0);
	~OperatorPannel();

Q_SIGNALS:
	void openFile(const QString &uuid);
	void openDir(const QString &uuid);
	void removeFile(const QString &uuid);

private slots:
	void onOpenfileClicked();
	void onOpendirClicked();
	void onRemovefileClicked();

private:
	QString  m_uuid;
};

OperatorPannel::OperatorPannel(const QString &uuid, QWidget *parent /*= 0*/)
: QWidget(parent), m_uuid(uuid)
{
	StyleToolButton *openfile = new StyleToolButton(this);
	openfile->setObjectName(QString::fromLatin1("openfile"));
	StyleToolButton::Info info;
	info.urlNormal = ":/images/fm_openfile_normal.png";
	info.urlHover = ":/images/fm_openfile_hover.png";
	info.urlPressed = ":/images/fm_openfile_pressed.png";
	info.tooltip = tr("Open");
	openfile->setInfo(info);
	
	StyleToolButton *opendir = new StyleToolButton(this);
	opendir->setObjectName(QString::fromLatin1("opendir"));
	info.urlNormal = ":/images/fm_opencatalog_normal.png";
	info.urlHover = ":/images/fm_opencatalog_hover.png";
	info.urlPressed = ":/images/fm_opencatalog_pressed.png";
	info.tooltip = tr("Open Dir");
	opendir->setInfo(info);

	StyleToolButton *removefile = 0;
	removefile = new StyleToolButton(this);
	removefile->setObjectName(QString::fromLatin1("removefile"));
	info.urlNormal = ":/images/fm_delfile_normal.png";
	info.urlHover = ":/images/fm_delfile_hover.png";
	info.urlPressed = ":/images/fm_delfile_pressed.png";
	info.tooltip = tr("Delete File");
	removefile->setInfo(info);

	QSpacerItem *horizontalSpacer_1 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);;
	QSpacerItem *horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

	QHBoxLayout *layout = new QHBoxLayout(this);
	layout->addItem(horizontalSpacer_1);
	layout->addWidget(openfile);
	layout->addWidget(opendir);
	layout->addWidget(removefile);
	layout->addItem(horizontalSpacer_2);

	connect(openfile, SIGNAL(clicked()), this, SLOT(onOpenfileClicked()), Qt::DirectConnection);
	connect(opendir, SIGNAL(clicked()), this, SLOT(onOpendirClicked()), Qt::DirectConnection);
	connect(removefile, SIGNAL(clicked()), this, SLOT(onRemovefileClicked()), Qt::DirectConnection);
}

OperatorPannel::~OperatorPannel()
{
}

void OperatorPannel::onOpenfileClicked()
{
	emit openFile(m_uuid);
}

void OperatorPannel::onOpendirClicked()
{
	emit openDir(m_uuid);
}

void OperatorPannel::onRemovefileClicked()
{
	emit removeFile(m_uuid);
}

//////////////////////////////////////////////////////////////////////////
class NameSizePannel : public QWidget
{
	Q_OBJECT
public:
	NameSizePannel(QWidget *parent = 0) : QWidget(parent) {}

	void setName(const QString &name) {m_name = name;}
	void setSize(const QString &size) {m_size = size;}

protected:
	void paintEvent(QPaintEvent *e);

private:
	QString m_name;
	QString m_size;
};

void NameSizePannel::paintEvent(QPaintEvent * /*e*/)
{
	QPainter painter(this);
	painter.save();
	QRect rect = this->rect();
	rect.setHeight(rect.height()/2);
	QFont font = this->font();
	QFontMetrics fm(font);
	QString name = fm.elidedText(m_name, Qt::ElideRight, rect.width());
	painter.drawText(rect, Qt::AlignLeft|Qt::AlignBottom|Qt::TextSingleLine, name);

	rect.moveTo(rect.x(), rect.y()+rect.height());
	painter.setPen(QColor(128, 128, 128));
	font.setPointSizeF(9.0f);
	painter.setFont(font);
	painter.drawText(rect, Qt::AlignLeft|Qt::AlignTop|Qt::TextSingleLine, m_size);
	painter.restore();
}

//////////////////////////////////////////////////////////////////////////
class IconItem : public QTableWidgetItem
{
	enum 
	{
		IconItemRole = Qt::UserRole + 1024
	};

public:
	explicit IconItem(const QString &format, bool isDir = false);
	virtual bool operator< (const QTableWidgetItem &other) const;
};

IconItem::IconItem(const QString &format, bool isDir /*= false*/)
: QTableWidgetItem()
{
	QStringList suffixes;
	suffixes << "doc" << "docx" << "xls" << "xlsx" << "ppt" << "pptx" << "gif" 
		     << "jpeg" << "jpg" << "png" << "bmp" << "pdf" << "swf" << "zip" << "rar" << "txt"
			 << "avi" << "wma" << "rmvb" << "rm" << "mp4" << "3gp";
	if (suffixes.contains(format, Qt::CaseInsensitive))
	{
		QString fileName = QString(":/html/images/file_formats/%1.png").arg(format.toLower());
		setIcon(QIcon(fileName));
	}
	else if (isDir)
	{
		QString fileName = QString(":/html/images/file_formats/folder.png");
		setIcon(QIcon(fileName));
	}
	else
	{
		QString fileName = QString(":/html/images/file_formats/other.png");
		setIcon(QIcon(fileName));
	}
	setData(IconItemRole, format);
}

bool IconItem::operator<(const QTableWidgetItem &other) const
{
	return (this->data(IconItemRole).toString() < other.data(IconItemRole).toString());
}

//////////////////////////////////////////////////////////////////////////
class FileNameItem : public QTableWidgetItem
{
	enum 
	{
		FileNameItemRole = Qt::UserRole + 1025
	};

public:
	explicit FileNameItem(const QString &fileName);
	virtual bool operator< (const QTableWidgetItem &other) const;
};

FileNameItem::FileNameItem(const QString &fileName)
: QTableWidgetItem()
{
	setData(FileNameItemRole, fileName);
}

bool FileNameItem::operator<(const QTableWidgetItem &other) const
{
	return (this->data(FileNameItemRole).toString() < other.data(FileNameItemRole).toString());
}

//////////////////////////////////////////////////////////////////////////
FileManagerDlg *FileManagerDlg::s_instance = 0;

static int SearchThreadIndex = 1;

FileManagerDlg::FileManagerDlg(QWidget *parent)
	: FramelessDialog(parent)
{
	ui = new Ui::FileManagerDlg();
	ui->setupUi(this);

	setAttribute(Qt::WA_DeleteOnClose, true);

	setWindowIcon(qApp->windowIcon());
	setWindowTitle(ui->title->text());

	setMainLayout(ui->verticalLayoutMain);
	resize(904, 596);
	setMinimumSize(904, 596);
	setResizeable(false);

	initUI();

	initSignals();

	setSkin();
}

FileManagerDlg::~FileManagerDlg()
{
	s_instance = 0;

	delete ui;
}

FileManagerDlg * FileManagerDlg::instance()
{
	if (!s_instance)
	{
		s_instance = new FileManagerDlg;
	}
	return s_instance;
}

void FileManagerDlg::setSkin()
{
	// set background image
	QPixmap bgPixmap(":/theme/dialog_bgs/dialog_bg_3.png");
	FramelessDialog::BGSizes bgSizes = {0};
	bgSizes.borderwidth = 5;
	bgSizes.topBarHeight = 32;
	setBG(bgPixmap, bgSizes);

	// set title label style
	ui->title->setStyleSheet("QLabel {color: white; font-size: 12pt;}");

	// set list view style
	ui->listViewPages->setStyleSheet("QListView#listViewPages {background: rgb(232, 232, 232); border: none; padding-top: 20px;}");
	ui->tableWidget->setStyleSheet("QTableWidget#tableWidget {"
										"background: transparent;" 
										"border: none;"
										"border-bottom: 1px solid rgb(219, 219, 219);"
										// "selection-background-color: rgb(254, 245, 204);"
										// "padding-left: 15px;"
										// "padding-right: 15px;"
									"}"
									"QTableWidget#tableWidget::item {"
										"background: transparent;" 
										"border: none;"
										"border-bottom: 1px solid rgb(219, 219, 219);"
									"}"
									);
	ui->tableWidget->horizontalHeader()->setStyleSheet(
		"QHeaderView {"
		"background-color: transparent;"
		"border: none;"
		"border-bottom: 1px solid rgb(219, 219, 219);"
		"}"
		"QHeaderView::section {"
		"background-color: transparent;"
		"color: #666666;"
		"border: none;"
		"height: 32px;"
		"padding-left: 6px;"
		"padding-right: 6px;"
		"}"
		"QHeaderView::down-arrow {"
		"image: url(:/images/down_arrow.png);"
		"subcontrol-position: right center;"
		"subcontrol-origin: padding;"
		"right: 4px;"
		"}"
		"QHeaderView::up-arrow {"
		"image: url(:/images/up_arrow.png);"
		"subcontrol-position: right center;"
		"subcontrol-origin: padding;"
		"right: 4px;"
		"}"
		);

	// set pageTable
	ui->pageTable->setStyleSheet("QWidget#pageTable {"
		"background: transparent;" 
		"border: none;"
		"}");

	// load text
	ui->labelLoadText->setStyleSheet("font-size: 9pt; color: rgb(128, 128, 128);");

	// set stackedWidget
	ui->stackedWidget->setStyleSheet("QWidget#stackedWidget {background: transparent;}");
}

void FileManagerDlg::onListViewPagesActivated(const QModelIndex &index)
{
	Q_UNUSED(index);
	qDebug() << Q_FUNC_INFO << ui->listViewPages->currentIndex().row() << " " << index.row();
	
	currentPageChanged();
	ui->tableWidget->horizontalHeader()->setSortIndicator(3, Qt::DescendingOrder);
	ui->tableWidget->sortItems(3, Qt::DescendingOrder);
}

void FileManagerDlg::onOpenFile(const QString &uuid)
{
	if (!m_data.contains(uuid))
		return;

	QVariantMap vmap = m_data.value(uuid);
	int id = vmap["id"].toInt();

	QString filepath = savedFilePath(vmap);
	QFileInfo fi(filepath);
	
	if (!FileUtil::fileExists(filepath))
	{
		PMessageBox::information(this, tr("Can't find file"), tr("File\n\"%1\" does not exist, may be deleted or moved to other place, this item will be cleared").arg(fi.fileName()));
		
		OperatorPannel *pannel = qobject_cast<OperatorPannel *>(sender());
		if (pannel)
		{
			setCurrentPannel(pannel);
		}
		
		int msgType = vmap.value("msgtype").toInt();
		doRemove(uuid, id, msgType);
		return;
	}

	QUrl url = QUrl::fromLocalFile(filepath);
	bool ok = QDesktopServices::openUrl(url);
	if (!ok)
	{
		qDebug() << Q_FUNC_INFO << filepath << url << "open failed";

		PMessageBox::warning(this, tr("Error"), tr("No related program to open this file"));
	}
}

void FileManagerDlg::onOpenDir(const QString &uuid)
{
	if (!m_data.contains(uuid))
		return;

	QVariantMap vmap = m_data.value(uuid);
	int id = vmap["id"].toInt();

	QString filepath = savedFilePath(vmap);
	QFileInfo fi(filepath);
	QDir dir = fi.dir();

	if (filepath.isEmpty() || !dir.exists())
	{
		PMessageBox::information(this, tr("Can't find directory"), tr("Directory\n\"%1\" does not exist, may be deleted or moved to other place, this item will be cleared").arg(dir.absolutePath()));
		
		OperatorPannel *pannel = qobject_cast<OperatorPannel *>(sender());
		if (pannel)
		{
			setCurrentPannel(pannel);
		}
		
		int msgType = vmap.value("msgtype").toInt();
		doRemove(uuid, id, msgType);
		return;
	}

	if (!ExplorerUtil::selectFile(fi))
	{
		PMessageBox::warning(this, tr("Error"), tr("Open directory of this file failed"));
	}
}

void FileManagerDlg::onRemoveFile(const QString &uuid)
{
	if (!m_data.contains(uuid))
		return;

	RemoveFileOptionDialog dlg(this);
	if (dlg.exec() == QDialog::Rejected)
	{
		return;
	}

	QVariantMap vmap = m_data.value(uuid);
	int id = vmap["id"].toInt();
	QString filename = savedFilePath(vmap);
	if (dlg.isRemoveFile() && FileUtil::fileExists(filename))
	{
		QFileInfo fi(filename);
		if (fi.isDir())
		{
			QDir dir(filename);
			dir.removeRecursively();
		}
		else
		{
			QFile::remove(filename);
		}
	}

	OperatorPannel *pannel = qobject_cast<OperatorPannel *>(sender());
	if (pannel)
	{
		setCurrentPannel(pannel);
	}

	int msgType = vmap.value("msgtype").toInt();
	doRemove(uuid, id, msgType);
}

void FileManagerDlg::on_btnSearch_clicked()
{
	QString keyword = ui->lineEditWord->text();
	int dateIndex = ui->comboBoxDate->currentIndex();
	int dateSelect = ui->comboBoxDate->itemData(dateIndex).toInt();

	search(keyword, dateSelect);
}

void FileManagerDlg::initUI()
{
	ui->btnMaximize->setVisible(false);

	QMovie *movie = new QMovie(this);
	movie->setFileName(":/images/loading_image_small.gif");
	ui->labelLoad->setMovie(movie);

	movie->start();

	ui->comboBoxDate->addItem(tr("Last Week"), SearchLastOneWeek);
	ui->comboBoxDate->addItem(tr("Last Month"), SearchLastOneMonth);
	ui->comboBoxDate->addItem(tr("Last Three Months"), SearchLastThreeMonthes);
	ui->comboBoxDate->addItem(tr("Last Year"), SearchLastOneYear);
	ui->comboBoxDate->addItem(tr("All"), SearchAll);
	ui->comboBoxDate->setCurrentIndex(0);

	// page list view
	ui->listViewPages->setEditTriggers(QListView::NoEditTriggers);
	QStringListModel *pagesListModel = new QStringListModel(ui->listViewPages);
	pagesListModel->setStringList(QStringList() << tr("All Attaches") << tr("    Sent") << tr("    Received"));
	ui->listViewPages->setModel(pagesListModel);

	CommonListItemDelegate *pagesListDelegate = new CommonListItemDelegate(ui->listViewPages);
	ui->listViewPages->setItemDelegate(pagesListDelegate);

	ui->listViewPages->setCurrentIndex(pagesListModel->index(0));
	ui->listViewPages->selectionModel()->select(pagesListModel->index(0), QItemSelectionModel::ClearAndSelect);

	ui->tableWidget->setFocusPolicy(Qt::NoFocus);
	ui->tableWidget->setShowGrid(false);
	ui->tableWidget->setSelectionBehavior(QTableView::SelectRows);
	ui->tableWidget->setSelectionMode(QTableView::SingleSelection);
	ui->tableWidget->setIconSize(QSize(32,32));
	ui->tableWidget->setEditTriggers(QTableWidget::NoEditTriggers);
	ui->tableWidget->horizontalHeader()->setDefaultAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
	ui->tableWidget->horizontalHeader()->setStretchLastSection(true);
	ui->tableWidget->horizontalHeader()->setSortIndicatorShown(true);
	ui->tableWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	ui->tableWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

	QStringList column_names;
	column_names << tr("Type") << tr("File Name") << tr("Sender/Receiver") << tr("Time") << tr("Operations");
	ui->tableWidget->setColumnCount(column_names.count());
	ui->tableWidget->setHorizontalHeaderLabels(column_names);
	ui->tableWidget->verticalHeader()->setVisible(false);
	ui->tableWidget->horizontalHeader()->setMinimumSectionSize(58);
	ui->tableWidget->horizontalHeader()->setDefaultAlignment(Qt::AlignCenter);

	// icon
	ui->tableWidget->horizontalHeader()->resizeSection(0, 58);
	ui->tableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
	// name
	ui->tableWidget->horizontalHeader()->resizeSection(1, 220);
	ui->tableWidget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Fixed);
	// from
	ui->tableWidget->horizontalHeader()->resizeSection(2, 130);
	ui->tableWidget->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Fixed);
	// time
	ui->tableWidget->horizontalHeader()->resizeSection(3, 140);
	ui->tableWidget->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Fixed);
	// operator
	ui->tableWidget->horizontalHeader()->resizeSection(4, 100);
	ui->tableWidget->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Fixed);

	ui->lineEditWord->setPlaceholderText(tr("Key Word"));

	on_btnSearch_clicked();
}

void FileManagerDlg::initSignals()
{
	connect(ui->btnClose, SIGNAL(clicked()), this, SLOT(close()));
	connect(ui->btnMinimize, SIGNAL(clicked()), this, SLOT(showMinimized()));
	connect(ui->btnMaximize, SIGNAL(clicked()), this, SLOT(triggerMaximize()));

	connect(ui->lineEditWord, SIGNAL(editingFinished()), this, SLOT(on_btnSearch_clicked()));

	connect(ui->listViewPages, SIGNAL(clicked(QModelIndex)), this, SLOT(onListViewPagesActivated(QModelIndex)));

	connect(ui->tableWidget, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(onTableViewDoubleclicked(QModelIndex)));
	connect(ui->tableWidget, SIGNAL(clicked(QModelIndex)), this, SLOT(onTableViewClicked(QModelIndex)));

	connect(ui->tableWidget->horizontalHeader(), SIGNAL(sortIndicatorChanged(int, Qt::SortOrder)), this, SLOT(onTableWidgetSortIndicatorChanged(int, Qt::SortOrder)));
	connect(ui->tableWidget->horizontalHeader(), SIGNAL(sectionClicked(int)), this, SLOT(onTableWidgetSortByColumn(int)));

	MessageDBStore *messageDBStore = qPmApp->getMessageDBStore();
	connect(messageDBStore, SIGNAL(gotAttachments(qint64, int, int, QVariantList)), this, SLOT(onSearcherFinish(qint64, int, int, QVariantList)));
}

void FileManagerDlg::currentPageChanged()
{
	showLoadWidget(true);

	QApplication::processEvents();
	if (!s_instance)
		return;

	// all
	int method = -1;

	switch (ui->listViewPages->currentIndex().row())
	{
	case 1:
		{
			method = 0;
		}
		break;
	case 2:
		{
			method = 1;
		}
		break;
	}

	ui->tableWidget->clear();
	ui->tableWidget->setRowCount(0);

	QStringList column_names;
	column_names << tr("Type") << tr("File Name") << tr("Sender/Receiver") << tr("Time") << tr("Operations");
	ui->tableWidget->setColumnCount(column_names.count());
	ui->tableWidget->setHorizontalHeaderLabels(column_names);

	int r = 0;
	foreach (QString uuid, m_data.keys())
	{
		QVariantMap vmap = m_data.value(uuid);
		if (method != -1)
		{
			int mthd = vmap.value("method", -1).toInt();
			if (mthd == -1 || mthd != method)
			{
				continue;
			}
		}

		QFileInfo fi(savedFilePath(vmap));
		if (!FileUtil::fileExists(savedFilePath(vmap)))
			continue;

		// insert row
		ui->tableWidget->insertRow(r);
		ui->tableWidget->setRowHeight(r, 58);

		bool isDir = (vmap["fttype"] == bean::AttachItem::Type_Dir);

		// c: 0 icon
		{
			IconItem *pItem = new IconItem(fi.suffix(), isDir);
			pItem->setTextAlignment(Qt::AlignCenter);
			ui->tableWidget->setItem(r, 0, pItem);
		}

		// c: 1 name & size
		{
			QString name = savedFileName(vmap);
			QString fileSizeStr;
			int size = vmap.value("size").toInt();
			if(size < 1024){
				fileSizeStr = QString("%1 B").arg(size);
			}else if(size < 1024*1024){
				fileSizeStr = QString("%1 KB").arg(size/1024);
			}else if(size < 1024*1024*1024) {
				fileSizeStr = QString("%1 MB").arg(size/(1024*1024));
			}else {
				fileSizeStr = QString("%1 GB").arg(size/(1024*1024*1024));
			}

			NameSizePannel *nameSizePannel = new NameSizePannel();
			nameSizePannel->setName(name);
			nameSizePannel->setSize(fileSizeStr);
			QString fileNameAndSizeStr = QString("%1\n%2").arg(name).arg(fileSizeStr);
			nameSizePannel->setToolTip(fileNameAndSizeStr);

			FileNameItem *pItem = new FileNameItem(fileNameAndSizeStr);
			ui->tableWidget->setItem(r, 1, pItem);

			ui->tableWidget->setCellWidget(r, 1, nameSizePannel);
		}

		// c: 2 from
		{
			QString toPrefix;
			int mthd = vmap.value("method", -1).toInt();
			if (mthd == 0)
			{
				toPrefix = tr("To:");
			}
			else
			{
				toPrefix = tr("From:");
			}

			QTableWidgetItem *pItem = new QTableWidgetItem();
			QString name = vmap.value("uname").toString();
			name = toPrefix + name;
			pItem->setText(name);
			pItem->setToolTip(name);
			pItem->setTextAlignment(Qt::AlignCenter);
			QFont font = this->font();
			font.setPointSizeF(9.0f);
			pItem->setFont(font);
			pItem->setTextColor(QColor(128, 128, 128));
			ui->tableWidget->setItem(r, 2, pItem);
		}

		// c: 3 date
		{
			QTableWidgetItem *pItem = new QTableWidgetItem();
			QString date = vmap.value("date").toString();
			date = CDateTime::localDateTimeStringFromUtcString(date);
			pItem->setText(date);
			pItem->setToolTip(date);
			pItem->setTextAlignment(Qt::AlignCenter);
			QFont font = this->font();
			font.setPointSizeF(9.0f);
			pItem->setFont(font);
			pItem->setTextColor(QColor(128, 128, 128));
			ui->tableWidget->setItem(r, 3, pItem);
		}

		// c: 4 op
		{
			OperatorPannel *pW = new OperatorPannel(uuid);
			connect(pW, SIGNAL(openFile(QString)), this, SLOT(onOpenFile(QString)));
			connect(pW, SIGNAL(openDir(QString)), this, SLOT(onOpenDir(QString)));
			connect(pW, SIGNAL(removeFile(QString)), this, SLOT(onRemoveFile(QString)));

			ui->tableWidget->setCellWidget(r, 4, pW);
		}

		++r;

		QApplication::processEvents();
		if (!s_instance)
			return;
	}

	showLoadWidget(false);
}


void FileManagerDlg::search(const QString &keyword /*= ""*/, int dateSelect /*= -1*/)
{
	QString date = "";
	switch (dateSelect)
	{
	case SearchLastOneWeek:
		{
			date = CDateTime::currentDateTime().date().addDays(-7).toString(DATEFORMAT_DATE);
		}
		break;
	case SearchLastOneMonth:
		{
			date = CDateTime::currentDateTime().date().addMonths(-1).toString(DATEFORMAT_DATE);
		}
		break;
	case SearchLastThreeMonthes:
		{
			date = CDateTime::currentDateTime().date().addMonths(-3).toString(DATEFORMAT_DATE);
		}
		break;
	case SearchLastOneYear:
		{
			date = CDateTime::currentDateTime().date().addYears(-1).toString(DATEFORMAT_DATE);
		}
		break;
	}

	MessageDBStore *messageDBStore = qPmApp->getMessageDBStore();
	m_searchSeq = messageDBStore->getAttachments(1, INT_MAX, date, keyword); // get all

	showLoadWidget(true);
	ui->listViewPages->setEnabled(false);
	ui->btnSearch->setEnabled(false);
}

void FileManagerDlg::doRemove(const QString &uuid, int id, int msgType)
{
	if (msgType == bean::Message_Invalid)
		return;
	
	MessageDBStore *messageDBStore = qPmApp->getMessageDBStore();
	messageDBStore->removeAttachById((bean::MessageType)msgType, id);

	ui->tableWidget->removeRow(ui->tableWidget->currentRow());
	m_data.remove(uuid);
}

QString FileManagerDlg::savedFilePath(const QVariantMap &data) const
{
	QString filePath = data["filename"].toString();
	return filePath;
}

QString FileManagerDlg::savedFileName(const QVariantMap &data) const
{
	QFileInfo fi(savedFilePath(data));
	QString fileName = fi.fileName();
	if (fileName.isEmpty())
	{
		fileName = data["name"].toString();
	}
	return fileName;
}

void FileManagerDlg::setCurrentPannel(OperatorPannel *pannel)
{
	if (!pannel)
	{
		return;
	}

	for (int i = 0; i < ui->tableWidget->rowCount(); i++)
	{
		OperatorPannel *p = qobject_cast<OperatorPannel *>(ui->tableWidget->cellWidget(i, 4));
		if (p == pannel)
		{
			ui->tableWidget->setCurrentCell(i, 4);
			return;
		}
	}
}

void FileManagerDlg::showLoadWidget(bool show /*= true*/)
{
	ui->loadWidget->setVisible(show);
}

void FileManagerDlg::onTableViewDoubleclicked(const QModelIndex &index)
{
	Q_UNUSED(index);
}

void FileManagerDlg::onTableViewClicked(const QModelIndex &index)
{
	Q_UNUSED(index);
}

void FileManagerDlg::onTableWidgetSortIndicatorChanged(int logicalIndex, Qt::SortOrder order)
{
	Q_UNUSED(logicalIndex);
	Q_UNUSED(order);
}

void FileManagerDlg::onTableWidgetSortByColumn(int logicalIndex)
{
	if (logicalIndex >= 4)
	{
		ui->tableWidget->horizontalHeader()->setSortIndicatorShown(false);
		return;
	}

	ui->tableWidget->horizontalHeader()->setSortIndicatorShown(true);

	Qt::SortOrder order =  ui->tableWidget->horizontalHeader()->sortIndicatorOrder();
	ui->tableWidget->sortItems(logicalIndex, order);
}

void FileManagerDlg::onSearcherFinish(qint64 seq, int page, int maxPage, const QVariantList &vlist)
{
	Q_UNUSED(page);
	Q_UNUSED(maxPage);
	if (seq != m_searchSeq)
		return;

	showLoadWidget(false);
	ui->listViewPages->setEnabled(true);
	ui->btnSearch->setEnabled(true);

	m_data.clear();
	foreach (QVariant v, vlist)
	{
		if (!v.canConvert<QVariantMap>())
			continue;

		QVariantMap vmap = v.value<QVariantMap>();

		if (!vmap.contains("id") || 
			!vmap.contains("uuid") ||
			!vmap.contains("filename") || 
			!FileUtil::fileExists(vmap.value("filename").toString()))
		{
			continue;
		}

		QString uuid = vmap.value("uuid", "").toString();
		if (!uuid.isEmpty())
		{
			m_data[uuid] = vmap;
		}
	}

	currentPageChanged();
	if (!s_instance)
		return;

	ui->tableWidget->horizontalHeader()->setSortIndicator(3, Qt::DescendingOrder);
	ui->tableWidget->sortItems(3, Qt::DescendingOrder);
}

void FileManagerDlg::onMaximizeStateChanged(bool isMaximized)
{
	if (isMaximized)
	{
		ui->btnMaximize->setChecked(true);
		ui->btnMaximize->setToolTip(tr("Restore"));
	}
	else
	{
		ui->btnMaximize->setChecked(false);
		ui->btnMaximize->setToolTip(tr("Maximize"));
	}
}


#include "FileManagerDlg.moc"
