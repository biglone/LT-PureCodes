#include <QMovie>
#include <QDebug>
#include <QApplication>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QDesktopWidget>
#include <QStackedWidget>
#include <QPushButton>
#include <QToolButton>
#include "EmotionUtil.h"
#include "emotion/favoriteemotionsettings.h"
#include "Account.h"
#include <QDir>
#include <QStandardPaths>
#include "util/FileDialog.h"
#include "emotionitem.h"
#include "addfavoriteemotiondialog.h"
#include <QCryptographicHash>
#include "pmessagebox.h"
#include <QFile>
#include <QFileInfo>
#include <QDateTime>
#include <QToolButton>
#include <QAction>
#include <QMenu>
#include <QMouseEvent>
#include <QDesktopWidget>
#include "PlainTextLineInput.h"
#include "delfavoritegroupdialog.h"
#include "settings/GlobalSettings.h"

static const int kInitFavoriteRowCount = 3;
static const int kInitFavoriteColCount = 7;
static const int kInitDefaultRowCount  = 7;
static const int kInitDefaultColCount  = 14;

//////////////////////////////////////////////////////////////////////////
// CLASS EmotionTableWidget
class EmotionTableWidget : public QTableWidget
{
public:
	EmotionTableWidget(QWidget *parent = 0)
		: QTableWidget(parent) 
	{
		m_clickedIndex = QModelIndex();
	}

	QModelIndex leftButtonClickedIndex() const {return m_clickedIndex;}

protected:
	void mouseReleaseEvent(QMouseEvent *event)
	{
		m_clickedIndex = QModelIndex();
		if (event->button() == Qt::LeftButton)
		{
			m_clickedIndex = indexAt(event->pos());
		}
		QTableWidget::mouseReleaseEvent(event);
	}

private:
	QModelIndex m_clickedIndex;
};

//////////////////////////////////////////////////////////////////////////
// CLASS FavoriteEmotionPanel
class EmotionDialog;
class FavoriteEmotionPanel : public QWidget
{
	Q_OBJECT

public:
	enum Page
	{
		EmotionPage = 0,
		EmptyPage = 1
	};

public:
	FavoriteEmotionPanel(EmotionDialog *parent, const QString &groupId, const QString &groupName);
	virtual ~FavoriteEmotionPanel();

	void setGroupId(const QString &groupId);
	void setGroupName(const QString &groupName);
	QString groupId() const;
	QString groupName() const;

	void setFavoriteEmotions(const QStringList &emotionIds, const QStringList &emotionNames, const QStringList &emotionFileNames);

	void setPage(FavoriteEmotionPanel::Page page);

private slots:
	void onFavoriteTableWidgetClicked(const QModelIndex &index);
	void favoriteContextMenu(const QPoint &pt);
	void delFavorite();
	void moveFirstFavorite();
	void renameFavorite();
	void moveToOtherGroup();

private:
	void initUI();
	void setSkin();

private:
	EmotionDialog                *m_emotionDialog;
	QStackedWidget               *m_stackedWidget;
	QLabel                       *m_labelGroupName;
	EmotionTableWidget           *m_favoriteTableWidget;
	QString                       m_groupId;
	QString                       m_groupName;
	QMap<QString, EmotionItem *>  m_favoriteEmotions;
	QList<EmotionItem *>          m_favoriteEmotionList;
	QAction                      *m_delFavorite;
	QAction                      *m_moveFirstFavorite;
	QAction                      *m_renameFavorite;
	QPushButton                  *m_addFavoriteButton;
	QMap<QString, QAction *>      m_otherGroupActions;
};

//////////////////////////////////////////////////////////////////////////
// CLASS EmotionDialog
class EmotionDialog : public QWidget
{
	Q_OBJECT

public:
	EmotionDialog();
	virtual ~EmotionDialog();

	void setDefaultEmotions(const QStringList &names, const QStringList &files);
	void resetFavoriteGroups();
	void addFavoriteGroup(const QString &groupId, const QString &groupName);
	void setFavoriteEmotions(const QString &groupId, const QStringList &emotionIds, 
		const QStringList &emotionNames, const QStringList &emotionFileNames);
	void setCallback(EmotionCallback *cb);
	EmotionCallback *emotionCallback() const;
	void selectFavoriteEmotion(const QString &emotionId, const QString &groupId);
	void setCurrentGroup(const QString &groupId);
	void checkHasEmotion();
	void delFavoriteGroup(const QString &groupId);
	void setFavoriteGroupName(const QString &oldGroupId, const QString &newGroupId, const QString &groupName);
	void moveFavoriteGroupBefore(const QString &groupId);
	void moveFavoriteGroupToTop(const QString &groupId);
	void moveFavoriteGroupAfter(const QString &groupId);
	int nextFavoriteButtonIndex() const;
	int currentFavoriteGroupIndex() const;

public slots:
	void setSkin();

private slots:
	void on_defaultButton_toggled(bool checked);
	void onFavoriteButtonToggled(bool checked);
	void onDefaultButtonContextMenu(const QPoint &pt);
	void onFavoriteButtonContextMenu(const QPoint &pt);
	void customContextMenu(const QPoint &pt);
	void addFavoriteEmotion();
	void onDefaultTableWidgetClicked(const QModelIndex &index);
	void onCBDestroy(QObject *obj);

	void addGroup();
	void renameGroup();
	void delGroup();
	void moveGroupBefore();
	void moveGroupAfter();
	void moveGroupToTop();

protected:
	void hideEvent(QHideEvent *e);

private:
	void setupUi(QWidget *dlg);
	void retranslateUi(QWidget *dlg);
	void configCurrentGroup();

private:
	EmotionTableWidget                   *m_defaultTableWidget;
	QStackedWidget                       *m_stackedWidget;
	QHBoxLayout                          *m_favoriteButtonLayout;
	QPushButton                          *m_defaultButton;
	QList<QPushButton *>                  m_favoriteButtons;
	QMap<QString, FavoriteEmotionPanel *> m_favoritePanels;
	QMap<QPushButton *, QString>          m_favoriteGroupIds;
	QToolButton                          *m_addGroupToolButton;
	QToolButton                          *m_addFavoriteToolButton;
	EmotionCallback                      *m_cb;

	QAction *m_addGroupAction;
	QAction *m_renameGroupAction;
	QAction *m_delGroupAction;
	QAction *m_moveGroupBeforeAction;
	QAction *m_moveGroupAfterAction;
	QAction *m_moveGroupToTopAction;
};

//////////////////////////////////////////////////////////////////////////
// MEMBER FUNCTIONS OF CLASS FavoriteEmotionPanel
FavoriteEmotionPanel::FavoriteEmotionPanel(EmotionDialog *parent, const QString &groupId, const QString &groupName)
: QWidget(parent), m_groupId(groupId), m_groupName(groupName), m_emotionDialog(parent)
{
	Q_ASSERT(m_emotionDialog);

	initUI();
	setSkin();

	m_delFavorite = new QAction(this);
	m_delFavorite->setText(tr("Delete Emotion"));

	m_moveFirstFavorite = new QAction(this);
	m_moveFirstFavorite->setText(tr("Put First"));

	m_renameFavorite = new QAction(this);
	m_renameFavorite->setText(tr("Rename"));

	connect(m_favoriteTableWidget, SIGNAL(clicked(QModelIndex)), this, SLOT(onFavoriteTableWidgetClicked(QModelIndex)));
	connect(m_delFavorite, SIGNAL(triggered()), this, SLOT(delFavorite()));
	connect(m_moveFirstFavorite, SIGNAL(triggered()), this, SLOT(moveFirstFavorite()));
	connect(m_renameFavorite, SIGNAL(triggered()), this, SLOT(renameFavorite()));
	connect(m_addFavoriteButton, SIGNAL(clicked()), m_emotionDialog, SLOT(addFavoriteEmotion()));
}

FavoriteEmotionPanel::~FavoriteEmotionPanel()
{

}

void FavoriteEmotionPanel::setGroupId(const QString &groupId)
{
	m_groupId = groupId;
}

void FavoriteEmotionPanel::setGroupName(const QString &groupName)
{
	m_groupName = groupName;
	m_labelGroupName->setText(groupName);
}

QString FavoriteEmotionPanel::groupId() const
{
	return m_groupId;
}

QString FavoriteEmotionPanel::groupName() const
{
	return m_groupName;
}

