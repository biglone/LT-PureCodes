#include "widgetkit.h"
#include <QTranslator>
#include <QApplication>

static QTranslator *s_translator = 0;

widgetkit::widgetkit()
{

}

widgetkit::~widgetkit()
{

}

void widgetkit::installTranslator()
{
	if (!s_translator)
	{
		s_translator = new QTranslator();
		s_translator->load("widgetkit_zh_CN", ":/translator");
	}
	qApp->installTranslator(s_translator);
}

void widgetkit::removeTranslator()
{
	if (s_translator)
	{
		qApp->removeTranslator(s_translator);
	}
}
