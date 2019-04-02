#include "passwdlineedit.h"
//#include <QRegExpValidator>

PasswdLineEdit::PasswdLineEdit(QWidget *parent) :
    FilterLineEdit(parent)
{
    setPlaceholderText(tr("Please input password"));
    QIcon icon = QIcon::fromTheme(layoutDirection() == Qt::LeftToRight ?
                     QLatin1String("edit-clear-locationbar-rtl") :
                     QLatin1String("edit-clear-locationbar-ltr"),
                     QIcon::fromTheme(QLatin1String("edit-clear"), QIcon(QLatin1String(":/images/keyboard.png"))));

    setButtonPixmap(FilterLineEdit::Left, icon.pixmap(16));
    setButtonVisible(FilterLineEdit::Left, true);
    setButtonToolTip(FilterLineEdit::Left, tr("Keyboard"));
    setAutoHideButton(FilterLineEdit::Left, false);
    setEchoMode(QLineEdit::Password);

//    // set password validator
//    setMaxLength(15);
//    QRegExp rx("[a-zA-Z0-9]{6,15}");
//    QValidator *validator = new QRegExpValidator(rx, this);
//    setValidator(validator);
}