void FavoriteEmotionPanel::setFavoriteEmotions(const QStringList &emotionIds, 
											   const QStringList &emotionNames, 
											   const QStringList &emotionFileNames)
{
	QString sFilename;
	QString sFacename;
	QString sFaceId;
	EmotionItem *pWidget = 0;
	int i = 0, j = 0;

	m_favoriteTableWidget->clear();
	qDeleteAll(m_favoriteEmotions.values());
	m_favoriteEmotions.clear();
	m_favoriteEmotionList.clear();

	int nColumn = m_favoriteTableWidget->columnCount();
	int nRow = emotionNames.count()/nColumn;
	if ((emotionNames.count() % nColumn) != 0)
		nRow += 1;
	if (nRow < kInitFavoriteRowCount)
		nRow = kInitFavoriteRowCount;
	m_favoriteTableWidget->setRowCount(nRow);
	nRow = m_favoriteTableWidget->rowCount();
	int count = nRow*nColumn;

	QMovie *pMovie = 0;
	for (int index = 0; index < count; ++index)
	{
		m_favoriteTableWidget->setRowHeight(index/nColumn, kFavoriteEmotionSize+7);
		m_favoriteTableWidget->setColumnWidth(index%nColumn, kFavoriteEmotionSize+7);

		if (index < emotionIds.count())
		{
			sFaceId = emotionIds.value(index);
			sFacename = emotionNames.value(index);
			sFilename = emotionFileNames.value(index);

			pWidget = new EmotionItem(m_favoriteTableWidget);
			pWidget->setFixedSize(QSize(kFavoriteEmotionSize+6, kFavoriteEmotionSize+6));
			pMovie = new QMovie(this);
			pMovie->setFileName(sFilename);
			pWidget->setEmotion(FavoriteEmotion, pMovie);
			pWidget->setEmotionId(sFaceId);
			pWidget->setContextMenuPolicy(Qt::CustomContextMenu);
			if (sFacename.isEmpty())
				sFacename = tr("Empty");
			pWidget->setToolTip(tr("Name: %1").arg(sFacename));
			m_favoriteEmotions.insert(sFaceId, pWidget);
			m_favoriteEmotionList.append(pWidget);
			connect(pWidget, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(favoriteContextMenu(const QPoint&)));
		}
		else
		{
			pWidget = new EmotionItem(m_favoriteTableWidget);
			pWidget->setFixedSize(QSize(kFavoriteEmotionSize+6, kFavoriteEmotionSize+6));
		}
		m_favoriteTableWidget->setCellWidget(i, j, pWidget);

		if (++j >= nColumn)
		{
			j = 0;
			++i;
		}
	}

	m_favoriteTableWidget->resizeColumnsToContents();
	m_favoriteTableWidget->resizeRowsToContents();

	if (m_favoriteEmotions.isEmpty())
	{
		if (m_stackedWidget->currentIndex() == 1)
			m_stackedWidget->setCurrentIndex(2);
	}
	else
	{
		if (m_stackedWidget->currentIndex() == 2)
			m_stackedWidget->setCurrentIndex(1);
	}
}

void FavoriteEmotionPanel::setPage(FavoriteEmotionPanel::Page page)
{
	m_stackedWidget->setCurrentIndex((int)page);
}

void FavoriteEmotionPanel::onFavoriteTableWidgetClicked(const QModelIndex &index)
{
	if (!index.isValid())
		return;

	if (m_favoriteTableWidget->leftButtonClickedIndex() != index)
		return;

	int row = index.row();
	int column = index.column();
	EmotionItem *emotionItem = qobject_cast<EmotionItem *>(m_favoriteTableWidget->cellWidget(row, column));
	if (!emotionItem)
		return;

	QString emotionId = emotionItem->emotionId();
	if (emotionId.isEmpty())
		return;

	m_favoriteTableWidget->scrollToTop();

	m_emotionDialog->selectFavoriteEmotion(emotionId, m_groupId);
}

void FavoriteEmotionPanel::favoriteContextMenu(const QPoint & /*pt*/)
{
	EmotionItem *emotionItem = static_cast<EmotionItem *>(sender());
	if (!emotionItem)
		return;

	int emotionIndex = m_favoriteEmotionList.indexOf(emotionItem);
	if (emotionIndex < 0)
		return;

	QString emotionId = emotionItem->emotionId();
	if (!emotionId.isEmpty())
	{
		m_delFavorite->setData(emotionId);
		m_moveFirstFavorite->setData(emotionId);
		m_renameFavorite->setData(emotionId);

		if (emotionIndex == 0)
			m_moveFirstFavorite->setEnabled(false);
		else
			m_moveFirstFavorite->setEnabled(true);

		qDeleteAll(m_otherGroupActions.values());
		m_otherGroupActions.clear();
		QStringList groupIds;
		QStringList groupNames;
		EmotionUtil::instance().getFavoriteGroups(groupIds, groupNames);
		int groupIndex = 0;
		foreach (QString groupId, groupIds)
		{
			if (groupId != m_groupId)
			{
				QAction *groupAction = new QAction(this);
				groupAction->setText(groupNames[groupIndex]);
				groupAction->setData(emotionId);
				connect(groupAction, SIGNAL(triggered()), this, SLOT(moveToOtherGroup()));
				m_otherGroupActions.insert(groupId, groupAction);
			}
			++groupIndex;
		}

		QMenu menu;
		menu.addAction(m_moveFirstFavorite);
		menu.addAction(m_delFavorite);
		menu.addAction(m_renameFavorite);

		if (!m_otherGroupActions.isEmpty())
		{
			menu.addSeparator();
			QMenu *otherGroupMenu = menu.addMenu(tr("Move Emotion To"));
			foreach (QString groupId, groupIds)
			{
				if (groupId != m_groupId)
				{
					QAction *groupAction = m_otherGroupActions[groupId];
					otherGroupMenu->addAction(groupAction);
				}
			}
		}

		qApp->postEvent(emotionItem, new QEvent(QEvent::HoverLeave));

		menu.exec(QCursor::pos());
	}
}

void FavoriteEmotionPanel::delFavorite()
{
	QAction *action = qobject_cast<QAction *>(sender());
	if (action != m_delFavorite)
		return;

	QString emotionId = action->data().toString();
	if (emotionId.isEmpty())
		return;

	EmotionUtil::instance().delFavoriteEmotion(emotionId, m_groupId);
}

void FavoriteEmotionPanel::moveFirstFavorite()
{
	QAction *action = qobject_cast<QAction *>(sender());
	if (action != m_moveFirstFavorite)
		return;

	QString emotionId = action->data().toString();
	if (emotionId.isEmpty())
		return;

	EmotionUtil::instance().moveFirstFavoriteEmotion(emotionId, m_groupId);
}

void FavoriteEmotionPanel::renameFavorite()
{
	QAction *action = qobject_cast<QAction *>(sender());
	if (action != m_renameFavorite)
		return;

	QString emotionId = action->data().toString();
	if (emotionId.isEmpty())
		return;

	EmotionItem *emotionItem = m_favoriteEmotions.value(emotionId, 0);
	if (!emotionItem)
		return;

	QString oldName = EmotionUtil::instance().favoriteEmotionName(emotionId);
	QWidget *parent = m_emotionDialog->emotionCallback() ? m_emotionDialog->emotionCallback()->instanceWindow() : 0;
	PlainTextLineInput emotionInput(parent);
	emotionInput.setWindowModality(Qt::WindowModal);
	emotionInput.init(tr("Rename"), tr("Emotion name:"), kMaxEmotionNameSize, PlainTextLineInput::ModeUnicode, oldName, true);
	if (QDialog::Rejected == emotionInput.exec())
		return;

	QString newName = emotionInput.getInputText().trimmed();
	if (EmotionUtil::instance().setFavoriteEmotionName(emotionId, newName))
	{
		if (newName.isEmpty())
			newName = tr("Empty");
		emotionItem->setToolTip(tr("Emotion name: %1").arg(newName));
	}
}

void FavoriteEmotionPanel::moveToOtherGroup()
{
	QAction *action = qobject_cast<QAction *>(sender());
	if (!m_otherGroupActions.values().contains(action))
		return;

	QString emotionId = action->data().toString();
	if (emotionId.isEmpty())
		return;

	QString groupId = m_otherGroupActions.key(action);
	EmotionUtil::instance().moveFavoriteEmotionToOtherGroup(emotionId, m_groupId, groupId);
}

