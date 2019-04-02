#include "delfavoritegroupdialog.h"
#include "ui_delfavoritegroupdialog.h"
#include "EmotionUtil.h"

DelFavoriteGroupDialog::DelFavoriteGroupDialog(const QString &groupId, QWidget *parent)
	: FramelessDialog(parent), m_groupId(groupId)
{
	ui = new Ui::DelFavoriteGroupDialog();
	ui->setupUi(this);

	setWindowIcon(qApp->windowIcon());

	setWindowTitle(ui->title->text());

	setMainLayout(ui->verticalLayoutMain);
	setFixedSize(306, 176);
	setResizeable(false);

	initUI();

	connect(ui->btnClose, SIGNAL(clicked()), this, SLOT(reject()));
	connect(ui->btnOK, SIGNAL(clicked()), this, SLOT(accept()));
	connect(ui->btnCancel, SIGNAL(clicked()), this, SLOT(reject()));

	setSkin();
}

DelFavoriteGroupDialog::~DelFavoriteGroupDialog()
{
	delete ui;
}

DelFavoriteGroupDialog::DeleteAction DelFavoriteGroupDialog::deleteAction() const
{
	if (ui->radioButtonDeleteAll->isChecked())
		return DeleteAll;
	else
		return DeleteMove;
}

QString DelFavoriteGroupDialog::moveGroupId() const
{
	if (ui->comboBoxGroup->count() > 0)
		return ui->comboBoxGroup->itemData(ui->comboBoxGroup->currentIndex()).toString();
	return QString();
}

void DelFavoriteGroupDialog::setSkin()
{
	// set background image
	QPixmap bgPixmap(":/theme/dialog_bgs/dialog_bg_9.png");
	FramelessDialog::BGSizes bgSizes = {0};
	bgSizes.borderwidth = 5;
	bgSizes.topBarHeight = 33;
	bgSizes.bottomBarHeight = 33;
	setBG(bgPixmap, bgSizes);

	// set title label style
	ui->title->setStyleSheet("QLabel {font-size: 12pt; color: white;}");

	// bottom bar button style
	QFile qssFile;
	qssFile.setFileName(":/theme/qss/pushbutton2_skin.qss");
	if (qssFile.open(QIODevice::ReadOnly))
	{
		QString qss = qssFile.readAll();
		ui->btnOK->setStyleSheet(qss);
		ui->btnCancel->setStyleSheet(qss);
		qssFile.close();
	}
}

void DelFavoriteGroupDialog::on_radioButtonDeleteAll_toggled(bool checked)
{
	if (checked)
	{
		ui->comboBoxGroup->setEnabled(false);
	}
	else
	{
		if (ui->comboBoxGroup->count() > 0)
			ui->comboBoxGroup->setEnabled(true);
		else
			ui->comboBoxGroup->setEnabled(false);
	}
}

void DelFavoriteGroupDialog::initUI()
{
	ui->comboBoxGroup->clear();

	QStringList groupIds;
	QStringList groupNames;
	EmotionUtil::instance().getFavoriteGroups(groupIds, groupNames);
	QString groupId;
	QString groupName;
	for (int i = 0; i < groupIds.count(); ++i)
	{
		groupId = groupIds[i];
		groupName = groupNames[i];
		if (groupId != m_groupId)
		{
			ui->comboBoxGroup->addItem(groupName, groupId);
		}
	}

	if (ui->comboBoxGroup->count() > 0)
	{
		ui->radioButtonDeleteAll->setChecked(false);
		ui->radioButtonDeleteMove->setChecked(true);
		ui->comboBoxGroup->setCurrentIndex(0);
	}
	else
	{
		ui->radioButtonDeleteAll->setChecked(true);
		ui->radioButtonDeleteMove->setEnabled(false);
		ui->comboBoxGroup->setEnabled(false);
	}
}
