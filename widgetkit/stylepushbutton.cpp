#include "stylepushbutton.h"

StylePushButton::StylePushButton(QWidget *parent) :
QPushButton(parent)
{
	m_currState = StylePushButton::State1st;
	m_stateCount = 1;
	connect(this, SIGNAL(clicked()), SLOT(triggerState()));
}

void StylePushButton::setInfo(StylePushButton::Info styleInfo, const StylePushButton::State state)
{
	if(state == StylePushButton::State1st)
	{
		m_styleInfo1st = styleInfo;
		if(!QPixmap(styleInfo.urlNormal).isNull())
		{
			setFixedSize(QPixmap(styleInfo.urlNormal).size());
		}
		else if(!QPixmap(styleInfo.urlHover).isNull())
		{
			setFixedSize(QPixmap(styleInfo.urlHover).size());
		}
		else if(!QPixmap(styleInfo.urlPressed).isNull())
		{
			setFixedSize(QPixmap(styleInfo.urlPressed).size());
		}

		QString styleSheet = QString("QPushButton{"
			"border: none;"
			"background: transparent;"
			"border-image: url(%1);"
			"}"
			"QPushButton:pressed{"
			"border-image: url(%2);"
			"}"
			"QPushButton:hover:!pressed{"
			"border-image: url(%3);"
			"}")
			.arg(styleInfo.urlNormal)
			.arg(styleInfo.urlPressed)
			.arg(styleInfo.urlHover);
		if (!styleInfo.urlDisabled.isEmpty())
		{
			styleSheet.append(QString(
				"QPushButton:disabled{"
				"border-image: url(%1);"
				"}")
				.arg(styleInfo.urlDisabled));
		}
		setStyleSheet(styleSheet);
		setToolTip(styleInfo.tooltip);
	}
	else if(state == StylePushButton::State2nd)
	{
		m_stateCount = 2;
		m_styleInfo2nd = styleInfo;
	}
}

void StylePushButton::setState(StylePushButton::State state)
{
	Q_ASSERT(!(state == StylePushButton::State2nd && m_stateCount == 1));
	if(state == m_currState)
		return;

	QString styleSheet;
	if(state == StylePushButton::State1st)
	{
		styleSheet = QString("QPushButton{"
			"border: none;"
			"background: transparent;"
			"border-image: url(%1);"
			"}"
			"QPushButton:pressed{"
			"border-image: url(%2);"
			"}"
			"QPushButton:hover:!pressed{"
			"border-image: url(%3);"
			"}")
			.arg(m_styleInfo1st.urlNormal)
			.arg(m_styleInfo1st.urlPressed)
			.arg(m_styleInfo1st.urlHover);
		if (!m_styleInfo1st.urlDisabled.isEmpty())
		{
			styleSheet.append(QString(
				"QPushButton:disabled{"
				"border-image: url(%1);"
				"}"
				).arg(m_styleInfo1st.urlDisabled));
		}
		setStyleSheet(styleSheet);
		setToolTip(m_styleInfo1st.tooltip);
	}
	else if(state == StylePushButton::State2nd)
	{
		styleSheet = QString("QPushButton{"
			"border: none;"
			"background: transparent;"
			"border-image: url(%1);"
			"}"
			"QPushButton:pressed{"
			"border-image: url(%2);"
			"}"
			"QPushButton:hover:!pressed{"
			"border-image: url(%3);"
			"}")
			.arg(m_styleInfo2nd.urlNormal)
			.arg(m_styleInfo2nd.urlPressed)
			.arg(m_styleInfo2nd.urlHover);
		if (!m_styleInfo2nd.urlDisabled.isEmpty())
		{
			styleSheet.append(QString(
				"QPushButton:disabled{"
				"border-image: url(%1);"
				"}"
				).arg(m_styleInfo2nd.urlDisabled));
		}
		setStyleSheet(styleSheet);
		setToolTip(m_styleInfo2nd.tooltip);
	}

	m_currState = state;
}

void StylePushButton::setTextWithColor(const QString &text, 
                                       const QString &normalClr, 
									   const QString &hoverClr, 
									   const QString &pressedClr,
									   const QString &disabledClr)
{
	QString style = this->styleSheet();
	style += QString("QPushButton{color: %1;}").arg(normalClr);

	if (!pressedClr.isEmpty())
	{
		style += QString("QPushButton:pressed{"
			"color: %1;"
			"}").arg(pressedClr);
	}

	if (!hoverClr.isEmpty())
	{
		style += QString("QPushButton:hover:!pressed{"
			"color: %1;"
			"}").arg(hoverClr);
	}

	if (!disabledClr.isEmpty())
	{
		style += QString("QPushButton:disabled{"
			"color: %1;"
			"}").arg(disabledClr);
	}

	setText(text);
	setStyleSheet(style);
}

void StylePushButton::triggerState()
{
	if(m_stateCount == 1)
		return;
	if(m_currState == StylePushButton::State1st)
	{
		setState(StylePushButton::State2nd);
	}
	else if(m_currState == StylePushButton::State2nd)
	{
		setState(StylePushButton::State1st);
	}
	stateChanged(m_currState);
}

