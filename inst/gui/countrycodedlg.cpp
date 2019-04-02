#include "countrycodedlg.h"
#include "ui_countrycodedlg.h"
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

static const int kNameRole   = Qt::UserRole + 1;
static const int kPinyinRole = Qt::UserRole + 2;
static const int kCodeRole   = Qt::UserRole + 3;
static const int kTypeRole   = Qt::UserRole + 4;
static const int kTypeValueGroup = 0;
static const int kTypeValueItem  = 1;

//////////////////////////////////////////////////////////////////////////
// CLASS CountryCodeItemDelegate
class CountryCodeItemDelegate : public QItemDelegate
{
public:
	explicit CountryCodeItemDelegate(QSortFilterProxyModel *proxyModel, QObject *parent = 0);
	QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
	void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;

private:
	void drawGroupItem(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
	void drawCountryCodeItem(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;

private:
	QSortFilterProxyModel *m_proxyModel;
};

//////////////////////////////////////////////////////////////////////////
// MEMBER FUNCTIONS OF CLASS CountryCodeItemDelegate
CountryCodeItemDelegate::CountryCodeItemDelegate(QSortFilterProxyModel *proxyModel, QObject *parent)
	: QItemDelegate(parent), m_proxyModel(proxyModel)
{
	Q_ASSERT(m_proxyModel);
}

QSize CountryCodeItemDelegate::sizeHint(const QStyleOptionViewItem &/*option*/, const QModelIndex &index) const
{
	int typeValue = index.data(kTypeRole).toInt();
	if (typeValue == kTypeValueGroup)
	{
		return QSize(22, 22);
	}
	else
	{
		return QSize(36, 36);
	}
}

void CountryCodeItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
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
		drawCountryCodeItem(painter, option, index);
	}
}

