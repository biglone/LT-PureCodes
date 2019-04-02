#include "styletestdialog.h"
#include "ui_styletestdialog.h"
#include <QMenu>
#include <QCursor>

StyleTestDialog *StyleTestDialog::instance()
{
	static StyleTestDialog *dlg = new StyleTestDialog();
	return dlg;
}

StyleTestDialog::StyleTestDialog(QWidget *parent)
	: QDialog(parent)
{
	ui = new Ui::StyleTestDialog();
	ui->setupUi(this);

	initUI();
	initStyle();
}

StyleTestDialog::~StyleTestDialog()
{
	delete ui;
}

void StyleTestDialog::initUI()
{
	QString lineEditText = tr("这是一个行输入框");
	ui->lineEditNormal->setText(lineEditText);
	ui->lineEditReadonly->setText(lineEditText);
	ui->lineEditDisabled->setText(lineEditText);

	QString textEditText = tr("以下是测试文字：PM PC客户端代码理解及整体结构重构；"
		"对于PM PC客户端缺失及未完善的功能添加与完善;"
		"新增名片、系统设置、聊天字体、窗口抖动等新功能;"
		"PM PC客户端整体UI风格设计及修改。");
	ui->textEditNormal->setText(textEditText);
	ui->textEditReadonly->setText(textEditText);
	ui->textEditDisabled->setText(textEditText);

	ui->textBrowserNormal->setText(textEditText);
	ui->textBrowserDisabled->setText(textEditText);

	ui->comboBoxEditable->addItem(tr("公网环境"));
	ui->comboBoxEditable->addItem(tr("测试环境"));
	ui->comboBoxEditable->addItem(tr("自定义环境"));
	ui->comboBoxEditable->setCurrentIndex(0);

	ui->comboBoxReadonly->addItem(tr("公网环境"));
	ui->comboBoxReadonly->addItem(tr("测试环境"));
	ui->comboBoxReadonly->addItem(tr("自定义环境"));
	ui->comboBoxReadonly->setCurrentIndex(0);

	ui->comboBoxDisabled->addItem(tr("公网环境"));
	ui->comboBoxDisabled->addItem(tr("测试环境"));
	ui->comboBoxDisabled->addItem(tr("自定义环境"));
	ui->comboBoxDisabled->setCurrentIndex(0);
}