void FavoriteEmotionPanel::initUI()
{
	m_labelGroupName = new QLabel(this);
	m_labelGroupName->setText(m_groupName);
	m_labelGroupName->setIndent(13);
	m_labelGroupName->setContentsMargins(0, 0, 0, 5);

	m_stackedWidget = new QStackedWidget(this);
	m_stackedWidget->setContentsMargins(0, 0, 2, 0);
	QVBoxLayout *verticalLayoutMain = new QVBoxLayout(this);
	verticalLayoutMain->setSpacing(0);
	verticalLayoutMain->setContentsMargins(0, 10, 0, 19);
	verticalLayoutMain->addWidget(m_labelGroupName);
	verticalLayoutMain->addWidget(m_stackedWidget);
	
	m_favoriteTableWidget = new EmotionTableWidget(this);
	m_favoriteTableWidget->setObjectName("favoriteTableWidget");
	m_favoriteTableWidget->setColumnCount(kInitFavoriteColCount);
	m_favoriteTableWidget->setRowCount(kInitFavoriteRowCount);
	m_favoriteTableWidget->setShowGrid(true);
	m_favoriteTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
	m_favoriteTableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
	m_favoriteTableWidget->verticalHeader()->setVisible(false);
	m_favoriteTableWidget->horizontalHeader()->setVisible(false);
	m_favoriteTableWidget->setContextMenuPolicy(Qt::CustomContextMenu);
	m_favoriteTableWidget->setVerticalScrollMode(QAbstractItemView::ScrollPerItem);
	m_favoriteTableWidget->setHorizontalScrollMode(QAbstractItemView::ScrollPerItem);
	m_favoriteTableWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
	m_favoriteTableWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	m_favoriteTableWidget->setFocusPolicy(Qt::NoFocus);

	QWidget *noneFavoritePage = new QWidget(this);
	noneFavoritePage->setObjectName("noneFavoritePage");
	QVBoxLayout *noneFavoritePageLayout = new QVBoxLayout(noneFavoritePage);
	m_addFavoriteButton = new QPushButton(noneFavoritePage);
	m_addFavoriteButton->setObjectName(QString::fromUtf8("addFavoriteButton"));
	m_addFavoriteButton->setText(tr("Add My First Emotion"));
	m_addFavoriteButton->setFixedSize(196, 40);
	QHBoxLayout *addFavoriteButtonLayout = new QHBoxLayout();
	addFavoriteButtonLayout->addStretch(1);
	addFavoriteButtonLayout->addWidget(m_addFavoriteButton);
	addFavoriteButtonLayout->addStretch(1);
	noneFavoritePageLayout->addStretch(1);
	noneFavoritePageLayout->addLayout(addFavoriteButtonLayout);
	noneFavoritePageLayout->addStretch(1);

	m_stackedWidget->addWidget(m_favoriteTableWidget);
	m_stackedWidget->addWidget(noneFavoritePage);
	m_stackedWidget->setCurrentIndex((int)EmotionPage);
}

void FavoriteEmotionPanel::setSkin()
{
	m_labelGroupName->setStyleSheet("color: rgb(128, 128, 128); font-size: 9pt;");
	this->setStyleSheet(QString(
		"QTableWidget#favoriteTableWidget {"
		"	padding-top: 4px;"
		"	padding-left: 13px;"
		"	selection-background-color: transparent;"
		"	gridline-color: #e0e0e0;"
		"	border: none;"
		"}"
		"QWidget#noneFavoritePage {"
		"	background-color: rgb(246, 251, 254);"
		"	margin: 10px 7px 3px 10px;"
		"}"
	));

	QFile qssFile;
	qssFile.setFileName(":/theme/qss/pushbutton2_skin.qss");
	if (qssFile.open(QIODevice::ReadOnly))
	{
		QString qss = qssFile.readAll();
		m_addFavoriteButton->setStyleSheet(qss);
		qssFile.close();
	}
}

//////////////////////////////////////////////////////////////////////////
// MEMBER FUNCTIONS OF CLASS EmotionDialog
EmotionDialog::EmotionDialog()
: m_cb(0)
{
	setWindowFlags(windowFlags()|Qt::Popup|Qt::FramelessWindowHint);

	setupUi(this);

	m_addGroupAction = new QAction(tr("Add Group"), this);
	m_renameGroupAction = new QAction(tr("Rename"), this);
	m_delGroupAction = new QAction(tr("Delete Group"), this);
	m_moveGroupBeforeAction = new QAction(tr("Move Forward"), this);
	m_moveGroupAfterAction = new QAction(tr("Move Afterward"), this);
	m_moveGroupToTopAction = new QAction(tr("Stick"), this);

	connect(m_addGroupAction, SIGNAL(triggered()), this, SLOT(addGroup()));
	connect(m_renameGroupAction, SIGNAL(triggered()), this, SLOT(renameGroup()));
	connect(m_delGroupAction, SIGNAL(triggered()), this, SLOT(delGroup()));
	connect(m_moveGroupBeforeAction, SIGNAL(triggered()), this, SLOT(moveGroupBefore()));
	connect(m_moveGroupAfterAction, SIGNAL(triggered()), this, SLOT(moveGroupAfter()));
	connect(m_moveGroupToTopAction, SIGNAL(triggered()), this, SLOT(moveGroupToTop()));

	connect(m_addGroupToolButton, SIGNAL(clicked()), this, SLOT(addGroup()));
	connect(m_addFavoriteToolButton, SIGNAL(clicked()), this, SLOT(addFavoriteEmotion()));
	connect(m_defaultTableWidget, SIGNAL(clicked(QModelIndex)), this, SLOT(onDefaultTableWidgetClicked(QModelIndex)));

	connect(m_defaultButton, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onDefaultButtonContextMenu(QPoint)));


	setSkin();
}

EmotionDialog::~EmotionDialog()
{
}

void EmotionDialog::setDefaultEmotions(const QStringList &names, const QStringList &files)
{
	QString sFilename;
	QString sFacename;
	EmotionItem *pWidget = 0;
	int i = 0, j = 0;

	int nColumn = m_defaultTableWidget->columnCount();
	int nRow = qRound(names.size()/(nColumn) * 1.0 + 0.5);
	m_defaultTableWidget->setRowCount(nRow);
	nRow = m_defaultTableWidget->rowCount();
	int count = nRow * nColumn;

	QMovie *pMovie = 0;
	for (int index = 0; index < count; ++index)
	{
		if (index < names.count())
		{
			sFacename = names.value(index);
			sFilename = files.value(index);

			pWidget = new EmotionItem(m_defaultTableWidget);
			pWidget->setFixedSize(QSize(kDefaultEmotionSize+10, kDefaultEmotionSize+10));
			pMovie = new QMovie(this);
			pMovie->setFileName(sFilename);
			pWidget->setEmotion(DefaultEmotion, pMovie);
			pWidget->setEmotionId(QString::number(index));
			pWidget->setToolTip(sFacename);
		}
		else
		{
			pWidget = new EmotionItem(m_defaultTableWidget);
			pWidget->setFixedSize(QSize(kDefaultEmotionSize+10, kDefaultEmotionSize+10));
		}
		m_defaultTableWidget->setCellWidget(i, j, pWidget);
		m_defaultTableWidget->setRowHeight(i, kDefaultEmotionSize+11);
		m_defaultTableWidget->setColumnWidth(j, kDefaultEmotionSize+11);
		if (++j >= nColumn)
		{
			j = 0;
			++i;
		}
	}

// 	m_defaultTableWidget->resizeColumnsToContents();
// 	m_defaultTableWidget->resizeRowsToContents();
}

void EmotionDialog::resetFavoriteGroups()
{
	// remove all panels
	foreach (FavoriteEmotionPanel *panel, m_favoritePanels.values())
	{
		m_stackedWidget->removeWidget(panel);
	}

	qDeleteAll(m_favoritePanels.values());
	m_favoritePanels.clear();

	// remove all buttons
	foreach (QPushButton *favoriteButton, m_favoriteButtons)
	{
		m_favoriteButtonLayout->removeWidget(favoriteButton);
	}
	m_favoriteGroupIds.clear();
	qDeleteAll(m_favoriteButtons);
	m_favoriteButtons.clear();
}