void CountryCodeItemDelegate::drawGroupItem(QPainter *painter, 
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

void CountryCodeItemDelegate::drawCountryCodeItem(QPainter* painter, 
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
	rect.setRight(rect.right() - 60);
	rect.setTop(rect.top() + 3);
	rect.setBottom(rect.bottom() - 3);
	painter->setPen(nameTextColor);
	QString name = m_proxyModel->mapToSource(index).data(kNameRole).toString();
	name = QFontMetrics(painter->font()).elidedText(name, Qt::ElideRight, rect.width());
	painter->drawText(rect, Qt::AlignLeft|Qt::AlignVCenter|Qt::TextSingleLine, name);

	// draw code
	rect.setLeft(rect.right()+5);
	rect.setWidth(50);
	QColor codeTextColor(128, 128, 128);
	painter->setPen(codeTextColor);
	QString code = m_proxyModel->mapToSource(index).data(kCodeRole).toString();
	code = QString("+") + code;
	QFont font = painter->font();
	font.setPointSize(9);
	code = QFontMetrics(font).elidedText(code, Qt::ElideRight, rect.width());
	painter->drawText(rect, Qt::AlignLeft|Qt::AlignVCenter|Qt::TextSingleLine, code);

	painter->restore();
}

//////////////////////////////////////////////////////////////////////////
// MEMBER FUNCTIONS OF CLASS CountryCodeDlg
CountryCodeDlg::CountryCodeDlg(QWidget *parent)
	: FramelessDialog(parent), m_proxyModel(0), m_dataModel(0)
{
	ui = new Ui::CountryCodeDlg();
	ui->setupUi(this);

	setWindowIcon(qApp->windowIcon());

	setWindowTitle(ui->title->text());

	setMainLayout(ui->verticalLayoutMain);
	setFixedSize(341, 492);
	setResizeable(false);

	initUI();

	connect(ui->btnClose, SIGNAL(clicked()), this, SLOT(reject()));

	setSkin();
}

CountryCodeDlg::~CountryCodeDlg()
{
	delete ui;
}

void CountryCodeDlg::showChineseCountryCode()
{
	loadCountryCodes(true);
	m_proxyModel->sort(0);
}

void CountryCodeDlg::showEnglishCountryCode()
{
	loadCountryCodes(false);
	m_proxyModel->sort(0);
}

QString CountryCodeDlg::selectedCountryCode() const
{
	return m_selectedCountryCode;
}

void CountryCodeDlg::setSkin()
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

void CountryCodeDlg::codeItemClicked(const QModelIndex &index)
{
	if (!index.isValid())
		return;

	if (index.data(kTypeRole).toInt() == kTypeValueGroup)
		return;

	m_selectedCountryCode = m_proxyModel->mapToSource(index).data(kCodeRole).toString();
	accept();
}

void CountryCodeDlg::countryFilterChanged(const QString &filter)
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

void CountryCodeDlg::countryFilterCleared()
{
	ui->lineEditFilter->setText("");
}

void CountryCodeDlg::initUI()
{
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
	m_proxyModel->setSortRole(kPinyinRole);
	m_proxyModel->setFilterRole(kNameRole);
	m_proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
	m_proxyModel->setFilterFixedString("");
	m_proxyModel->setSourceModel(m_dataModel);
	ui->listView->setModel(m_proxyModel);

	ui->listView->setItemDelegate(new CountryCodeItemDelegate(m_proxyModel, ui->listView));

	ui->lineEditFilter->setPlaceholderText(tr("Search"));

	bool connectOK = false;
	connectOK = connect(ui->listView, SIGNAL(clicked(QModelIndex)), this, SLOT(codeItemClicked(QModelIndex)));
	Q_ASSERT(connectOK);

	connectOK = connect(ui->lineEditFilter, SIGNAL(filterChanged(QString)), this, SLOT(countryFilterChanged(QString)));
	Q_ASSERT(connectOK);

	connectOK = connect(ui->lineEditFilter, SIGNAL(rightButtonClicked()), this, SLOT(countryFilterCleared()));
	Q_ASSERT(connectOK);
}

bool CountryCodeDlg::loadCountryCodes(bool chinese)
{
	QFile codeFile;
	if (chinese)
		codeFile.setFileName(":/countrycode/chineseCountryJson.txt");
	else
		codeFile.setFileName(":/countrycode/englishCountryJson.txt");
	codeFile.open(QIODevice::ReadOnly);
	QByteArray content = codeFile.readAll();
	codeFile.close();

	bool ok = false;
	QVariant v = QtJson::parse(QString::fromUtf8(content), ok);
	if (!ok)
		return false;

	// read all code items
	QMap<QString, QStandardItem *> codeGroups;
	QVariantList listV = v.toMap().value("data").toList();
	foreach (QVariant itemV, listV)
	{
		QVariantMap codeItem = itemV.toMap();
		QString countryName = codeItem["countryName"].toString();
		QString countryCode = codeItem["phoneCode"].toString();
		QString pinyinName;
		if (chinese)
			pinyinName = codeItem["countryPinyin"].toString().toUpper();
		else
			pinyinName = countryName.toUpper();

		if (countryCode != "(null)" && !pinyinName.isEmpty())
		{
			QStandardItem *codeItem = new QStandardItem();
			codeItem->setData(countryName, kNameRole);
			codeItem->setData(countryCode, kCodeRole);
			codeItem->setData(pinyinName, kPinyinRole);
			codeItem->setData(kTypeValueItem, kTypeRole);

			QString groupName = pinyinName[0];
			QStandardItem *groupItem = 0;
			if (codeGroups.contains(groupName))
			{
				groupItem = codeGroups[groupName];
			}
			else
			{
				groupItem = new QStandardItem();
				codeGroups[groupName] = groupItem;
				groupItem->setData(groupName, kNameRole);
				groupItem->setData(groupName, kPinyinRole);
				groupItem->setData(kTypeValueGroup, kTypeRole);
				m_dataModel->appendRow(groupItem);
			}
			m_dataModel->appendRow(codeItem);
		}
	}

	return true;
}
