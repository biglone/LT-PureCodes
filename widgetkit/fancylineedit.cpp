#include "fancylineedit.h"

#include <QtCore/QEvent>
#include <QtCore/QDebug>
#include <QtCore/QString>

#include <QApplication>
#include <QLabel>
#include <QMenu>
#include <QMouseEvent>
#include <QPainter>
#include <QPaintEvent>
#include <QStyle>
#include "iconbutton.h"

enum { margin = 6 };

class FancyLineEditPrivate : public QObject
{
public:
    explicit FancyLineEditPrivate(FancyLineEdit *parent);

    virtual bool eventFilter(QObject *obj, QEvent *event);

public:
    FancyLineEdit  *m_lineEdit;
    QPixmap m_pixmap[2];
    QMenu *m_menu[2];
    bool m_menuTabFocusTrigger[2];
    IconButton *m_iconbutton[2];
    bool m_iconEnabled[2];
    QString m_oldText;
	QString m_toolTip[2];
};


FancyLineEditPrivate::FancyLineEditPrivate(FancyLineEdit *parent) :
    QObject(parent),
    m_lineEdit(parent)
{
    for (int i = 0; i < 2; ++i) {
        m_menu[i] = 0;
        m_menuTabFocusTrigger[i] = false;
        m_iconbutton[i] = new IconButton(parent);
        m_iconbutton[i]->installEventFilter(this);
        m_iconbutton[i]->hide();
        m_iconbutton[i]->setAutoHide(false);
        m_iconEnabled[i] = false;
		m_toolTip[i] = "";
    }
}

bool FancyLineEditPrivate::eventFilter(QObject *obj, QEvent *event)
{
    int buttonIndex = -1;

    for (int i = 0; i < 2; ++i) {
        if (obj == m_iconbutton[i]) {
            buttonIndex = i;
            break;
        }
    }

    if (buttonIndex == -1)
        return QObject::eventFilter(obj, event);

    switch (event->type()) {
    case QEvent::FocusIn:
        if (m_menuTabFocusTrigger[buttonIndex] && m_menu[buttonIndex]) {
            m_lineEdit->setFocus();
            m_menu[buttonIndex]->exec(m_iconbutton[buttonIndex]->mapToGlobal(
                    m_iconbutton[buttonIndex]->rect().center()));
            return true;
        }
	case QEvent::ToolTip:
		if (m_iconbutton[buttonIndex]->iconOpacity() < 1.0)
		{
			m_iconbutton[buttonIndex]->setToolTip("");
		}
		else
		{
			m_iconbutton[buttonIndex]->setToolTip(m_toolTip[buttonIndex]);
		}
		return false;
    default:
        break;
    }

    return QObject::eventFilter(obj, event);
}

/*!
    \class FancyLineEdit

    \brief A line edit with an embedded pixmap on one side that is connected to
    a menu.

    Additionally, it can display a grayed hintText (like "Type Here to")
    when not focused and empty. When connecting to the changed signals and
    querying text, one has to be aware that the text is set to that hint
    text if isShowingHintText() returns true (that is, does not contain
    valid user input).
*/
FancyLineEdit::FancyLineEdit(QWidget *parent) :
    QLineEdit(parent),
    d(new FancyLineEditPrivate(this))
{
    ensurePolished();
    updateMargins();

    connect(this, SIGNAL(textChanged(QString)), this, SLOT(checkButtons(QString)));
    connect(d->m_iconbutton[Left], SIGNAL(clicked()), this, SLOT(iconClicked()));
    connect(d->m_iconbutton[Right], SIGNAL(clicked()), this, SLOT(iconClicked()));
}

FancyLineEdit::~FancyLineEdit()
{
}

QMenu *FancyLineEdit::buttonMenu(Side side) const
{
    return  d->m_menu[side];
}

void FancyLineEdit::setButtonMenu(Side side, QMenu *buttonMenu)
{
     d->m_menu[side] = buttonMenu;
     d->m_iconbutton[side]->setIconOpacity(1.0);
}

QPixmap FancyLineEdit::buttonPixmap(Side side) const
{
    return d->m_pixmap[side];
}

void FancyLineEdit::setButtonPixmap(Side side, const QPixmap &buttonPixmap)
{
    d->m_iconbutton[side]->setPixmap(buttonPixmap);
    updateMargins();
    updateButtonPositions();
    update();
}

bool FancyLineEdit::isButtonVisible(Side side) const
{
    return d->m_iconEnabled[side];
}