void EmotionDialog::addFavoriteGroup(const QString &groupId, const QString &groupName)
{
	const QString favoriteButtonStyle = QString(
		"QPushButton {"
		"	padding-left: 0px;"
		"	padding-right: 0px;"
		"	border: none;"
		"	border-image: none;"
		"	background: transparent;"
		"	color: rgb(255, 163, 41);"
		"	font-size: 9pt;"
		"	text-align: bottom;"
		"}"
		"QPushButton:!checked:hover {"
		"	border-image: none;"
		"	border: none;"
		"	border-left: 1px solid rgb(212, 212, 212);"
		"	border-right: 1px solid rgb(212, 212, 212);"
		"	background: white;"
		"}"
		"QPushButton:checked {"
		"	border-image: none;"
		"	border: none;"
		"	border-left: 1px solid rgb(212, 212, 212);"
		"	border-right: 1px solid rgb(212, 212, 212);"
		"	background: white;"
		"}"
	);

	// add favorite button
	QPushButton *favoriteButton = new QPushButton(this);
	favoriteButton->setFixedHeight(36);
	favoriteButton->setFixedWidth(45);
	favoriteButton->setCheckable(true);
	favoriteButton->setChecked(false);
	favoriteButton->setIcon(QIcon(":/images/Icon_137.png"));
	favoriteButton->setIconSize(QSize(24, 24));
	favoriteButton->setToolTip(groupName);
	favoriteButton->setStyleSheet(favoriteButtonStyle);
	favoriteButton->setContextMenuPolicy(Qt::CustomContextMenu);

	if (groupId != QString::fromLatin1(kFirstGroupId) && 0 != nextFavoriteButtonIndex())
		favoriteButton->setText(QString::number(nextFavoriteButtonIndex()));
	m_favoriteButtonLayout->addWidget(favoriteButton);
	connect(favoriteButton, SIGNAL(toggled(bool)), this, SLOT(onFavoriteButtonToggled(bool)));
	connect(favoriteButton, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onFavoriteButtonContextMenu(QPoint)));

	// add favorite panel
	FavoriteEmotionPanel *panel = new FavoriteEmotionPanel(this, groupId, groupName);
	panel->setFavoriteEmotions(QStringList(), QStringList(), QStringList());
	if (EmotionUtil::instance().hasFavoriteEmotion())
		panel->setPage(FavoriteEmotionPanel::EmotionPage);
	else
		panel->setPage(FavoriteEmotionPanel::EmptyPage);
	m_stackedWidget->addWidget(panel);

	m_favoriteButtons.append(favoriteButton);
	m_favoriteGroupIds.insert(favoriteButton, groupId);
	m_favoritePanels.insert(groupId, panel);
}

void EmotionDialog::setFavoriteEmotions(const QString &groupId, const QStringList &emotionIds, 
										const QStringList &emotionNames, const QStringList &emotionFileNames)
{
	if (groupId.isEmpty())
		return;

	FavoriteEmotionPanel *favoritePanel = m_favoritePanels.value(groupId, 0);
	if (favoritePanel)
		favoritePanel->setFavoriteEmotions(emotionIds, emotionNames, emotionFileNames);
}

void EmotionDialog::setSkin()
{
	this->setStyleSheet(QString( 
		"QStackedWidget#stackedWidget {"
		"	background: white;"	
		"	padding-right: 3px;"
		"	padding-bottom: 3px;"
		"	border-top: 1px solid rgb(128, 129, 130);"
		"	border-left: 1px solid rgb(128, 129, 130);"
		"	border-right: 1px solid rgb(128, 129, 130);"
		"	border-bottom: none;"
		"}"
		"QTableWidget#defaultTableWidget {"
		"	padding-top: 13px;"
		"	padding-left: 13px;"
		"	selection-background-color: transparent;"
		"	gridline-color: #e0e0e0;"
		"	border: none;"
		"}"
		"QWidget#bottomBar {"
		"	background-color: rgb(228, 228, 228);"
		"	border-left: 1px solid rgb(128, 129, 130);"
		"	border-right: 1px solid rgb(128, 129, 130);"
		"	border-bottom: 1px solid rgb(128, 129, 130);"
		"	padding-bottom: 1px;"
		"}"
		"QPushButton#defaultButton {"
		"	padding-left: 0px;"
		"	padding-right: 0px;"
		"	border: none;"
		"	border-image: none;"
		"	background: transparent;"
		"	color: black;"
		"}"
		"QPushButton#defaultButton:!checked:hover {"
		"	border-image: none;"
		"	border: none;"
		"	border-left: 1px solid rgb(212, 212, 212);"
		"	border-right: 1px solid rgb(212, 212, 212);"
		"	background: white;"
		"}"
		"QPushButton#defaultButton:checked {"
		"	border-image: none;"
		"	border: none;"
		"	border-left: 1px solid rgb(212, 212, 212);"
		"	border-right: 1px solid rgb(212, 212, 212);"
		"	background: white;"
		"}"
		"QToolButton#addFavoriteToolButton, QToolButton#addGroupToolButton {"
		"	font-size: 9pt;"
		"}"
		));
}

void EmotionDialog::hideEvent(QHideEvent *e)
{
	if (m_cb)
	{
		m_cb->emotionClosed();
	}
	QWidget::hideEvent(e);
}

void EmotionDialog::setupUi(QWidget *dlg)
{
	dlg->setObjectName(QString::fromUtf8("EmotionPopup"));
	dlg->resize(497, 293);

	// form main panel
	QVBoxLayout *verticalLayoutMain = new QVBoxLayout(dlg);
	verticalLayoutMain->setSpacing(0);
	verticalLayoutMain->setContentsMargins(1, 1, 1, 1);
	verticalLayoutMain->setObjectName(QString::fromUtf8("verticalLayoutMain"));
	
	m_defaultTableWidget = new EmotionTableWidget(dlg);
	m_defaultTableWidget->setObjectName(QString::fromUtf8("defaultTableWidget"));
	m_defaultTableWidget->setColumnCount(kInitDefaultColCount);
	m_defaultTableWidget->setRowCount(kInitDefaultRowCount);
	m_defaultTableWidget->setShowGrid(true);
	m_defaultTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
	m_defaultTableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
	m_defaultTableWidget->verticalHeader()->setVisible(false);
	m_defaultTableWidget->horizontalHeader()->setVisible(false);
	m_defaultTableWidget->setVerticalScrollMode(QAbstractItemView::ScrollPerItem);
	m_defaultTableWidget->setHorizontalScrollMode(QAbstractItemView::ScrollPerItem);
	m_defaultTableWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
	m_defaultTableWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	m_defaultTableWidget->setFocusPolicy(Qt::NoFocus);

	m_stackedWidget = new QStackedWidget(dlg);
	m_stackedWidget->setObjectName(QString::fromUtf8("stackedWidget"));
	m_stackedWidget->addWidget(m_defaultTableWidget);
	m_stackedWidget->setCurrentIndex(0);

	// form bottom bar
	QWidget *bottomBar = new QWidget(dlg);
	bottomBar->setObjectName(QString::fromUtf8("bottomBar"));
	bottomBar->setFixedHeight(37);
	bottomBar->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(bottomBar, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(customContextMenu(QPoint)));
	QHBoxLayout *bottomBarLayout = new QHBoxLayout(bottomBar);
	bottomBarLayout->setSpacing(1);
	bottomBarLayout->setContentsMargins(15, 0, 15, 0);

	m_defaultButton = new QPushButton(dlg);
	m_defaultButton->setObjectName(QString::fromUtf8("defaultButton"));
	m_defaultButton->setFixedHeight(36);
	m_defaultButton->setFixedWidth(45);
	m_defaultButton->setCheckable(true);
	m_defaultButton->setChecked(true);
	m_defaultButton->setIcon(QIcon(":/images/Icon_136.png"));
	m_defaultButton->setIconSize(QSize(24, 24));
	m_defaultButton->setToolTip(tr("Default"));
	m_defaultButton->setContextMenuPolicy(Qt::CustomContextMenu);

	m_favoriteButtonLayout = new QHBoxLayout();
	m_favoriteButtonLayout->setSpacing(1);
	m_favoriteButtonLayout->setContentsMargins(0, 0, 0, 0);
	
	m_addFavoriteToolButton = new QToolButton(dlg);
	m_addFavoriteToolButton->setObjectName(QString::fromUtf8("addFavoriteToolButton"));
	m_addFavoriteToolButton->setText(tr("Add Emotion"));

	m_addGroupToolButton = new QToolButton(dlg);
	m_addGroupToolButton->setObjectName(QString::fromUtf8("addGroupToolButton"));
	m_addGroupToolButton->setText(tr("Add Group"));

	bottomBarLayout->addWidget(m_defaultButton);
	bottomBarLayout->addLayout(m_favoriteButtonLayout);
	bottomBarLayout->addStretch();
	bottomBarLayout->addWidget(m_addGroupToolButton);
	bottomBarLayout->addWidget(m_addFavoriteToolButton);
	m_addGroupToolButton->setVisible(false);

	verticalLayoutMain->addWidget(m_stackedWidget);
	verticalLayoutMain->addWidget(bottomBar);

	retranslateUi(dlg);

	QMetaObject::connectSlotsByName(dlg);
}

void EmotionDialog::retranslateUi(QWidget *dlg)
{
	dlg->setWindowTitle(QCoreApplication::translate("EmotionPopup", "EmotionPopup", 0));
}

