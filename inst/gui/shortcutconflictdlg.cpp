#include "shortcutconflictdlg.h"
#include "ui_shortcutconflictdlg.h"
#include "PmApp.h"
#include "ModelManager.h"
#include "Account.h"
#include "settings/GlobalSettings.h"

QPointer<ShortcutConflictDlg> ShortcutConflictDlg::s_instance;

ShortcutConflictDlg* ShortcutConflictDlg::instance(int failedCount)
{
	if (s_instance.isNull())
	{
		s_instance = new ShortcutConflictDlg();
	}
	s_instance.data()->setFailedCount(failedCount);
	return s_instance.data();
}

ShortcutConflictDlg::ShortcutConflictDlg(QWidget *parent)
	: FramelessDialog(parent)
{
	ui = new Ui::ShortcutConflictDlg();
	ui->setupUi(this);

	setAttribute(Qt::WA_DeleteOnClose, true);

	setWindowIcon(qApp->windowIcon());
	setWindowTitle(ui->labelTitle->text());

	setMainLayout(ui->verticalLayoutMain);
	setFixedSize(359, 190);
	setResizeable(false);

	connect(ui->btnClose, SIGNAL(clicked()), SLOT(close()));
	connect(ui->pushButtonClose, SIGNAL(clicked()), SLOT(close()));

	setSkin();
}

ShortcutConflictDlg::~ShortcutConflictDlg()
{
	delete ui;
}

void ShortcutConflictDlg::setFailedCount(int failedCount)
{
	QString id = Account::instance()->id();
	QString selfName = qPmApp->getModelManager()->userName(id);
	QString tip = tr("%1, you have %2 hot key(s) taken up. Please change to make the functions work well. Or:").arg(selfName).arg(failedCount);
	ui->labelTip->setText(tip);
}

void ShortcutConflictDlg::setSkin()
{
	// set background image
	QPixmap bgPixmap(":/theme/dialog_bgs/dialog_bg_9.png");
	FramelessDialog::BGSizes bgSizes = {0};
	bgSizes.borderwidth = 5;
	bgSizes.topBarHeight = 33;
	bgSizes.bottomBarHeight = 33;
	setBG(bgPixmap, bgSizes);

	// set title label style
	ui->labelTitle->setStyleSheet("QLabel {font-size: 12pt; color: white;}");

	// set modify label style
	ui->labelModify->setFontAtt(QColor(0, 120, 216), 10, false);

	// bottom bar button style
	QFile qssFile;
	QString btnQss;
	qssFile.setFileName(":/theme/qss/pushbutton2_skin.qss");
	if (qssFile.open(QIODevice::ReadOnly))
	{
		btnQss = qssFile.readAll();
		qssFile.close();
	}
	ui->pushButtonClose->setStyleSheet(btnQss);
}

void ShortcutConflictDlg::on_checkBoxNotRemind_toggled(bool checked)
{
	GlobalSettings::setShortcutConflictTipOn(!checked);
}

void ShortcutConflictDlg::on_labelModify_clicked()
{
	emit modifyShortcutKey();
}

