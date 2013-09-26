#include <QPainter>
#include <QPalette>
#include <QLabel>

#include "selectioncondition.h"
#include "nsscheme.h"
#include "selection.h"

SelectionCondition::SelectionCondition(Selection *parent)
:QWidget(parent)
{
	p_parent = parent;
	//setMinimumSize (6, 4);
	boundRect.setWidth (20);
	boundRect.setHeight (60);
	label = new QLabel(this);
	label->setAlignment (Qt::AlignHCenter | Qt::AlignTop);
	label->setWordWrap (true);
	label->setVisible (false);
}

void SelectionCondition::showEvent (QShowEvent *e)
{
	Q_UNUSED(e);
#ifdef DEBUG4
	qDebug("[ENTER] SelectionCondition::showEvent");
#endif
	/* musimy odrysować etykietę bo rozmiar komponentu mógł się zmienić */ 
	do_paintLabel ();
#ifdef DEBUG4
	qDebug("[EXIT] SelectionCondition::showEvent");
#endif
}

void SelectionCondition::resizeEvent (QResizeEvent *event)
{
	Q_UNUSED(event);
	do_paintLabel ();
}

void SelectionCondition::do_paintLabel ()
{
#ifdef DEBUG5
	qDebug(QString("width=%1 parent width=%2").arg(this->width ()).arg(p_parent->width ()).toLocal8Bit ());
#endif
	label->resize (this->width () - 60, this->height () - 20);
	label->setText (labelTemplate
		    .arg (m_comment.isEmpty () ? QString::null : escape (m_comment).append ("<br/>"))
		    .arg (escape (m_text)));
}

void SelectionCondition::setText (const QString &s)
{
	m_text = s;
	do_paintLabel ();
	repaint ();
}

void SelectionCondition::setComment (const QString &s)
{
	m_comment = s;
	do_paintLabel ();
	repaint ();
}
	
QSize SelectionCondition::minimumSizeHint () const
{
	return boundRect;
}

void SelectionCondition::paintEvent (QPaintEvent *e)
{
	Q_UNUSED(e);
#ifdef DEBUG4
	qDebug("[ENTER] SelectionCondition::paintEvent");
#endif

	QPainter p(this);
	p.setClipRect (e->rect ());
	p.fillRect (1, 0, width () - 2, height (), p_parent->backgroundColor ());

	QPalette pal = label->palette ();
	pal.setBrush (QPalette::Background, p_parent->backgroundColor ());
	label->setPalette (pal);
	
	QPixmap pixmap = QPixmap::grabWidget (label);

	boundRect.setWidth (label->minimumSizeHint ().width () + 20);
	boundRect.setHeight (label->minimumSizeHint ().height () + 20);
	
	//p.drawPixmap (30, 1, pixmap);
	
	double d = p_parent->instructionsSplit ();
	p.drawLine (0, 0, this->width (), 0);
	p.drawLine (0, 0, (int)(d * this->width ()), this->height ());
	p.drawLine ((int)(d * this->width ()), this->height (), this->width (), 0);
	
	QRect r(0,0,0,0); 
	int h = 0; 
	int w = 0;
	if (! m_comment.isEmpty ()) {
		QFont oldFont = p.font ();
		QFont newFont(oldFont);
		newFont.setItalic (true);
		p.setFont (newFont);
		QPen oldPen = p.pen ();
		p.setPen (Qt::gray);
		p.drawText (1, 1, this->width () - 1, this->height () - 21, Qt::AlignTop|Qt::AlignHCenter|Qt::TextWordWrap, m_comment, &r);
		w = r.width ();
		h = r.height ();
		p.setFont (oldFont);
		p.setPen (oldPen);
	}
	p.drawText (1, r.height (), this->width () - 1, this->height () - r.height () - 21, Qt::AlignTop|Qt::AlignVCenter|Qt::AlignHCenter|Qt::TextWordWrap, m_text, &r);
	h += r.height ();
	w += r.width ();
	p.drawText (QRect(10, this->height () - 20, this->width () / 2, 20), 
		    Qt::AlignLeft | Qt::AlignBottom, tr("YES"));
	p.drawText (QRect(10 + this->width () / 2, this->height () - 20, this->width () / 2 - 20, 20), 
		    Qt::AlignRight | Qt::AlignBottom, tr("NO"));

	boundRect.setWidth (w);
	boundRect.setHeight (h + 20);
#ifdef DEBUG5
        qDebug() << boundRect;
#endif
#ifdef DEBUG4
	qDebug("[EXIT] SelectionCondition::paintEvent");
#endif
}
