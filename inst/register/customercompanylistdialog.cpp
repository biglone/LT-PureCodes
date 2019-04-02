#include "customercompanylistdialog.h"
#include "ui_customercompanylistdialog.h"
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QItemDelegate>
#include <QPainter>
#include "qt-json/json.h"
#include <QVariant>
#include <QVariantList>
#include <QVariantMap>
#include <QModelIndex>
#include <QMovie>
#include "PmApp.h"
#include "companyregistermanager.h"
#include "util/PinYinConverter.h"
#include "settings/GlobalSettings.h"

static const int kNameRole   = Qt::UserRole + 1;
static const int kPinyinRole = Qt::UserRole + 2;
static const int kTypeRole   = Qt::UserRole + 3;
static const int kIdRole     = Qt::UserRole + 4;
static const int kAddressRole= Qt::UserRole + 5;

static const int kTypeValueGroup = 0;
static const int kTypeValueItem  = 1;

//////////////////////////////////////////////////////////////////////////
// CLASS CustomerCompanyItemDelegate
class CustomerCompanyItemDelegate : public QItemDelegate
{
public:
	explicit CustomerCompanyItemDelegate(QSortFilterProxyModel *proxyModel, QObject *parent = 0);
	QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
	void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;

private:
	void drawGroupItem(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
	void drawCompanyItem(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;

private:
	QSortFilterProxyModel *m_proxyModel;
};

//////////////////////////////////////////////////////////////////////////
// MEMBER FUNCTIONS OF CLASS CustomerCompanyItemDelegate
CustomerCompanyItemDelegate::CustomerCompanyItemDelegate(QSortFilterProxyModel *proxyModel, QObject *parent)
	: QItemDelegate(parent), m_proxyModel(proxyModel)
{
	Q_ASSERT(m_proxyModel);
}

QSize CustomerCompanyItemDelegate::sizeHint(const QStyleOptionViewItem &/*option*/, const QModelIndex &index) const
{
	int typeValue = index.data(kTypeRole).toInt();
	if (typeValue == kTypeValueGroup)
	{
		return QSize(22, 22);
	}
	else
	{
		return QSize(58, 58);
	}
}

void CustomerCompanyItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	painter->setRenderHints(QPainter::TextAntialiasing|QPainter::SmoothPixmapTransform|QPainter::HighQualityAntialiasing);
	QRect paintRect = option.rect;
	int typeValue = index.data(kTypeRole).toInt();
	if (typeValue == kTypeValueGroup)
	{
		drawGroupItem(painter, option, index);
	}
	else
	{
		drawCompanyItem(painter, option, index);
	}
}

void CustomerCompanyItemDelegate::drawGroupItem(QPainter *painter, 
	const QStyleOptionViewItem &option, 
	const QModelIndex &index) const
{
	painter->save();

	// fill item background color
	QRect paintRect = option.rect;
	paintRect.setHeight(paintRect.height() - 1);
	painter->fillRect(paintRect, QColor("#f7f7f7"));

	// draw name
	QFont font = painter->font();
	font.setPointSize(9);
	painter->setFont(font);

	QFontMetrics fm = option.fontMetrics;
	paintRect = option.rect;
	paintRect.setLeft(paintRect.left() + 5);
	painter->setPen(QColor("#808080"));
	QString groupName = m_proxyModel->mapToSource(index).data(kNameRole).toString();
	painter->drawText(paintRect, Qt::AlignLeft|Qt::AlignVCenter, groupName);

	painter->restore();
}

void CustomerCompanyItemDelegate::drawCompanyItem(QPainter* painter, 
	const QStyleOptionViewItem& option, 
	const QModelIndex& index) const
{ 
	painter->save();

	// fill item background color
	QColor selectedBg("#fff5cc");
	QColor hoveredBg("#dfeefa");
	QRect paintRect = option.rect;
	paintRect.setHeight(paintRect.height() - 1);
	if (option.state & QStyle::State_Selected)
	{
		painter->fillRect(paintRect, selectedBg);
	}
	else if(option.state & QStyle::State_MouseOver)
	{
		painter->fillRect(paintRect, hoveredBg);
	}

	QModelIndex nextIndex = index.sibling(index.row()+1, index.column());
	if (nextIndex.isValid())
	{
		if (nextIndex.isValid() && nextIndex.data(kTypeRole).toInt() == kTypeValueItem)
		{
			QPoint leftPt = option.rect.bottomLeft();
			leftPt.rx() += 10;
			QPoint rightPt = option.rect.bottomRight();
			rightPt.rx() -= 10;
			painter->setPen(QColor("#f0f0f0"));
			painter->drawLine(leftPt, rightPt);
		}
	}

	// draw name
	QColor nameTextColor(0, 0, 0);
	QRect rect = paintRect;
	rect.setLeft(rect.left() + 5);
	rect.setRight(rect.right() - 6);
	rect.setBottom(rect.bottom()-rect.height()/2-3);
	painter->setPen(nameTextColor);
	QString name = m_proxyModel->mapToSource(index).data(kNameRole).toString();
	name = QFontMetrics(painter->font()).elidedText(name, Qt::ElideRight, rect.width());
	painter->drawText(rect, Qt::AlignLeft|Qt::AlignBottom|Qt::TextSingleLine, name);

	// draw address
	QColor addressTextColor(128, 128, 128);
	rect = paintRect;
	rect.setLeft(rect.left() + 5);
	rect.setRight(rect.right() - 6);
	rect.setTop(rect.bottom()-rect.height()/2+3);
	painter->setPen(addressTextColor);
	QString address = m_proxyModel->mapToSource(index).data(kAddressRole).toString();
	name = QFontMetrics(painter->font()).elidedText(address, Qt::ElideRight, rect.width());
	painter->drawText(rect, Qt::AlignLeft|Qt::AlignTop|Qt::TextSingleLine, address);

	painter->restore();
}

//////////////////////////////////////////////////////////////////////////
// MEMBER FUNCTIONS OF CLASS CustomerCompanyListDialog
CustomerCompanyListDialog::CustomerCompanyListDialog(QWidget *parent)
	: FramelessDialog(parent)
{
	ui = new Ui::CustomerCompanyListDialog();
	ui->setupUi(this);

	setWindowIcon(qApp->windowIcon());
	setWindowTitle(ui->title->text());

	setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);

