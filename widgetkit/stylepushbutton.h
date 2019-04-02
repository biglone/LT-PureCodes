#ifndef STYLEPUSHBUTTON_H
#define STYLEPUSHBUTTON_H

#include <QPushButton>
#include "widgetkit_global.h"

class WIDGETKIT_EXPORT StylePushButton :
        public QPushButton
{
    Q_OBJECT
public:
    explicit StylePushButton(QWidget *parent = 0);

    enum State{
        State1st,
        State2nd
    };

    struct Info{
        QString urlNormal;
        QString urlHover;
        QString urlPressed;
		QString urlDisabled;
        QString tooltip;

		Info() {urlNormal = ""; urlHover = ""; urlPressed = ""; urlDisabled = ""; tooltip = "";}
    };

    virtual void setInfo(StylePushButton::Info styleInfo, const StylePushButton::State state = StylePushButton::State1st);
    virtual void setState(StylePushButton::State state);
	StylePushButton::State state() const {return m_currState;}

	// must call after setInfo function
	void setTextWithColor(const QString &text, 
		                  const QString &normalClr, 
						  const QString &hoverClr = QString(), 
						  const QString &pressedClr = QString(),
						  const QString &disabledClr = QString());

public slots:
    void triggerState();
    
signals:
    void stateChanged(StylePushButton::State);

private:
    StylePushButton::State m_currState;
    StylePushButton::Info m_styleInfo1st;
    StylePushButton::Info m_styleInfo2nd;
    int m_stateCount;
};

#endif // STYLEPUSHBUTTON_H
