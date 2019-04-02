#ifndef STYLETOOLBUTTON_H
#define STYLETOOLBUTTON_H

#include <QToolButton>
#include <QMap>
#include <QVariant>
#include "widgetkit_global.h"

class WIDGETKIT_EXPORT StyleToolButton :
        public QToolButton
{
    Q_OBJECT
public:
    explicit StyleToolButton(QWidget *parent = 0);

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

    virtual void setInfo(StyleToolButton::Info styleInfo, const StyleToolButton::State state = StyleToolButton::State1st);
    virtual void setState(StyleToolButton::State state);
	void setUserData(const QVariant &value, int role = Qt::UserRole);
	QVariant userData(int role = Qt::UserRole) const;

public slots:
    void triggerState();
    
signals:
    void stateChanged(StyleToolButton::State);

private:
    StyleToolButton::State m_currState;
    StyleToolButton::Info m_styleInfo1st;
    StyleToolButton::Info m_styleInfo2nd;
    int m_stateCount;
	QMap<int, QVariant> m_userdata;
};

#endif // STYLEPUSHBUTTON_H