void FancyLineEdit::setButtonVisible(Side side, bool visible)
{
    d->m_iconbutton[side]->setVisible(visible);
    d->m_iconEnabled[side] = visible;
    updateMargins();
}

void FancyLineEdit::setButtonToolTip(Side side, const QString &tip)
{
	d->m_toolTip[side] = tip;
}

void FancyLineEdit::setButtonFocusPolicy(Side side, Qt::FocusPolicy policy)
{
    d->m_iconbutton[side]->setFocusPolicy(policy);
}

/*!
    Sets whether tabbing in will trigger the menu.
*/
bool FancyLineEdit::hasMenuTabFocusTrigger(Side side) const
{
    return d->m_menuTabFocusTrigger[side];
}

void FancyLineEdit::setMenuTabFocusTrigger(Side side, bool v)
{
    if (d->m_menuTabFocusTrigger[side] == v)
        return;

    d->m_menuTabFocusTrigger[side] = v;
    d->m_iconbutton[side]->setFocusPolicy(v ? Qt::TabFocus : Qt::NoFocus);
}

bool FancyLineEdit::hasAutoHideButton(Side side) const
{
    return d->m_iconbutton[side]->hasAutoHide();
}

/*!
    Set if icon should be hidden when text is empty
*/
void FancyLineEdit::setAutoHideButton(Side side, bool h)
{
    d->m_iconbutton[side]->setAutoHide(h);
    if (h)
        d->m_iconbutton[side]->setIconOpacity(text().isEmpty() ?  0.0 : 1.0);
    else
        d->m_iconbutton[side]->setIconOpacity(1.0);
}

void FancyLineEdit::resizeEvent(QResizeEvent *)
{
    updateButtonPositions();
}

void FancyLineEdit::focusInEvent(QFocusEvent *e)
{
	QLineEdit::focusInEvent(e);

	emit gainFocus();
}

void FancyLineEdit::focusOutEvent(QFocusEvent *e)
{
	emit loseFocus();

	QLineEdit::focusOutEvent(e);
}

void FancyLineEdit::checkButtons(const QString &text)
{
    if (d->m_oldText.isEmpty() || text.isEmpty()) {
        for (int i = 0; i < 2; ++i) {
            if (d->m_iconbutton[i]->hasAutoHide())
                d->m_iconbutton[i]->animateShow(!text.isEmpty());
        }
        d->m_oldText = text;
    }
}

void FancyLineEdit::iconClicked()
{
    IconButton *button = qobject_cast<IconButton *>(sender());
    int index = -1;
    for (int i = 0; i < 2; ++i)
        if (d->m_iconbutton[i] == button)
            index = i;
    if (index == -1)
        return;
    if (d->m_menu[index]) {
        d->m_menu[index]->exec(QCursor::pos());
    } else {
        emit buttonClicked((Side)index);
        if (index == Left)
            emit leftButtonClicked();
        else if (index == Right)
            emit rightButtonClicked();
    }
}

void FancyLineEdit::updateMargins()
{
    bool leftToRight = (layoutDirection() == Qt::LeftToRight);
    Side realLeft = (leftToRight ? Left : Right);
    Side realRight = (leftToRight ? Right : Left);

    int leftMargin = d->m_iconbutton[realLeft]->pixmap().width() + 8;
    int rightMargin = d->m_iconbutton[realRight]->pixmap().width() + 8;
    // Note KDE does not reserve space for the highlight color
    if (style()->inherits("OxygenStyle")) {
        leftMargin = qMax(24, leftMargin);
        rightMargin = qMax(24, rightMargin);
    }

    QMargins margins((d->m_iconEnabled[realLeft] ? leftMargin : 0), 0,
                     (d->m_iconEnabled[realRight] ? rightMargin : 0), 0);

    setTextMargins(margins);
}

void FancyLineEdit::updateButtonPositions()
{
    QRect contentRect = rect();
    for (int i = 0; i < 2; ++i) {
        Side iconpos = (Side)i;
        if (layoutDirection() == Qt::RightToLeft)
            iconpos = (iconpos == Left ? Right : Left);

        if (iconpos == FancyLineEdit::Right) {
            const int iconoffset = textMargins().right() + 4;
            d->m_iconbutton[i]->setGeometry(contentRect.adjusted(width() - iconoffset, 0, 0, 0));
        } else {
            const int iconoffset = textMargins().left() + 4;
            d->m_iconbutton[i]->setGeometry(contentRect.adjusted(0, 0, -width() + iconoffset, 0));
        }
    }
}