void EmotionDialog::configCurrentGroup()
{
	if (m_defaultButton->isChecked())
	{
		m_stackedWidget->setCurrentIndex(0);
		return;
	}

	for (int i = 0; i < m_favoriteButtons.count(); ++i)
	{
		QPushButton *favoriteButton = m_favoriteButtons[i];
		if (favoriteButton->isChecked())
		{
			m_stackedWidget->setCurrentIndex(i+1);
			return;
		}
	}
}

void EmotionDialog::on_defaultButton_toggled(bool checked)
{
	if (checked)
	{
		m_stackedWidget->setCurrentIndex(0);

		foreach (QPushButton *favoriteButton, m_favoriteButtons)
		{
			favoriteButton->setChecked(false);
		}
	}
}

void EmotionDialog::onFavoriteButtonToggled(bool checked)
{
	if (checked)
	{
		QPushButton *senderButton = qobject_cast<QPushButton *>(sender());
		if (!senderButton)
			return;

		int index = m_favoriteButtons.indexOf(senderButton);
		if (index < 0)
			return;

		m_stackedWidget->setCurrentIndex(index+1);

		m_defaultButton->setChecked(false);
		foreach (QPushButton *favoriteButton, m_favoriteButtons)
		{
			if (favoriteButton != senderButton)
			{
				favoriteButton->setChecked(false);
			}
		}
	}
}

void EmotionDialog::onDefaultButtonContextMenu(const QPoint & /*pt*/)
{
	QPushButton *defaultButton = qobject_cast<QPushButton *>(sender());
	if (!defaultButton)
		return;

	if (defaultButton != m_defaultButton)
		return;

	QMenu menu;
	menu.addAction(m_addGroupAction);

	qApp->postEvent(m_defaultButton, new QEvent(QEvent::HoverLeave));

	menu.exec(QCursor::pos());
}

void EmotionDialog::customContextMenu(const QPoint &/*pt*/)
{
	QPoint pt = m_addFavoriteToolButton->mapToGlobal(QPoint(0, 0));
	QRect rect = QRect(pt, m_addFavoriteToolButton->size());
	if (!rect.contains(QCursor::pos()))
	{
		QMenu menu;
		menu.addAction(m_addGroupAction);
		menu.exec(QCursor::pos());
	}
}

void EmotionDialog::onFavoriteButtonContextMenu(const QPoint & /*pt*/)
{
	QPushButton *favoriteButton = qobject_cast<QPushButton *>(sender());
	if (!favoriteButton)
		return;

	QString groupId = m_favoriteGroupIds.value(favoriteButton, QString());
	if (groupId.isEmpty())
		return;

	m_addGroupAction->setData(groupId);
	m_renameGroupAction->setData(groupId);
	m_delGroupAction->setData(groupId);
	m_moveGroupBeforeAction->setData(groupId);
	m_moveGroupAfterAction->setData(groupId);
	m_moveGroupToTopAction->setData(groupId);
	
	int index = m_favoriteButtons.indexOf(favoriteButton);
	if (index == 0)
	{
		m_moveGroupBeforeAction->setEnabled(false);
		m_moveGroupToTopAction->setEnabled(false);
	}
	else
	{
		m_moveGroupBeforeAction->setEnabled(true);
		m_moveGroupToTopAction->setEnabled(true);
	}
	if (index == m_favoriteButtons.count()-1)
		m_moveGroupAfterAction->setEnabled(false);
	else
		m_moveGroupAfterAction->setEnabled(true);

	QMenu menu;
	menu.addAction(m_addGroupAction);
	menu.addAction(m_renameGroupAction);
	menu.addAction(m_delGroupAction);
	menu.addAction(m_moveGroupToTopAction);
	menu.addAction(m_moveGroupBeforeAction);
	menu.addAction(m_moveGroupAfterAction);

	qApp->postEvent(favoriteButton, new QEvent(QEvent::HoverLeave));

	menu.exec(QCursor::pos());
}

void EmotionDialog::onDefaultTableWidgetClicked(const QModelIndex &index)
{
	if (!index.isValid())
		return;

	if (m_defaultTableWidget->leftButtonClickedIndex() != index)
		return;

	int row = index.row();
	int column = index.column();
	EmotionItem *emotionItem = qobject_cast<EmotionItem *>(m_defaultTableWidget->cellWidget(row, column));
	if (!emotionItem)
		return;

	QString emotionId = emotionItem->emotionId();
	if (emotionId.isEmpty())
		return;

	if (m_cb)
	{
		m_cb->onEmotionSelected(true, emotionId);
	}

	this->hide();
}

void EmotionDialog::addFavoriteEmotion()
{
	QWidget *parentWidget = this;
	if (m_cb)
		parentWidget = m_cb->instanceWindow();

	// get image file
	QString sDir = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
	// self settings
	AccountSettings *accountSettings = Account::settings();
	if (accountSettings)
		sDir = accountSettings->getCurDir();

	// filter
	QString filterTag = tr("Image Files");
	wchar_t szFilter[256];
	memset(szFilter, 0, sizeof(szFilter));
	int len = filterTag.toWCharArray(szFilter);
	wchar_t szFilterSuffix[] = L"(*.bmp;*.jpg;*.jpeg;*.gif;*.png)\0*.BMP;*.JPG;*.JPEG;*.GIF;*.PNG\0\0";
	memcpy((char *)(szFilter+len), (char *)szFilterSuffix, sizeof(szFilterSuffix));
	
	QString filePath = FileDialog::getOpenFileName(parentWidget, tr("Choose Image"), sDir, szFilter);

	// insert image
	if (!filePath.isEmpty())
	{
		EmotionUtil::instance().addFavoriteEmotion(filePath, parentWidget);

		// self settings
		QFileInfo fi(filePath);
		if (accountSettings)
			accountSettings->setCurDir(fi.absoluteDir().absolutePath());
	}
	this->show();
}

void EmotionDialog::setCallback(EmotionCallback *cb)
{
	if (m_cb != cb)
	{
		if (m_cb)
		{
			disconnect(m_cb->instance(), SIGNAL(destroyed(QObject*)), this, SLOT(onCBDestroy(QObject*)));
		}
		m_cb = cb;
		connect(m_cb->instance(), SIGNAL(destroyed(QObject*)), this, SLOT(onCBDestroy(QObject*)), Qt::UniqueConnection);
	}
}

EmotionCallback *EmotionDialog::emotionCallback() const
{
	return m_cb;
}

void EmotionDialog::selectFavoriteEmotion(const QString &emotionId, const QString &groupId)
{
	Q_UNUSED(groupId);
	if (emotionId.isEmpty())
		return;

	if (m_cb)
	{
		m_cb->onEmotionSelected(false, emotionId);
	}

	this->hide();
}

void EmotionDialog::setCurrentGroup(const QString &groupId)
{
	QPushButton *favoriteButton = m_favoriteGroupIds.key(groupId, 0);
	if (!favoriteButton)
		return;

	favoriteButton->setChecked(true);
}

void EmotionDialog::checkHasEmotion()
{
	if (EmotionUtil::instance().hasFavoriteEmotion())
	{
		foreach (FavoriteEmotionPanel *panel, m_favoritePanels.values())
		{
			panel->setPage(FavoriteEmotionPanel::EmotionPage);
		}
	}
	else
	{
		foreach (FavoriteEmotionPanel *panel, m_favoritePanels.values())
		{
			panel->setPage(FavoriteEmotionPanel::EmptyPage);
		}
	}
}

void EmotionDialog::delFavoriteGroup(const QString &groupId)
{
	if (groupId.isEmpty())
		return;

	QPushButton *favoriteButton = m_favoriteGroupIds.key(groupId, 0);
	if (!favoriteButton)
		return;

	FavoriteEmotionPanel *panel = m_favoritePanels.value(groupId, 0);
	if (!panel)
		return;

	int index = m_favoriteButtons.indexOf(favoriteButton);
	m_favoriteButtons.removeAt(index);
	m_favoriteButtonLayout->removeWidget(favoriteButton);
	m_favoriteGroupIds.remove(favoriteButton);
	delete favoriteButton;
	favoriteButton = 0;

	m_stackedWidget->removeWidget(panel);
	m_favoritePanels.remove(groupId);
	delete panel;
	panel = 0;

	int panelIndex = m_stackedWidget->currentIndex();
	if (panelIndex == 0)
	{
		m_defaultButton->setChecked(true);
	}
	else
	{
		m_favoriteButtons[panelIndex-1]->setChecked(true);
	}
}

