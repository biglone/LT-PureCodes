#include "filterlineedit.h"
#include "KeyDelegate.h"
#include <QKeyEvent>

/*!
    \class FilterLineEdit

    \brief A fancy line edit customized for filtering purposes with a clear button.
*/

FilterLineEdit::FilterLineEdit(QWidget *parent) :
   FancyLineEdit(parent),
   m_lastFilterText(text()),
   m_keyDelegate(0)
{
    // KDE has custom icons for this. Notice that icon namings are counter intuitive.
    // If these icons are not available we use the freedesktop standard name before
    // falling back to a bundled resource.
    QIcon icon = QIcon::fromTheme(layoutDirection() == Qt::LeftToRight ?
                     QLatin1String("edit-clear-locationbar-rtl") :
                     QLatin1String("edit-clear-locationbar-ltr"),
                     QIcon::fromTheme(QLatin1String("edit-clear"), QIcon(QLatin1String(":/images/editclear.png"))));

    setButtonPixmap(Right, icon.pixmap(16));
    setButtonVisible(Right, true);
    setPlaceholderText(tr("Filter"));
    setButtonToolTip(Right, tr("Clear"));
    setAutoHideButton(Right, true);
    connect(this, SIGNAL(rightButtonClicked()), this, SLOT(clear()));
    connect(this, SIGNAL(textChanged(QString)), this, SLOT(slotTextChanged()));
}

void FilterLineEdit::setKeyDelegate(KeyDelegate *keyDelegate)
{
	m_keyDelegate = keyDelegate;
}

void FilterLineEdit::keyPressEvent(QKeyEvent *e) 
{  
	if (m_keyDelegate) 
	{  
		int key = e->key();  
		if (Qt::Key_Down == key) 
		{
			m_keyDelegate->downKeyPressed();
		} 
		else if (Qt::Key_Up == key) 
		{  
			m_keyDelegate->upKeyPressed();
		}
		else if (Qt::Key_Enter == key || Qt::Key_Return == key) 
		{  
			m_keyDelegate->returnKeyPressed();
		} 
		else 
		{  
			QLineEdit::keyPressEvent(e);  
		}  
	} 
	else 
	{  
		QLineEdit::keyPressEvent(e);  
	}  
}  

void FilterLineEdit::slotTextChanged()
{
    QString newlyTypedText = text();
	QString oldTypedText = m_lastFilterText;
    if (newlyTypedText != m_lastFilterText) 
	{
        m_lastFilterText = newlyTypedText;
       
		emit filterChanged(m_lastFilterText);

		emit filterChanged(newlyTypedText, oldTypedText);
    }
}
