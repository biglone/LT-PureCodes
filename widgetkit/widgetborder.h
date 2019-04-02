#ifndef WIDGETBORDER_H
#define WIDGETBORDER_H

struct WidgetBorder
{
	int top;
	int left;
	int right;
	int bottom;
	int height;
	int width;
	int radius;

	WidgetBorder()
	{
		top = 0;
		left = 0;
		right = 0;
		bottom = 0;
		height = 0;
		width = 0;
		radius = 0;
	}
};

struct StyleUrls
{
	QString normal;
	QString hover;
	QString pressed;
	QString selected;
	QString disabled;
	QString readonly;
	QString background;

	StyleUrls() {}
};

struct WidgetBorderStyle
{
	StyleUrls urls;
	WidgetBorder border;
};

#endif // WIDGETBORDER_H