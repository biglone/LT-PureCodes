#include "styletoolbutton.h"

StyleToolButton::StyleToolButton(QWidget *parent) :
    QToolButton(parent)
{
    m_currState = StyleToolButton::State1st;
    m_stateCount = 1;
    connect(this, SIGNAL(clicked()), SLOT(triggerState()));
}

void StyleToolButton::setInfo(StyleToolButton::Info styleInfo, const StyleToolButton::State state)
{
    if(state == StyleToolButton::State1st)
    {
        m_styleInfo1st = styleInfo;
        setFixedSize(QPixmap(styleInfo.urlNormal).size());
		setIcon(QIcon());
		setText("");
		QString styleSheet = QString("QToolButton{"
			"border: none;"
			"background: transparent;"
			"border-image: url(%1);"
			"}"
			"QToolButton:pressed{"
			"border-image: url(%2);"
			"}"
			"QToolButton:hover:!pressed{"
			"border-image: url(%3);"
			"}"
			)
			.arg(styleInfo.urlNormal)
			.arg(styleInfo.urlPressed)
			.arg(styleInfo.urlHover);
		if (!styleInfo.urlDisabled.isEmpty())
		{
			styleSheet.append(QString(
				"QToolButton:disabled{"
				"border-image: url(%1);"
				"}"
				).arg(styleInfo.urlDisabled));
		}
        setStyleSheet(styleSheet);
        setToolTip(styleInfo.tooltip);
    }
    else if(state == StyleToolButton::State2nd)
    {
        m_stateCount = 2;
        m_styleInfo2nd = styleInfo;
    }
}

void StyleToolButton::setUserData(const QVariant &value, int role /* = Qt::UserRole */)
{
	m_userdata[role] = value;
}

QVariant StyleToolButton::userData(int role /* = Qt::UserRole */) const
{
	return m_userdata.value(role);
}

void StyleToolButton::setState(StyleToolButton::State state)
{
    Q_ASSERT(!(state == StyleToolButton::State2nd && m_stateCount == 1));
    if(state == m_currState)
        return;

	QString styleSheet;
    if(state == StyleToolButton::State1st)
    {
		styleSheet = QString("QToolButton{"
			"border: none;"
			"background: transparent;"
			"border-image: url(%1);"
			"}"
			"QToolButton:pressed{"
			"border-image: url(%2);"
			"}"
			"QToolButton:hover:!pressed{"
			"border-image: url(%3);"
			"}")
			.arg(m_styleInfo1st.urlNormal)
			.arg(m_styleInfo1st.urlPressed)
			.arg(m_styleInfo1st.urlHover);

		if (!m_styleInfo1st.urlDisabled.isEmpty())
		{
			styleSheet.append(QString(
				"QToolButton:disabled{"
				"border-image: url(%1);"
				"}"
				).arg(m_styleInfo1st.urlDisabled));
		}
		setStyleSheet(styleSheet);
        setToolTip(m_styleInfo1st.tooltip);
    }
    else if(state == StyleToolButton::State2nd)
    {
        styleSheet = QString("QToolButton{"
                      "border: none;"
                      "background: transparent;"
                      "border-image: url(%1);"
                      "}"
                      "QToolButton:pressed{"
                      "border-image: url(%2);"
                      "}"
                      "QToolButton:hover:!pressed{"
                      "border-image: url(%3);"
                      "}")
                      .arg(m_styleInfo2nd.urlNormal)
                      .arg(m_styleInfo2nd.urlPressed)
                      .arg(m_styleInfo2nd.urlHover);
		if (!m_styleInfo2nd.urlDisabled.isEmpty())
		{
			styleSheet.append(QString(
				"QToolButton:disabled{"
				"border-image: url(%1);"
				"}"
				).arg(m_styleInfo2nd.urlDisabled));
		}
		setStyleSheet(styleSheet);
        setToolTip(m_styleInfo2nd.tooltip);
    }

    m_currState = state;
}

void StyleToolButton::triggerState()
{
    if(m_stateCount == 1)
        return;
    if(m_currState == StyleToolButton::State1st)
    {
        setState(StyleToolButton::State2nd);
    }
    else if(m_currState == StyleToolButton::State2nd)
    {
        setState(StyleToolButton::State1st);
    }
    stateChanged(m_currState);
}