	setMainLayout(ui->verticalLayoutMain);
	setFixedSize(341, 492);
	setResizeable(false);

	initUI();

	connect(ui->btnClose, SIGNAL(clicked()), this, SLOT(reject()));

	setSkin();
}

CustomerCompanyListDialog::~CustomerCompanyListDialog()
{
	delete ui;
}

QString CustomerCompanyListDialog::selCompanyName() const
{
	return m_selCompanyName;
}

void CustomerCompanyListDialog::setSkin()
{
	// set background image
	QPixmap bgPixmap(":/theme/dialog_bgs/dialog_bg_3.png");
	FramelessDialog::BGSizes bgSizes = {0};
	bgSizes.borderwidth = 5;
	bgSizes.topBarHeight = 31;
	setBG(bgPixmap, bgSizes);

	// set title label style
	ui->title->setStyleSheet("QLabel {font-size: 12pt; color: white;}");

	// set tree view style
	ui->listView->setStyleSheet(QString("QListView{"
		"font-size: 10.5pt;"
		"border: none;"
		"background-color: transparent;"
		"}"
		));
}

void CustomerCompanyListDialog::onCustomerListOK()
{
	stopLoading();

	QMap<QString, QStandardItem *> companyGroups;
	QList<CompanyRegisterManager::CustomerItem> companies = qPmApp->getCompanyRegisterManager()->customerList();
	foreach (CompanyRegisterManager::CustomerItem company, companies)
	{
		if (company.m_id.isEmpty() || company.m_name.isEmpty())
			continue;

		QStringList pinyinName = PinyinConveter::instance().quanpin(company.m_name);
		QString address = company.m_region;
		if (address.isEmpty())
			address = company.m_province;
		else
			address.append(QString(" ") + company.m_province);
		if (address.isEmpty())
			address = company.m_country;
		else
			address.append(QString(" ") + company.m_country);

		QStandardItem *companyItem = new QStandardItem();
		companyItem->setData(company.m_name, kNameRole);
		companyItem->setData(company.m_id, kIdRole);
		companyItem->setData(pinyinName.join(""), kPinyinRole);
		companyItem->setData(kTypeValueItem, kTypeRole);
		companyItem->setData(address, kAddressRole);

		QString groupName = pinyinName[0].left(1).toUpper();
		QStandardItem *groupItem = 0;
		if (companyGroups.contains(groupName))
		{
			groupItem = companyGroups[groupName];
		}
		else
		{
			groupItem = new QStandardItem();
			companyGroups[groupName] = groupItem;
			groupItem->setData(groupName, kNameRole);
			groupItem->setData(groupName, kPinyinRole);
			groupItem->setData(kTypeValueGroup, kTypeRole);
			m_dataModel->appendRow(groupItem);
		}
		m_dataModel->appendRow(companyItem);
	}

	m_proxyModel->sort(0);
	ui->listView->update();
}

