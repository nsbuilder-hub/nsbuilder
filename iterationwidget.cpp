#include "iterationwidget.h"
#include <QVBoxLayout>

IterationWidget::IterationWidget(QWidget *parent, const char * name)
:QWidget(parent)
{
	QVBoxLayout *vLayout = new QVBoxLayout();
	QHBoxLayout *hLayout = new QHBoxLayout();
	QWidget *hWidget = new QWidget ();
	hLayout->setMargin (30);

	contentsLabel = new QLabel(this);
	instructionsWidget = new QWidget (hWidget);

	vLayout->addWidget (contentsLabel);
	hLayout->addWidget (instructionsWidget);

	hWidget->setLayout (hLayout);
	setLayout (vLayout);
}