void EmotionDialog::setFavoriteGroupName(const QString &oldGroupId, const QString &newGroupId, const QString &groupName)
{
	if (oldGroupId.isEmpty() || newGroupId.isEmpty() || groupName.isEmpty())
		return;

	QPushButton *favoriteButton = m_favoriteGroupIds.key(oldGroupId, 0);
	if (!favoriteButton)
		return;

	FavoriteEmotionPanel *panel = m_favoritePanels.value(oldGroupId, 0);
	if (!panel)
		return;

	favoriteButton->setToolTip(groupName);
	panel->setGroupId(newGroupId);
	panel->setGroupName(groupName);

	if (oldGroupId != newGroupId)
	{
		if (oldGroupId == QString::fromLatin1(kFirstGroupId) && 0 != nextFavoriteButtonIndex())
		{
			favoriteButton->setText(QString::number(nextFavoriteButtonIndex()));
		}
		else if (newGroupId == QString::fromLatin1(kFirstGroupId))
		{
			favoriteButton->setText("");
		}

		m_favoriteGroupIds[favoriteButton] = newGroupId;
		m_favoritePanels.remove(oldGroupId);
		m_favoritePanels.insert(newGroupId, panel);
	}
}

void EmotionDialog::moveFavoriteGroupBefore(const QString &groupId)
{
	if (groupId.isEmpty())
		return;

	QPushButton *favoriteButton = m_favoriteGroupIds.key(groupId, 0);
	if (!favoriteButton)
		return;

	FavoriteEmotionPanel *panel = m_favoritePanels.value(groupId, 0);
	if (!panel)
		return;

	int index = m_favoriteButtons.indexOf(favoriteButton);
	if (index == 0)
		return;

	m_favoriteButtons.removeAt(index);
	m_favoriteButtons.insert(index-1, favoriteButton);
	m_favoriteButtonLayout->removeWidget(favoriteButton);
	m_favoriteButtonLayout->insertWidget(index-1, favoriteButton);

	m_stackedWidget->removeWidget(panel);
	m_stackedWidget->insertWidget(index, panel);

	configCurrentGroup();
}

void EmotionDialog::moveFavoriteGroupToTop(const QString &groupId)
{
	if (groupId.isEmpty())
		return;

	QPushButton *favoriteButton = m_favoriteGroupIds.key(groupId, 0);
	if (!favoriteButton)
		return;

	FavoriteEmotionPanel *panel = m_favoritePanels.value(groupId);
	if (!panel)
		return;

	int index = m_favoriteButtons.indexOf(favoriteButton);
	if (index == 0)
		return;

	m_favoriteButtons.removeAt(index);
	m_favoriteButtons.insert(0, favoriteButton);
	m_favoriteButtonLayout->removeWidget(favoriteButton);
	m_favoriteButtonLayout->insertWidget(0, favoriteButton);

	m_stackedWidget->removeWidget(panel);
	m_stackedWidget->insertWidget(1, panel);

	configCurrentGroup();
}

void EmotionDialog::moveFavoriteGroupAfter(const QString &groupId)
{
	if (groupId.isEmpty())
		return;

	QPushButton *favoriteButton = m_favoriteGroupIds.key(groupId, 0);
	if (!favoriteButton)
		return;

	FavoriteEmotionPanel *panel = m_favoritePanels.value(groupId, 0);
	if (!panel)
		return;

	int index = m_favoriteButtons.indexOf(favoriteButton);
	if (index == m_favoriteButtons.count()-1)
		return;

	m_favoriteButtons.removeAt(index);
	m_favoriteButtons.insert(index+1, favoriteButton);
	m_favoriteButtonLayout->removeWidget(favoriteButton);
	m_favoriteButtonLayout->insertWidget(index+1, favoriteButton);

	m_stackedWidget->removeWidget(panel);
	m_stackedWidget->insertWidget(index+2, panel);

	configCurrentGroup();
}

int EmotionDialog::nextFavoriteButtonIndex() const
{
	for (int i = 0; i < kMaxEmotionGroupCount; ++i)
	{
		bool found = false;
		foreach (QPushButton *favoriteButton, m_favoriteButtons)
		{
			QString buttonText = favoriteButton->text();
			if (!buttonText.isEmpty())
			{
				if (buttonText.toInt() == i)
				{
					found = true;
					break;
				}
			}
			else if (0 == i)
			{
				found = true;
				break;
			}
		}

		if (!found)
			return i;
	}

	return kMaxEmotionGroupCount;
}

int EmotionDialog::currentFavoriteGroupIndex() const
{
	return (m_stackedWidget->currentIndex() > 0? m_stackedWidget->currentIndex()- 1 : 0);
}

void EmotionDialog::onCBDestroy(QObject *obj)
{
	if (m_cb->instance() == obj)
	{
		m_cb = 0;
	}
}

void EmotionDialog::addGroup()
{
	QWidget *parent = m_cb ? m_cb->instanceWindow() : 0;
	QStringList groupIds;
	QStringList groupNames;
	EmotionUtil::instance().getFavoriteGroups(groupIds, groupNames);
	if (groupIds.count() >= kMaxEmotionGroupCount)
	{
		PMessageBox msgBox(PMessageBox::Information, tr("At most %1 groups").arg(kMaxEmotionGroupCount), 
			QDialogButtonBox::Ok, tr("Emotion Management"), this);
		msgBox.setWindowModality(Qt::WindowModal);
		msgBox.exec();
		return;
	}

	PlainTextLineInput groupInput(parent);
	groupInput.setWindowModality(Qt::WindowModal);
	groupInput.init(tr("Add New Group"), tr("Group Name:"), kMaxEmotionGroupNameSize, PlainTextLineInput::ModeUnicode);
	if (QDialog::Rejected == groupInput.exec())
		return;

	QString groupName = groupInput.getInputText().trimmed();
	if (groupName.isEmpty())
		return;

	if (groupNames.contains(groupName))
	{
		PMessageBox msgBox(PMessageBox::Information, tr("Group name exists"), QDialogButtonBox::Ok, tr("Emotion Management"), parent);
		msgBox.setWindowModality(Qt::WindowModal);
		msgBox.exec();
		return;
	}

	if (EmotionUtil::instance().addFavoriteGroup(groupName))
	{
		QPushButton *addedFavoriteButton = m_favoriteButtons.last();
		addedFavoriteButton->setChecked(true);
		this->show();
	}
}

void EmotionDialog::renameGroup()
{
	QAction *renameAction = qobject_cast<QAction *>(sender());
	if (!renameAction)
		return;

	if (renameAction != m_renameGroupAction)
		return;

	QString groupId = renameAction->data().toString();
	if (groupId.isEmpty())
		return;

	QString oldGroupName = EmotionUtil::instance().favoriteGroupName(groupId);
	QWidget *parent = m_cb ? m_cb->instanceWindow() : 0;
	PlainTextLineInput groupInput(parent);
	groupInput.setWindowModality(Qt::WindowModal);
	groupInput.init(tr("Rename"), tr("Group Name:"), kMaxEmotionGroupNameSize, PlainTextLineInput::ModeUnicode, oldGroupName);
	if (QDialog::Rejected == groupInput.exec())
		return;

	QString groupName = groupInput.getInputText().trimmed();
	if (groupName.isEmpty())
		return;

	QStringList groupIds;
	QStringList groupNames;
	EmotionUtil::instance().getFavoriteGroups(groupIds, groupNames);
	if (groupNames.contains(groupName))
	{
		PMessageBox msgBox(PMessageBox::Information, tr("Group name exists"), QDialogButtonBox::Ok, tr("Emotion Management"), parent);
		msgBox.setWindowModality(Qt::WindowModal);
		msgBox.exec();
		return;
	}

	if (EmotionUtil::instance().setFavoriteGroupName(groupId, groupName))
	{
		this->show();
	}
}

void EmotionDialog::delGroup()
{
	QAction *delAction = qobject_cast<QAction *>(sender());
	if (!delAction)
		return;

	if (delAction != m_delGroupAction)
		return;

	QString groupId = delAction->data().toString();
	if (groupId.isEmpty())
		return;

	if (EmotionUtil::instance().hasFavoriteEmotionInGroup(groupId))
	{
		QWidget *parent = m_cb ? m_cb->instanceWindow() : 0;
		DelFavoriteGroupDialog delDialog(groupId, parent);
		delDialog.setWindowModality(Qt::WindowModal);
		if (!delDialog.exec())
			return;

		DelFavoriteGroupDialog::DeleteAction op = delDialog.deleteAction();
		if (op == DelFavoriteGroupDialog::DeleteMove)
		{
			QString toGroupId = delDialog.moveGroupId();
			EmotionUtil::instance().moveFavoriteEmotionGroup(groupId, toGroupId);
		}
	}

	if (EmotionUtil::instance().delFavoriteGroup(groupId))
	{
		this->show();
	}
}

