#include "selectionwidget.h"

#include <QVBoxLayout>
#include <QHBoxLayout>

SelectionWidget::SelectionWidget(QWidget *parent, const char * name)
:QWidget(parent)
{
	QPalette p = palette ();
	p.setBrush (QPalette::Background, QBrush(Qt::yellow));
	setPalette (p);

	QVBoxLayout *vLayout = new QVBoxLayout(this);
	vLayout->setSpacing (0);
	vLayout->setMargin (2);

	label = new QLabel(this);
	label->setText (tr("<empty>"));
	label->setVisible (false);

	QWidget *hWidget = new QWidget(this);
	QHBoxLayout *hLayout = new QHBoxLayout(hWidget);
	hLayout->setSpacing (0);
	hLayout->setMargin (1);

	onTrueWidget = new QWidget(hWidget);
	onFalseWidget = new QWidget(hWidget);

	hLayout->addWidget (onTrueWidget);
	hLayout->addWidget (onFalseWidget);

	vLayout->addWidget (label);
	vLayout->addWidget (hWidget);
}

void SelectionWidget::showEvent (QShowEvent *e)
{
	Q_UNUSED(e);
	label->setVisible (true);
}

