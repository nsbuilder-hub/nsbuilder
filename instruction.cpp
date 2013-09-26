#include "instruction.h"
#include "nsscheme.h"

#include <QBrush>
#include <QPalette>
#include <QMessageBox>
#include <QPainter>
#include <QDebug>

Instruction::Instruction(NSScheme *scheme, QWidget *parent, Instruction *parentInstruction, bool visible)
:QWidget(parent), p_scheme(scheme), p_parentInstruction(parentInstruction)
{
#if QT_VERSION >= 0x040100
	setAutoFillBackground (true);
#endif
	
	setActive (false);
	m_running = false;

	setVisible (visible);
	
	QPalette p = palette ();
	p.setBrush (QPalette::Background, QBrush(Qt::white));
	setPalette (p);

	connect(this, SIGNAL(runningChanged()), this, SLOT(on_runningChanged()));
        connect(&functionMap, SIGNAL(newCompoundStatement(const QString&)),
                    this, SLOT(on_newCompoundStatement(const QString&)));
        connect(&functionMap, SIGNAL(compoundStatementDeleted(const QString&)),
                this, SLOT(on_compoundStatementDeleted(const QString&)));
}

void Instruction::on_newCompoundStatement (const QString &name)
{
    Q_UNUSED(name);
    validateContents();
}

void Instruction::on_compoundStatementDeleted (const QString &name)
{
    Q_UNUSED(name);
    validateContents();
}

NSScheme* Instruction::scheme () const
{
	return p_scheme;
}

void Instruction::setScheme (NSScheme *scheme)
{
	if (p_scheme) {
		disconnect (this, SIGNAL(instructionActivated(Instruction*)), 
			    p_scheme, SLOT(on_instructionActivated(Instruction*)));
	}

	p_scheme = scheme;

	if (p_scheme) {
		connect (this, SIGNAL(instructionActivated(Instruction*)), 
			 p_scheme, SLOT(on_instructionActivated(Instruction*)));

		setVisible (true);
	}
}

Instruction* Instruction::parentInstruction () const
{
	return p_parentInstruction;
}

void Instruction::setParentInstruction (Instruction *parent)
{
	p_parentInstruction = parent;
}

QString Instruction::contents () 
{
	return m_contents;
}

QString Instruction::comment () 
{
	return m_comment;
}

QPixmap& Instruction::pixmap () 
{
	return m_pixmap;
}

QColor Instruction::backgroundColor ()
{
	if (p_scheme == 0)
		return Qt::white;

	if (m_running)
		return p_scheme->executionColor ();
	else if (m_active)
		return p_scheme->selectionColor ();
	else if (! m_valid)
		return p_scheme->invalidColor ();
	else 
		return Qt::white;
}

bool Instruction::isActive () const
{
	return m_active;
}

void Instruction::setActive (bool v)
{
	m_active = v;

	if (m_active) {
		emit instructionActivated (this);
	}

	QPalette p = palette ();
	p.setBrush (QPalette::Background, QBrush(backgroundColor ()));
	setPalette (p);
	repaint ();
}

void Instruction::mousePressEvent (QMouseEvent *e)
{
	qDebug(QString("Instruction::mousePressEvent: contents=%1 className=%2").arg(m_contents).arg(this->metaObject ()->className ()).toUtf8 ());

	if (e->button () == Qt::LeftButton) {
                /*if (isActive ()) {
			setActive (false);
			if (parentInstruction ()) 
				parentInstruction ()->setActive (true);
                } else*/ {
			setActive (true);
		}
	} else if (e->button () == Qt::RightButton) {
		// menu kontekstowe
	}
}

void Instruction::mouseDoubleClickEvent (QMouseEvent *event)
{
	Q_UNUSED(event);

	if (scheme ()) {
		scheme ()->editInstruction ();
	}
}

void Instruction::do_validate (const QSet<enum typ_skladnika> &types)
{
#ifdef DEBUGV
qDebug() << "[ENTER] Instruction::do_validate" << m_contents;
#endif
	if (scheme () == 0) {
		m_valid = true;
#ifdef DEBUGV
qDebug() << "[EXIT] Instruction::do_validate | no scheme";
#endif
		return;
	}

        if (scheme ()->isClosing ()) {
            m_valid = true;
#ifdef DEBUGV
qDebug() << "[EXIT] Instruction::do_validate | scheme is closing";
#endif
            return;
        }

	if (! scheme ()->containsPascalCode ()) {
		m_valid = true;
#ifdef DEBUGV
qDebug() << "[EXIT] Instruction::do_validate | not pascal";
#endif
		return;
	}
	
	if (! scheme ()->checkSyntax ()) {
		m_valid = true;
#ifdef DEBUGV
qDebug() << "[EXIT] Instruction::do_validate | no syntax check";
#endif
		return;
	}
	
	m_valid = false;
	
        statement = ProgramStatement::scan_buffer (m_contents, scheme ()->variables ());

	if (statement) { 
		if (statement->syntacticTree ()) {
			if (types.contains (statement->syntacticTree ()->typ)) {
                                QString failed;
                                if (statement->syntacticTree ()->typeCheck (scheme ()->variables (), failed)) {
					m_valid = true;
#ifdef DEBUGV
qDebug() << "syntax and type check OK for statement" << m_contents;
#endif
                                } else {
#ifdef DEBUGV
qDebug() << "type check failed";
#endif
                                    syntaxErrorText = failed; //tr("Syntax correct but type check failed");
                                    typeCheckFailed = true;
				}
			} else {
#ifdef DEBUGV
qDebug() << "this is not expected statement type, got " << statement->syntacticTree ()->typ << "expected " << types;
#endif
                        }
                } else {
#ifdef DEBUGV
qDebug("no syntactic tree, parser error (?)");
#endif
                }
	} else {
#ifdef DEBUGV
qDebug("no statement, syntax error");
#endif
        }

	if (! m_valid) {
		delete statement;
		statement = 0;
	}
#ifdef DEBUGV
	qDebug() << "[EXIT] Instruction::do_validate";
#endif
}