void EmotionDialog::moveGroupBefore()
{
	QAction *moveAction = qobject_cast<QAction *>(sender());
	if (!moveAction)
		return;

	if (moveAction != m_moveGroupBeforeAction)
		return;

	QString groupId = moveAction->data().toString();
	if (groupId.isEmpty())
		return;

	EmotionUtil::instance().moveFavoriteGroupBefore(groupId);
}

void EmotionDialog::moveGroupAfter()
{
	QAction *moveAction = qobject_cast<QAction *>(sender());
	if (!moveAction)
		return;

	if (moveAction != m_moveGroupAfterAction)
		return;

	QString groupId = moveAction->data().toString();
	if (groupId.isEmpty())
		return;

	EmotionUtil::instance().moveFavoriteGroupAfter(groupId);
}

void EmotionDialog::moveGroupToTop()
{
	QAction *moveAction = qobject_cast<QAction *>(sender());
	if (!moveAction)
		return;

	if (moveAction != m_moveGroupToTopAction)
		return;

	QString groupId = moveAction->data().toString();
	if (groupId.isEmpty())
		return;
	
	EmotionUtil::instance().moveFavoriteGroupToTop(groupId);
}

//////////////////////////////////////////////////////////////////////////
// MEMBER FUNCTIONS OF EmotionUtil
EmotionUtil& EmotionUtil::instance()
{
	static EmotionUtil ins;
	return ins;
}

void EmotionUtil::showEmotion(EmotionCallback *cb, const QPoint &pos)
{
	if (!m_favoriteLoaded)
	{
		this->loadFavoriteEmotions();
	}

	m_pPopup->setCallback(cb);

	QPoint newPos(0, 0);
	newPos.setY(pos.y() - m_pPopup->height() - 4);
	newPos.setX(pos.x() - m_pPopup->width()/2);

	newPos.setX(qMax(2, newPos.x()));
	newPos.setX(qMin(newPos.x(), QApplication::desktop()->width() - m_pPopup->width() - 2));
	newPos.setY(qMax(2, newPos.y()));
	m_pPopup->move(newPos);

	m_pPopup->show();
}

bool EmotionUtil::isEmotionShown() const
{
	return m_pPopup->isVisible();
}

void EmotionUtil::closeEmotion()
{
	m_pPopup->hide();
}

QString EmotionUtil::favoriteEmotionFilePath(const QString &emotionId) const
{
	FavoriteEmotionSettings::EmotionItem emotion = m_pFavoriteEmotionSettings->emotion(emotionId);
	if (emotion.isValid())
	{
		QDir emotionDir = Account::instance()->emotionDir();
		QString emotionPath = emotionDir.absoluteFilePath(emotion.fileName);
		return emotionPath;
	}

	return QString();
}

QString EmotionUtil::favoriteEmotionName(const QString &emotionId) const
{
	FavoriteEmotionSettings::EmotionItem emotion = m_pFavoriteEmotionSettings->emotion(emotionId);
	if (emotion.isValid())
	{
		return emotion.shortcut;
	}

	return QString();
}

bool EmotionUtil::setFavoriteEmotionName(const QString &emotionId, const QString &emotionName)
{
	if (emotionId.isEmpty())
		return false;

	if (!m_pFavoriteEmotionSettings->renameFavoriteEmotion(emotionId, emotionName))
		return false;

	return true;
}

bool EmotionUtil::addFavoriteEmotion(const QString &filePath, QWidget *parent)
{
	if (filePath.isEmpty())
		return false;

	QString groupId;
	QString emotionName;
	QString emotionFileName;
	QString emotionFilePath;
	QString md5Hex;
	bool needCopy = true;
	int currentGroupIndex = m_pPopup->currentFavoriteGroupIndex();

	// check file size and get md5
	{
		const int kMaxEmotionSize = (3*1024*1024);
		QFile file(filePath);
		int fileSize = file.size();
		if (fileSize > kMaxEmotionSize)
		{
			PMessageBox msgBox(PMessageBox::Information, QObject::tr("Emotion file size is no more than 3MB"), 
				QDialogButtonBox::Ok, QObject::tr("Add Failed"), parent);
			msgBox.setWindowModality(Qt::WindowModal);
			msgBox.exec();
			return false;
		}

		if (!file.open(QFile::ReadOnly))
		{
			PMessageBox msgBox(PMessageBox::Information, QObject::tr("Open emotion failed, please check if emotion file is valid"), 
				QDialogButtonBox::Ok, QObject::tr("Add Failed"), parent);
			msgBox.setWindowModality(Qt::WindowModal);
			msgBox.exec();
			return false;
		}

		// check if a valid image
		QMovie *imageMovie = new QMovie(&file);
		bool validMovie = imageMovie->isValid();
		delete imageMovie;
		imageMovie = 0;

		QByteArray content = file.readAll();

		file.close();

		// get file name
		emotionFileName = QString("%1.%2").arg(QDateTime::currentDateTime().toString("yyyy_MM_dd_hh_mm_ss_zzz"))
			.arg(QFileInfo(filePath).suffix());
		emotionFilePath = Account::instance()->emotionDir().absoluteFilePath(emotionFileName);

		// check image
		if (!validMovie)
		{
			QImage image;
			if (image.loadFromData(content))
			{
				if (image.save(emotionFilePath))
				{
					qDebug() << Q_FUNC_INFO << "save to another location: " << emotionFilePath;
					needCopy = false;
				}
			}

			if (needCopy)
			{
				PMessageBox msgBox(PMessageBox::Information, QObject::tr("Can't read this file, please check if it is a valid image file"), 
					QDialogButtonBox::Ok, QObject::tr("Add Failed"), parent);
				msgBox.setWindowModality(Qt::WindowModal);
				msgBox.exec();
				return false;
			}
		}

		// get hex
		QByteArray raw = QCryptographicHash::hash(content, QCryptographicHash::Md5);
		md5Hex = raw.toHex().toLower();
	}

	// save emotion
	if (needCopy)
	{
		if (!QFile::copy(filePath, emotionFilePath))
		{
			PMessageBox msgBox(PMessageBox::Information, QObject::tr("Save emotion file failed"), 
				QDialogButtonBox::Ok, QObject::tr("Add Failed"), parent);
			msgBox.setWindowModality(Qt::WindowModal);
			msgBox.exec();
			return false;
		}
	}

	bool confirmAdd = false;
	{
		// get emotion name
		AddFavoriteEmotionDialog addDlg(emotionFilePath, currentGroupIndex, parent);
		addDlg.setWindowModality(Qt::WindowModal);
		if (addDlg.exec())
		{
			emotionName = addDlg.emotionName();
			groupId = addDlg.emotionGroupId();
			confirmAdd = true;
		}
	}

	if (!confirmAdd)
	{
		if (!QFile::remove(emotionFilePath))
		{
			qDebug() << Q_FUNC_INFO << "file can't removed: " << emotionFilePath;
		}
		return false;
	}

	// check if duplicate
	if (m_pFavoriteEmotionSettings->hasDuplicateEmotion(md5Hex))
	{
		if (!QFile::remove(emotionFilePath))
		{
			qDebug() << Q_FUNC_INFO << "file can't removed: " << emotionFilePath;
		}

		PMessageBox msgBox(PMessageBox::Information, QObject::tr("This emotion exists in my favorite"), 
			QDialogButtonBox::Ok, QObject::tr("Add Failed"), parent);
		msgBox.setWindowModality(Qt::WindowModal);
		msgBox.exec();
		return false;
	}

	// add to settings
	m_pFavoriteEmotionSettings->addFavoriteEmotion(emotionFileName, emotionName, md5Hex, groupId);

	// reload favorite emotions
	loadFavoriteEmotions(groupId);

	// switch to added group
	m_pPopup->setCurrentGroup(groupId);

	// check if has emotion
	m_pPopup->checkHasEmotion();

	return true;
}

bool EmotionUtil::delFavoriteEmotion(const QString &emotionId, const QString &groupId)
{
	if (emotionId.isEmpty() || groupId.isEmpty())
		return false;

	QString emotionFilePath = this->favoriteEmotionFilePath(emotionId);
	if (!m_pFavoriteEmotionSettings->delFavoriteEmotion(emotionId, groupId))
		return false;
	
	loadFavoriteEmotions(groupId);

	/* // do not remove file, because the message uses this emotion if this message was sended out 
	QFile::remove(emotionFilePath);
	*/

	// check if has emotion
	m_pPopup->checkHasEmotion();

	return true;
}