void CustomerCompanyListDialog::onCustomerListFailed(int retCode, const QString &desc)
{
	ui->labelLoading->movie()->stop();
	ui->labelLoading->setVisible(false);
	ui->btnRetry->setVisible(true);
	ui->labelText->setText(QString("%1(%2)").arg(desc).arg(retCode));
}

void CustomerCompanyListDialog::companyItemClicked(const QModelIndex &index)
{
	if (!index.isValid())
		return;

	if (index.data(kTypeRole).toInt() == kTypeValueGroup)
		return;

	m_selCompanyName = m_proxyModel->mapToSource(index).data(kNameRole).toString();
	accept();
}

void CustomerCompanyListDialog::companyFilterChanged(const QString &filter)
{
	if (!filter.isEmpty())
	{
		QString filterString = QString("*%1*").arg(filter);
		m_proxyModel->setFilterWildcard(filterString);
	}
	else
	{
		m_proxyModel->setFilterFixedString("");
	}
	m_proxyModel->invalidate();
}

void CustomerCompanyListDialog::companyFilterCleared()
{
	ui->lineEditFilter->setText("");
}

void CustomerCompanyListDialog::on_btnRetry_clicked()
{
	QString language("EN");
	if (GlobalSettings::language() == GlobalSettings::Language_ZH_CN)
		language = QString("ZH");
	CompanyRegisterManager *companyRegisterManager = qPmApp->getCompanyRegisterManager();
	companyRegisterManager->getCustomerList(language);
	startLoading();
}

void CustomerCompanyListDialog::initUI()
{
	// loading animation
	QMovie *movie = new QMovie(this);
	movie->setFileName(":/images/loading_image_small.gif");
	ui->labelLoading->setMovie(movie);

	ui->btnRetry->setVisible(false);

	ui->listView->setEditTriggers(QListView::NoEditTriggers);
	ui->listView->setContextMenuPolicy(Qt::NoContextMenu);
	ui->listView->setMouseTracking(true);
	ui->listView->setAttribute(Qt::WA_Hover, true);
	ui->listView->setDragEnabled(false);
	ui->listView->setAcceptDrops(false);
	ui->listView->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);

	m_dataModel = new QStandardItemModel(this);
	m_proxyModel = new QSortFilterProxyModel(this);
	m_proxyModel->setDynamicSortFilter(false);
	m_proxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
	m_proxyModel->setSortRole(kPinyinRole);
	m_proxyModel->setFilterRole(kNameRole);
	m_proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
	m_proxyModel->setFilterFixedString("");
	m_proxyModel->setSourceModel(m_dataModel);
	ui->listView->setModel(m_proxyModel);

	ui->listView->setItemDelegate(new CustomerCompanyItemDelegate(m_proxyModel, ui->listView));

	ui->lineEditFilter->setPlaceholderText(tr("Search"));

	bool connectOK = false;
	connectOK = connect(ui->listView, SIGNAL(clicked(QModelIndex)), this, SLOT(companyItemClicked(QModelIndex)));
	Q_ASSERT(connectOK);

	connectOK = connect(ui->lineEditFilter, SIGNAL(filterChanged(QString)), this, SLOT(companyFilterChanged(QString)));
	Q_ASSERT(connectOK);

	connectOK = connect(ui->lineEditFilter, SIGNAL(rightButtonClicked()), this, SLOT(companyFilterCleared()));
	Q_ASSERT(connectOK);

	CompanyRegisterManager *companyRegisterManager = qPmApp->getCompanyRegisterManager();
	connectOK = connect(companyRegisterManager, SIGNAL(getCustomerListOK()), this, SLOT(onCustomerListOK()));
	Q_ASSERT(connectOK);

	connectOK = connect(companyRegisterManager, SIGNAL(getCustomerListFailed(int, QString)), this, SLOT(onCustomerListFailed(int, QString)));
	Q_ASSERT(connectOK);

	QString language("EN");
	if (GlobalSettings::language() == GlobalSettings::Language_ZH_CN)
		language = QString("ZH");
	companyRegisterManager->getCustomerList(language);
	startLoading();
}

void CustomerCompanyListDialog::startLoading()
{
	ui->centralStackedWidget->setCurrentIndex(1);
	ui->labelLoading->setVisible(true);
	ui->labelLoading->movie()->start();
	ui->btnRetry->setVisible(false);
	ui->labelText->setText(tr("Loading..."));
}

void CustomerCompanyListDialog::stopLoading()
{
	ui->labelLoading->movie()->stop();
	ui->labelLoading->setVisible(false);
	ui->labelText->clear();
	ui->centralStackedWidget->setCurrentIndex(0);
}

