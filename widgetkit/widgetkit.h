#ifndef PMWIDGETKIT_H
#define PMWIDGETKIT_H

#include "widgetkit_global.h"

class WIDGETKIT_EXPORT widgetkit
{
public:
	widgetkit();
	~widgetkit();
	
	static void installTranslator(); 
	static void removeTranslator();
};

#endif // PMWIDGETKIT_H