bool EmotionUtil::moveFirstFavoriteEmotion(const QString &emotionId, const QString &groupId)
{
	if (emotionId.isEmpty() || groupId.isEmpty())
		return false;

	if (!m_pFavoriteEmotionSettings->moveFirstFavoriteEmotion(emotionId, groupId))
		return false;

	loadFavoriteEmotions(groupId);

	return true;
}

bool EmotionUtil::moveFavoriteEmotionToOtherGroup(const QString &emotionId, const QString &fromGroupId, const QString &toGroupId)
{
	if (emotionId.isEmpty() || fromGroupId.isEmpty() || toGroupId.isEmpty())
		return false;

	if (fromGroupId == toGroupId)
		return true;

	if (!m_pFavoriteEmotionSettings->moveFavoriteEmotionNewGroup(fromGroupId, emotionId, toGroupId))
		return false;

	loadFavoriteEmotions(fromGroupId);
	loadFavoriteEmotions(toGroupId);
	return true;
}

bool EmotionUtil::moveFavoriteEmotionGroup(const QString &fromGroupId, const QString &toGroupId)
{
	if (fromGroupId.isEmpty() || toGroupId.isEmpty())
		return false;

	if (fromGroupId == toGroupId)
		return true;

	if (!m_pFavoriteEmotionSettings->moveFavoriteEmotionGroup(fromGroupId, toGroupId))
		return false;

	loadFavoriteEmotions(fromGroupId);
	loadFavoriteEmotions(toGroupId);
	return true;
}

void EmotionUtil::getFavoriteGroups(QStringList &groupIds, QStringList &groupNames) const
{
	QList<FavoriteEmotionSettings::EmotionGroup> emotionGroups = m_pFavoriteEmotionSettings->allGroups();
	QMap<QString, QString> groups;
	foreach (FavoriteEmotionSettings::EmotionGroup groupItem, emotionGroups)
	{
		groupIds.append(groupItem.groupId);
		groupNames.append(groupItem.groupName);
	}
}

bool EmotionUtil::addFavoriteGroup(const QString &groupName)
{
	QString groupId = m_pFavoriteEmotionSettings->addGroup(groupName);
	if (groupId.isEmpty())
		return false;

	m_pPopup->addFavoriteGroup(groupId, groupName);
	return true;
}

bool EmotionUtil::delFavoriteGroup(const QString &groupId)
{
	if (!m_pFavoriteEmotionSettings->delGroup(groupId))
		return false;

	m_pPopup->delFavoriteGroup(groupId);
	return true;
}

QString EmotionUtil::favoriteGroupName(const QString &groupId) const
{
	return m_pFavoriteEmotionSettings->groupName(groupId);
}

bool EmotionUtil::setFavoriteGroupName(const QString &groupId, const QString &groupName)
{
	QString newGroupId;
	if (!m_pFavoriteEmotionSettings->setGroupName(groupId, groupName, newGroupId))
		return false;

	m_pPopup->setFavoriteGroupName(groupId, newGroupId, groupName);
	return true;
}

bool EmotionUtil::moveFavoriteGroupBefore(const QString &groupId)
{
	if (!m_pFavoriteEmotionSettings->moveGroupBefore(groupId))
		return false;

	m_pPopup->moveFavoriteGroupBefore(groupId);
	return true;
}

bool EmotionUtil::moveFavoriteGroupAfter(const QString &groupId)
{
	if (!m_pFavoriteEmotionSettings->moveGroupAfter(groupId))
		return false;

	m_pPopup->moveFavoriteGroupAfter(groupId);
	return true;
}

bool EmotionUtil::moveFavoriteGroupToTop(const QString &groupId)
{
	if (!m_pFavoriteEmotionSettings->moveGroupToTop(groupId))
	{
		return false;
	}

	m_pPopup->moveFavoriteGroupToTop(groupId);
	return true;
}

bool EmotionUtil::hasFavoriteEmotionInGroup(const QString &groupId)
{
	QList<FavoriteEmotionSettings::EmotionItem> emotions = m_pFavoriteEmotionSettings->allEmotions(groupId);
	return (!emotions.isEmpty());
}

bool EmotionUtil::hasFavoriteEmotion() const
{
	return m_pFavoriteEmotionSettings->hasEmotion();
}

EmotionUtil::EmotionUtil() : m_favoriteLoaded(false)
{
}

void EmotionUtil::init()
{
	m_pPopup.reset(new EmotionDialog());//注销之后考虑到中英文切换，此处不判断m_pPopup是否为空而直接重新创建表情窗口。
	loadDefaultEmotions();

	QDir settingsDir = Account::instance()->emotionDir();
	QString settingsFilePath = settingsDir.absoluteFilePath("favorite.conf");
	m_pFavoriteEmotionSettings.reset(new FavoriteEmotionSettings(settingsFilePath));

	m_pPopup->resetFavoriteGroups();
	QList<FavoriteEmotionSettings::EmotionGroup> favoriteGroups = m_pFavoriteEmotionSettings->allGroups();
	foreach (FavoriteEmotionSettings::EmotionGroup groupItem, favoriteGroups)
	{
		m_pPopup->addFavoriteGroup(groupItem.groupId, groupItem.groupName);
	}

	m_favoriteLoaded = false;
}

void EmotionUtil::uninit()
{
	m_favoriteLoaded = false;
	m_pFavoriteEmotionSettings.reset(0);
}

void EmotionUtil::loadDefaultEmotions()
{
	// load all default emotion names from resource file
	QFile file(QString::fromLatin1(":/face/default_emotions.txt"));
	file.open(QFile::ReadOnly);
	QByteArray content = file.readAll();
	file.close();
	QString allEmotions = QString::fromUtf8(content);
	m_faceNames = allEmotions.split(QChar(','));

	//load all default emotion codes from resource file
	file.setFileName(QString::fromLatin1(":/face/default_emojicodenames.txt"));
	file.open(QFile::ReadOnly);
	QByteArray emojiCodes = file.readAll();
	file.close();
	QString allEmojiCodes = QString::fromUtf8(emojiCodes);
	m_faceCodeNames = allEmojiCodes.split(QChar(','));

	// get all file path
	QString faceFileName;
	for (int i = 0; i < m_faceNames.count(); ++i)
	{
		faceFileName = QString(":/face/%1.png").arg(((int)(i+1)), 2, 10, QChar('0'));
		m_faceFileNames.append(faceFileName);
	}
/*	m_pPopup->setDefaultEmotions(m_faceNames, m_faceFileNames);*/
	if (GlobalSettings::Language_ZH_CN == GlobalSettings::language())
	{
		m_pPopup->setDefaultEmotions(m_faceNames, m_faceFileNames);
	}
	else
	{
		m_pPopup->setDefaultEmotions(m_faceCodeNames, m_faceFileNames);
	}
}

void EmotionUtil::loadFavoriteEmotions()
{
	QList<FavoriteEmotionSettings::EmotionGroup> groups = m_pFavoriteEmotionSettings->allGroups();
	foreach (FavoriteEmotionSettings::EmotionGroup groupItem, groups)
	{
		loadFavoriteEmotions(groupItem.groupId);
	}
	m_favoriteLoaded = true;
}

void EmotionUtil::loadFavoriteEmotions(const QString &groupId)
{
	// add favorite emotions
	QList<FavoriteEmotionSettings::EmotionItem> favoriteEmotions = m_pFavoriteEmotionSettings->allEmotions(groupId);
	QStringList delEmotionIds;
	QStringList favoriteEmotionIds;
	QStringList favoriteEmotionNames;
	QStringList favoriteEmotionFileNames;
	QDir emotionDir = Account::instance()->emotionDir();
	foreach (FavoriteEmotionSettings::EmotionItem favoriteEmotion, favoriteEmotions)
	{
		QString emotionFilePath = emotionDir.absoluteFilePath(favoriteEmotion.fileName);
		if (QFile::exists(emotionFilePath))
		{
			favoriteEmotionIds.append(favoriteEmotion.emotionId);
			favoriteEmotionNames.append(favoriteEmotion.shortcut);
			favoriteEmotionFileNames.append(emotionFilePath);
		}
		else
		{
			delEmotionIds.append(favoriteEmotion.emotionId);
		}
	}
	m_pPopup->setFavoriteEmotions(groupId, favoriteEmotionIds, favoriteEmotionNames, favoriteEmotionFileNames);

	// delete not exist emotions
	foreach (QString emotionId, delEmotionIds)
	{
		m_pFavoriteEmotionSettings->delFavoriteEmotion(emotionId, groupId);
	}
}

#include "EmotionUtil.moc"