QDomElement Instruction::createSVGRect (QDomDocument &document)
{
	QPoint p = positionInScheme ();
	QDomElement rect = document.createElement ("rect");
	QDomAttr x = document.createAttribute ("x");
	x.setValue (QString("%1").arg (p.x ()));
	rect.setAttributeNode (x);
	QDomAttr y = document.createAttribute ("y");
	y.setValue (QString("%1").arg (p.y ()));
	rect.setAttributeNode (y);
	QDomAttr width = document.createAttribute ("width");
	width.setValue (QString("%1").arg (this->width ()));
	rect.setAttributeNode (width);
	QDomAttr height = document.createAttribute ("height");
	height.setValue (QString("%1").arg (this->height ()));
	rect.setAttributeNode (height);
	QDomAttr fill = document.createAttribute ("fill");
	fill.setValue ("none");
	rect.setAttributeNode (fill);
	QDomAttr stroke = document.createAttribute ("stroke");
	stroke.setValue ("black");
	rect.setAttributeNode (stroke);
	QDomAttr strokewidth = document.createAttribute ("stroke-width");
	strokewidth.setValue ("1");
	rect.setAttributeNode (strokewidth);

	return rect;
}


QString Instruction::alignmentToAnchor (Qt::AlignmentFlag f)
{
	switch (f) {
		case Qt::AlignRight:
			return "end";
		case Qt::AlignHCenter:
			return "middle";
		case Qt::AlignLeft:
		case Qt::AlignJustify:
		default:
			return "start";
	}
}

QDomElement Instruction::createSVGTextNode (QDomDocument &document, int x, int y, const QString& text, Qt::AlignmentFlag f)
{
	QDomText textContents = document.createTextNode (text);
	QDomElement t = document.createElement ("text");
	t.appendChild (textContents);
	QDomAttr xcn = document.createAttribute ("x");
	xcn.setValue (QString ("%1").arg (x));
	t.setAttributeNode (xcn);
	QDomAttr ycn = document.createAttribute ("y");
	ycn.setValue (QString ("%1").arg (y));
	t.setAttributeNode (ycn);
	QDomAttr ta = document.createAttribute ("text-anchor");
	ta.setValue (alignmentToAnchor(f));
	t.setAttributeNode (ta);
	
	return t;
}

QDomElement Instruction::createSVGLine (QDomDocument &document, int x1, int y1, int x2, int y2)
{
	QDomElement line = document.createElement ("line");
	QDomAttr ax1 = document.createAttribute ("x1");
	ax1.setValue (QString("%1").arg (x1));
	line.setAttributeNode (ax1);
	QDomAttr ay1 = document.createAttribute ("y1");
	ay1.setValue (QString("%1").arg (y1));
	line.setAttributeNode (ay1);
	QDomAttr ax2 = document.createAttribute ("x2");
	ax2.setValue (QString("%1").arg (x2));
	line.setAttributeNode (ax2);
	QDomAttr ay2 = document.createAttribute ("y2");
	ay2.setValue (QString("%1").arg (y2));
	line.setAttributeNode (ay2);
	QDomAttr stroke = document.createAttribute ("stroke");
	stroke.setValue ("black");
	line.setAttributeNode (stroke);
	QDomAttr strokewidth = document.createAttribute ("stroke-width");
	strokewidth.setValue ("1");
	line.setAttributeNode (strokewidth);

	return line;
}

bool Instruction::isFirstInSequence ()
{
        Sequence *s = qobject_cast<Sequence *> (parentInstruction());

        if (s) {
                return s->indexOf(this) == 0;
        } else {
                return false;
        }
}

void Instruction::paintEvent (QPaintEvent *e)
{
	QPainter p(this);
	p.setClipRect (e->rect ());
        p.fillRect (QRect(0, 0, this->width (), this->height ()), backgroundColor ());
        if (isFirstInSequence())
                p.drawRect (0, 0, this->width () - 1, this->height () - 1);
        else
                p.drawRect (0, -1, this->width () - 1, this->height ());        
}

bool Instruction::valid ()
{
	return m_valid;
}

QString Instruction::toString ()
{
	return QString("{%2} %1").arg (m_contents).arg (m_comment);
}

Instruction* Instruction::nextInstruction ()
{
	Instruction *next = 0;
	Sequence *seq = 0;

	if ((seq = qobject_cast<Sequence*> (parentInstruction ()))) {
		next = seq->nextInstruction (this);
	}

	return next;
}

void Instruction::on_runningChanged ()
{
	update ();
}

bool Instruction::isRunning ()
{
	return m_running;
}

void Instruction::setRunning (bool b)
{
	m_running = b;

	emit runningChanged();
}

QPoint Instruction::positionInScheme ()
{
	QPoint p(0, 0);
	
	if (p_scheme) {
		QPoint pi = mapToGlobal (QPoint (0, 0));
		QPoint ps = p_scheme->instructions ()->mapToGlobal (QPoint (0, 0));
		p = pi - ps;
	}
	
	return p;
}

