#include <QtCore/QSize>
#include <QtCore/QDebug>

#include "tooltip.h"

const QSize defaultSize(140, 51);
CToolTip::CToolTip(QWidget *parent)
: QLabel(parent/*, Qt::Window*/)
{
	resize(defaultSize);
	setAlignment(Qt::AlignCenter);
}

CToolTip::~CToolTip()
{
}

void CToolTip::setText(const QString &tooltip)
{
	QLabel::setText(tooltip);
	QSize s = size();
	if (defaultSize.height() < sizeHint().height())
		s.rheight() = sizeHint().height();
	else
		s.rheight() = defaultSize.height();

	if (defaultSize.width() < sizeHint().width())
		s.rwidth() = sizeHint().width();
	else
		s.rwidth() = defaultSize.width();

	resize(s);
}

