#include "FilterOkLineEdit.h"

FilterOkLineEdit::FilterOkLineEdit(QWidget *parent) :
   FancyLineEdit(parent),
   m_lastFilterText(text())
{
	QIcon icon = QIcon(QLatin1String(":/images/editclear.png"));
    setButtonPixmap(Right, icon.pixmap(16));
    setButtonVisible(Right, true);
    setPlaceholderText(tr("Filter"));
    setButtonToolTip(Right, tr("Clear"));
    setAutoHideButton(Right, true);
    connect(this, SIGNAL(textChanged(QString)), this, SLOT(slotTextChanged()));
}

void FilterOkLineEdit::setClearRightButton()
{
	QIcon icon = QIcon(QLatin1String(":/images/editclear.png"));
	setButtonPixmap(Right, icon.pixmap(16));
	setButtonToolTip(Right, tr("Clear"));
}

void FilterOkLineEdit::setOkRightButton()
{
	QIcon icon = QIcon(QLatin1String(":/images/editok.png"));
	setButtonPixmap(Right, icon.pixmap(16));
	setButtonToolTip(Right, tr("Chat"));
}

void FilterOkLineEdit::slotTextChanged()
{
    const QString newlyTypedText = text();
    if (newlyTypedText != m_lastFilterText) {
        m_lastFilterText = newlyTypedText;
        emit filterChanged(m_lastFilterText);
    }
}
