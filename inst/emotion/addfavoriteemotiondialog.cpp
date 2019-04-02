#include "addfavoriteemotiondialog.h"
#include "ui_addfavoriteemotiondialog.h"
#include <QMovie>
#include "plaintextlineinput.h"
#include "EmotionUtil.h"
#include "pmessagebox.h"
#include "emotionconsts.h"

AddFavoriteEmotionDialog::AddFavoriteEmotionDialog(const QString &emotionPath, const int &curGroupIndex, QWidget *parent)
	: FramelessDialog(parent)
{
	ui = new Ui::AddFavoriteEmotionDialog();
	ui->setupUi(this);

	setWindowIcon(qApp->windowIcon());

	setWindowTitle(ui->title->text());

	setMainLayout(ui->verticalLayoutMain);
	setFixedSize(364, 176);
	setResizeable(false);

	initUI(emotionPath, curGroupIndex);

	connect(ui->btnClose, SIGNAL(clicked()), this, SLOT(reject()));
	connect(ui->btnOK, SIGNAL(clicked()), this, SLOT(onAccept()));
	connect(ui->btnCancel, SIGNAL(clicked()), this, SLOT(reject()));

	setSkin();
}

AddFavoriteEmotionDialog::~AddFavoriteEmotionDialog()
{
	delete ui;
}

QString AddFavoriteEmotionDialog::emotionName() const
{
	return ui->lineEditName->text().trimmed();
}

QString AddFavoriteEmotionDialog::emotionGroupId() const
{
	return ui->comboBoxGroup->itemData(ui->comboBoxGroup->currentIndex()).toString();
}

void AddFavoriteEmotionDialog::setSkin()
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

	ui->labelEmotion->setStyleSheet("border: 1px solid rgb(177, 186, 197);");

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

void AddFavoriteEmotionDialog::addGroup()
{
	if (ui->comboBoxGroup->count() >= kMaxEmotionGroupCount)
	{
		PMessageBox msgBox(PMessageBox::Information, tr("At most %1 groups").arg(kMaxEmotionGroupCount), 
			QDialogButtonBox::Ok, tr("Emotion management"), this);
		msgBox.setWindowModality(Qt::WindowModal);
		msgBox.exec();
		return;
	}

	PlainTextLineInput groupInput(this);
	groupInput.setWindowModality(Qt::WindowModal);
	groupInput.init(tr("Add new group"), tr("Group name:"), kMaxEmotionGroupNameSize, PlainTextLineInput::ModeUnicode);
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
		PMessageBox msgBox(PMessageBox::Information, tr("Same group name exists"), 
			QDialogButtonBox::Ok, tr("Emotion management"), this);
		msgBox.setWindowModality(Qt::WindowModal);
		msgBox.exec();
		return;
	}

	EmotionUtil::instance().addFavoriteGroup(groupName);
	addComboBoxGroups();
	ui->comboBoxGroup->setCurrentIndex(ui->comboBoxGroup->count()-1);
}

void AddFavoriteEmotionDialog::onAccept()
{
	QString groupId = this->emotionGroupId();
	if (groupId.isEmpty())
	{
		PMessageBox msgBox(PMessageBox::Information, tr("Please choose a group or create a new one"), 
			QDialogButtonBox::Ok, tr("Emotion management"), this);
		msgBox.setWindowModality(Qt::WindowModal);
		msgBox.exec();
		return;
	}

	accept();
}

void AddFavoriteEmotionDialog::initUI(const QString &emotionPath, const int &curGroupIndex)
{
	ui->lineEditName->setPlaceholderText(tr("At most %1 characters").arg(kMaxEmotionNameSize));
	ui->lineEditName->setMaxLength(kMaxEmotionNameSize);

	QMovie *emotionMovie = new QMovie(this);
	emotionMovie->setFileName(emotionPath);
	ui->labelEmotion->setEmotion(FavoriteEmotion, emotionMovie);

	ui->labelAddGroup->setFontAtt(QColor(0, 120, 216), 10, false);
	connect(ui->labelAddGroup, SIGNAL(clicked()), this, SLOT(addGroup()));

	addComboBoxGroups();
	ui->comboBoxGroup->setCurrentIndex(curGroupIndex);
}

void AddFavoriteEmotionDialog::addComboBoxGroups()
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
		ui->comboBoxGroup->addItem(groupName, groupId);
	}
}