void StyleTestDialog::initStyle()
{
	QString strStyleSheet;
	strStyleSheet += QString(
			"QPushButton{"
			"padding-right: 9px;"
			"padding-left: 9px;"
			"padding-top: 2px;"
			"padding-bottom: 2px;"
			"border-image: url(./theme/pushbutton/pushbutton_normal.png);"
			"border-top: 2px;"
			"border-left: 2px;"
			"border-right: 2px;"
			"border-bottom: 2px;"
			"color: #333333;"
			"}"
			"QPushButton:hover:!pressed{"
			"border-image: url(./theme/pushbutton/pushbutton_hover.png);"
			"}"
			"QPushButton:hover:pressed{"
			"border-image: url(./theme/pushbutton/pushbutton_pressed.png);"
			"}"
			"QPushButton:disabled{"
			"color: #666666;"
			"border-image: url(./theme/pushbutton/pushbutton_disabled.png);"
			"}"
			);

	strStyleSheet += QString(
		"QLineEdit QTextEdit QTextBrowser {"
		"padding-right: 2px;"
		"padding-left: 2px;"
		"padding-top: 2px;"
		"padding-bottom: 2px;"
		"border-image: url(./theme/lineedit/lineedit_normal.png);"
		"border-top: 2px;"
		"border-left: 2px;"
		"border-right: 2px;"
		"border-bottom: 2px;"
		"color: #333333;"
		"}"
		"QLineEdit:hover:!read-only:!disabled{"
		"border-image: url(./theme/lineedit/lineedit_hover.png);"
		"}"
		"QLineEdit:focus:!read-only:!disabled{"
		"border-image: url(./theme/lineedit/lineedit_pressed.png);"
		"}"
		"QLineEdit:disabled{"
		"border-image: url(./theme/lineedit/lineedit_disabled.png);"
		"color: #666666;"
		"}"
		"QLineEdit:read-only{"
		"border-image: url(./theme/lineedit/lineedit_readonly.png);"
		"color: #444444;"
		"}"
		);

	strStyleSheet += QString(
		"QCheckBox {"
		"spacing: 5px;"
		"color: #333333;"
		"}"
		"QCheckBox::indicator {"
		"width: 17px;"
		"height: 17px;"
		"}"
		"QCheckBox::indicator:unchecked {"
		"image: url(./theme/checkbox/checkbox_unchecked_normal.png);"
		"}"
		"QCheckBox::indicator:unchecked:hover {"
		"image: url(./theme/checkbox/checkbox_unchecked_hover.png);"
		"}"
		"QCheckBox::indicator:unchecked:pressed {"
		"image: url(./theme/checkbox/checkbox_unchecked_pressed.png);"
		"}"
		"QCheckBox::indicator:unchecked:disabled {"
		"image: url(./theme/checkbox/checkbox_unchecked_disabled.png);"
		"}"
		"QCheckBox::indicator:checked {"
		"image: url(./theme/checkbox/checkbox_checked_normal.png);"
		"}"
		"QCheckBox::indicator:checked:hover {"
		"image: url(./theme/checkbox/checkbox_checked_hover.png);"
		"}"
		"QCheckBox::indicator:checked:pressed {"
		"image: url(./theme/checkbox/checkbox_checked_pressed.png);"
		"}"
		"QCheckBox::indicator:checked:disabled {"
		"image: url(./theme/checkbox/checkbox_checked_disabled.png);"
		"}"
		"QCheckBox:disabled {"
		"color: #666666;"
		"}"
		);

	strStyleSheet += QString(
		"QRadioButton {"
		"spacing: 5px;"
		"color: #333333;"
		"border: transparent;"
		"}"
		"QRadioButton::indicator {"
		"width: 17px;"
		"height: 17px;"
		"}"
		"QRadioButton::indicator:unchecked {"
		"image: url(./theme/radiobutton/radiobutton_unchecked_normal.png);"
		"}"
		"QRadioButton::indicator:unchecked:hover {"
		"image: url(./theme/radiobutton/radiobutton_unchecked_hover.png);"
		"}"
		"QRadioButton::indicator:unchecked:pressed {"
		"image: url(./theme/radiobutton/radiobutton_unchecked_pressed.png);"
		"}"
		"QRadioButton::indicator:unchecked:disabled {"
		"image: url(./theme/radiobutton/radiobutton_unchecked_disabled.png);"
		"}"
		"QRadioButton::indicator:checked {"
		"image: url(./theme/radiobutton/radiobutton_checked_normal.png);"
		"}"
		"QRadioButton::indicator:checked:hover {"
		"image: url(./theme/radiobutton/radiobutton_checked_hover.png);"
		"}"
		"QRadioButton::indicator:checked:pressed {"
		"image: url(./theme/radiobutton/radiobutton_checked_pressed.png);"
		"}"
		"QRadioButton::indicator:checked:disabled {"
		"image: url(./theme/radiobutton/radiobutton_checked_disabled.png);"
		"}"
		"QRadioButton:disabled {"
		"color: #666666;"
		"}"
		);

	strStyleSheet += QString(
		"QProgressBar{"
		"border-image: url(./theme/progressbar/progressbar_normal.png);"
		"border-top: 1px;"
		"border-left: 1px;"
		"border-right: 1px;"
		"border-bottom: 1px;"
		"height: 8px;"
		"color: #333333;"
		"}"
		"QProgressBar::chunk {"
		"background-color: rgb(64, 149, 212);"
		"width: 5px;"
		"height: 8px;"
		"}"
		);

	strStyleSheet += QString(
		"QToolButton{"
		"border-top: 2px;"
		"border-left: 2px;"
		"border-right: 2px;"
		"border-bottom: 2px;"
		"color: #333333;"
		"border-image: url(./theme/toolbutton/toolbutton_unchecked_normal.png);"
		"}"
		"QToolButton:!checked:hover:!pressed{"
		"border-image: url(./theme/toolbutton/toolbutton_unchecked_hover.png);"
		"}"
		"QToolButton:!checked:hover:pressed{"
		"border-image: url(./theme/toolbutton/toolbutton_unchecked_pressed.png);"
		"}"
		"QToolButton:checked{"
		"border-image: url(./theme/toolbutton/toolbutton_checked_normal.png);"
		"}"
		"QToolButton:checked:hover:!pressed{"
		"border-image: url(./theme/toolbutton/toolbutton_checked_hover.png);"
		"}"
		"QToolButton:checked:hover:pressed{"
		"border-image: url(./theme/toolbutton/toolbutton_checked_pressed.png);"
		"}"
		"QToolButton:disabled{"
		"color: #666666;"
		"}"
		"QToolButton:checked:disabled{"
		"border-image: url(./theme/toolbutton/);"
		"}"
	);

	strStyleSheet += QString(
		"QComboBox{"
		"padding-top: 2px;"
		"padding-left: 2px;"
		"padding-right: 2px;"
		"padding-bottom: 2px;"
		"border-image: url(./theme/combobox/combobox_normal.png);"
		"border-top: 2px;"
		"border-left: 2px;"
		"border-right: 2px;"
		"border-bottom: 2px;"
		"color: #333333;"
		"}"
		"QComboBox:hover:!read-only:!disabled{"
		"border-image: url(./theme/combobox/combobox_hover.png);"
		"}"
		"QComboBox:focus:!read-only:!disabled{"
		"border-image: url(./theme/combobox/combobox_pressed.png);"
		"}"
		"QComboBox:disabled{"
		"border-image: url(./theme/combobox/combobox_disabled.png);"
		"color: #666666;"
		"}"
		"QComboBox:!editable{"
		"border-image: url(./theme/combobox/combobox_readonly.png);"
		"color: #444444;"
		"}"
		"QComboBox::drop-down {"
		"width: 27px;"
		"border-image: url(./theme/combobox/combobox_drop_down_normal.png);"
		"}"
		"QComboBox::drop-down:hover {"
		"border-image: url(./theme/combobox/combobox_drop_down_hover.png);"
		"}"
		"QComboBox::drop-down:pressed {"
		"border-image: url(./theme/combobox/combobox_drop_down_pressed.png);"
		"}"
		"QComboBox::drop-down:disabled {"
		"border-image: url(./theme/combobox/combobox_drop_down_disabled.png);"
		"}"
		);

	strStyleSheet += QString(
		"QMenu {"
		"margin-top: 0px;"
		"margin-left: 0px;"
		"margin-right: 0px;"
		"margin-bottom: 0px;"
		"background-image: url(./theme/menu/menu_background.png);"
		"border-radius: 2px;"
		"border-top: 1px solid rgb(126, 148, 150);"
		"border-left: 1px solid rgb(126, 148, 150);"
		"border-right: 1px solid rgb(126, 148, 150);"
		"border-bottom: 1px solid rgb(126, 148, 150);"
		"}"

		"QMenu::item {"
		"color: rgb(95, 95, 95);"
		"padding-top: 4px;"
		"padding-left: 30px;"
		"padding-right: 20px;"
		"padding-bottom: 4px;"
		"}"

		"QMenu::item:selected:!disabled {"
		"border-image: url(./theme/menu/menu_item_background.png);"
		"border-top: 0px;"
		"border-left: 0px;"
		"border-right: 0px;"
		"border-bottom: 0px;"
		"color: rgb(254, 254, 254);"
		"}"
		"QMenu::item:disabled {"
		"color: rgb(186, 186, 186);"
		"}"

		"QMenu::icon {"
		"padding-top: 6px;"
		"padding-left: 4px;"
		"padding-right: 6px;"
		"padding-bottom: 8px;"
		"}"

		"QMenu::separator {"
		"height: 1px;"
		"border-image: url(./theme/menu/menu_separator_background.png);"
		"border-top: 0px;"
		"border-left: 0px;"
		"border-right: 0px;"
		"border-bottom: 0px;"
		"padding-left: 38px;"
		"padding-right: 2px;"
		"padding-top: 0px;"
		"padding-bottom: 0px;"
		"}"
		"QMenu::indicator {"
		"width: 13px;"
		"height: 30px;"
		"}"
		);

	strStyleSheet += QString(
		"QScrollBar::vertical {"
		"margin-top: 15px;"
		"margin-left: 0px;"
		"margin-right: 0px;"
		"margin-bottom: 15px;"
		"border-image: url(./theme/scrollbar/scrollbar_vertical_normal.png);"
		"border-top: 1px;"
		"border-left: 1px;"
		"border-right: 1px;"
		"border-bottom: 1px;"
		"width: 15px;"
		"}"
		);

	strStyleSheet += QString("QScrollBar::handle:vertical {"
		"border-image: url(./theme/scrollbar/scrollbar_vertical_handle_normal.png);"
		"border-top: 1px;"
		"border-left: 1px;"
		"border-right: 1px;"
		"border-bottom: 1px;"
		"min-height: 20px;"
		"}"
		"QScrollBar::handle:vertical:hover {"
		"border-image: url(./theme/scrollbar/scrollbar_vertical_handle_hover.png);"
		"}"
		"QScrollBar::handle:vertical:pressed {"
		"border-image: url(./theme/scrollbar/scrollbar_vertical_handle_pressed.png);"
		"}"
		);

	strStyleSheet += QString("QScrollBar::sub-line:vertical {"
		"border-image: url(./theme/scrollbar/scrollbar_vertical_subline_normal.png);"
		"border-top: 0px;"
		"border-left: 0px;"
		"border-right: 0px;"
		"border-bottom: 0px;"
		"height: 15px;"
		"subcontrol-position: top;"
		"subcontrol-origin: margin;"
		"}"
		"QScrollBar::sub-line:vertical:hover {"
		"border-image: url(./theme/scrollbar/scrollbar_vertical_subline_hover.png);"
		"}"
		"QScrollBar::sub-line:vertical:pressed {"
		"border-image: url(./theme/scrollbar/scrollbar_vertical_subline_pressed.png);"
		"}"
		);

	strStyleSheet += QString("QScrollBar::add-line:vertical {"
		"border-image: url(./theme/scrollbar/scrollbar_vertical_addline_normal.png);"
		"border-top: 0px;"
		"border-left: 0px;"
		"border-right: 0px;"
		"border-bottom: 0px;"
		"height: 15px;"
		"subcontrol-position: bottom;"
		"subcontrol-origin: margin;"
		"}"
		"QScrollBar::add-line:vertical:hover {"
		"border-image: url(./theme/scrollbar/scrollbar_vertical_addline_hover.png);"
		"}"
		"QScrollBar::add-line:vertical:pressed {"
		"border-image: url(./theme/scrollbar/scrollbar_vertical_addline_pressed.png);"
		"}"
		);

	strStyleSheet += QString(
		"QScrollBar::horizontal {"
		"margin-top: 0px;"
		"margin-left: 15px;"
		"margin-right: 15px;"
		"margin-bottom: 0px;"
		"border-image: url(./theme/scrollbar/scrollbar_horizontal_normal.png);"
		"border-top: 1px;"
		"border-left: 1px;"
		"border-right: 1px;"
		"border-bottom: 1px;"
		"height: 15px;"
		"}"
		);

	strStyleSheet += QString("QScrollBar::handle:horizontal {"
		"border-image: url(./theme/scrollbar/scrollbar_horizontal_handle_normal.png);"
		"border-top: 1px;"
		"border-left: 1px;"
		"border-right: 1px;"
		"border-bottom: 1px;"
		"min-width: 20px;"
		"}"
		"QScrollBar::handle:horizontal:hover {"
		"border-image: url(./theme/scrollbar/scrollbar_horizontal_handle_hover.png);"
		"}"
		"QScrollBar::handle:horizontal:pressed {"
		"border-image: url(./theme/scrollbar/scrollbar_horizontal_handle_pressed.png);"
		"}"
		);

	strStyleSheet += QString("QScrollBar::sub-line:horizontal {"
		"border-image: url(./theme/scrollbar/scrollbar_horizontal_subline_normal.png);"
		"border-top: 0px;"
		"border-left: 0px;"
		"border-right: 0px;"
		"border-bottom: 0px;"
		"width: 15px;"
		"subcontrol-position: left;"
		"subcontrol-origin: margin;"
		"}"
		"QScrollBar::sub-line:horizontal:hover {"
		"border-image: url(./theme/scrollbar/scrollbar_horizontal_subline_hover.png);"
		"}"
		"QScrollBar::sub-line:horizontal:pressed {"
		"border-image: url(./theme/scrollbar/scrollbar_horizontal_subline_pressed.png);"
		"}"
		);

	strStyleSheet += QString("QScrollBar::add-line:horizontal {"
		"border-image: url(./theme/scrollbar/scrollbar_horizontal_addline_normal.png);"
		"border-top: 0px;"
		"border-left: 0px;"
		"border-right: 0px;"
		"border-bottom: %0px;"
		"width: 15px;"
		"subcontrol-position: right;"
		"subcontrol-origin: margin;"
		"}"
		"QScrollBar::add-line:horizontal:hover {"
		"border-image: url(./theme/scrollbar/scrollbar_horizontal_addline_hover.png);"
		"}"
		"QScrollBar::add-line:horizontal:pressed {"
		"border-image: url(./theme/scrollbar/scrollbar_horizontal_addline_pressed.png);"
		"}"
		);

	this->setStyleSheet(strStyleSheet);
}

void StyleTestDialog::on_toolButtonMenu_clicked()
{
	QMenu menu(this);
	menu.addAction(tr("公网环境"));
	menu.addAction(tr("测试环境"));
	menu.addSeparator();
	QMenu *subMenu = menu.addMenu(tr("自定义环境"));
	subMenu->addAction(tr("58_.211.187.150:28801"));
	subMenu->addAction(tr("58_.211.187.150:28701"));
	subMenu->addAction(tr("127.0.0.1:28801"));
	menu.addAction(tr("默认环境"));
	menu.exec(QCursor::pos());
}
