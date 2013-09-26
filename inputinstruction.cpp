#include "inputinstruction.h"
#include "nsscheme.h"
#include "executionthread.h"

#include <QMessageBox>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QPainter>

InputInstruction::InputInstruction(NSScheme *scheme, QWidget *parent, Instruction *parentInstruction, bool visible)
:Instruction(scheme, parent, parentInstruction, visible)
{
	setObjectName ("input instruction");
	
	QPalette p = palette ();
	p.setBrush (QPalette::Background, QBrush(Qt::white));
	setPalette (p);

	QHBoxLayout *layout = new QHBoxLayout(this);
	layout->setSpacing (0);
	layout->setMargin (4);

	label = new QLabel(this);
	label->setText (tr("<empty>"));
	label->setVisible (false);
	label->setEnabled (false);
	p = label->palette ();
	p.setBrush (QPalette::Background, QBrush(Qt::white));
	label->setPalette (p);

	layout->addSpacing (20);
	layout->addWidget (label);
	layout->setStretchFactor (label, 0);

	if (scheme) {
		connect (this, SIGNAL(instructionActivated(Instruction*)), 
			 scheme, SLOT(on_instructionActivated(Instruction*)));
	}
}

void InputInstruction::showEvent (QShowEvent *e)
{
	Q_UNUSED(e);
	label->setVisible (true);
}

void InputInstruction::setContents (QString newContents)
{
	m_contents = newContents;
	label->setText (labelTemplate
			.arg (m_comment.isEmpty () ? QString::null : escape (m_comment).append ("<br/>"))
			.arg (escape (m_contents)));
	label->adjustSize ();
	adjustSize ();
	layout ()->update ();

	//validateContents ();
}

void InputInstruction::setComment (QString newComment)
{
	m_comment = newComment;
	label->setText (labelTemplate
			.arg (m_comment.isEmpty () ? QString::null : escape (m_comment).append ("<br/>"))
			.arg (escape (m_contents)));
	label->adjustSize ();
	adjustSize ();
	layout ()->update ();
}

void InputInstruction::setPixmap (QPixmap newImage)
{
	m_pixmap = newImage;
}

void InputInstruction::formatXMLNode (QDomDocument& document, QDomNode& parent)
{
	QDomText textContents = document.createTextNode (m_contents);
	QDomElement text = document.createElement ("text");
	text.appendChild (textContents);

	QDomText commentContents = document.createTextNode (m_comment);
	QDomElement comment = document.createElement ("comment");
	comment.appendChild (commentContents);

	QDomElement e = document.createElement ("inputinstruction");
	e.appendChild (text);
	e.appendChild (comment);
	parent.appendChild (e);
}

void InputInstruction::formatSVGNode (QDomDocument& document, QDomNode& parent)
{
	QPoint p = positionInScheme ();
	QDomElement g = document.createElement ("g");
	g.appendChild (createSVGTextNode (document, p.x () + 10, this->height () / 2 + p.y () + label->fontInfo ().pixelSize () / 2, m_contents));
	if (! m_comment.isEmpty ())
		g.appendChild (createSVGTextNode (document, p.x () + 10, this->height () / 2 + p.y () - label->fontInfo ().pixelSize () / 2, m_comment));
	g.appendChild (createSVGRect (document));
	g.appendChild (createSVGLine (document, p.x () + 0, p.y () + 20, p.x () + 20, p.y () + 0));
	parent.appendChild (g);
}

bool InputInstruction::setAsXMLNode (QDomNode& node)
{
	if (node.hasChildNodes ()) {
		QDomNodeList nodeList = node.childNodes ();
		
		for (unsigned i = 0; i < nodeList.length (); i++) {
			QDomElement e = nodeList.item (i).toElement ();

			if (! e.isNull ()) {
				if (e.tagName () == "text") {
					QDomNode t = e.firstChild ();
					setContents (t.nodeValue ());
				} else if (e.tagName () == "comment") {
					QDomNode t = e.firstChild ();
					setComment (t.nodeValue ());
				}
			}
		}
	} else {
		// tekst, komentarz i pixmapa puste
	}

	validateContents ();

	return true;
}

void InputInstruction::paintEvent (QPaintEvent *e)
{
        Instruction::paintEvent (e);

	QPainter p(this);
	p.fillRect (QRect(1, 1, 30, this->height () - 2), backgroundColor ());
	p.drawLine (0, 20, 20, 0);
}

Instruction* InputInstruction::copyOf ()
{
	InputInstruction* retval = new InputInstruction(0,0,0,false);

	retval->setContents (m_contents);
	retval->setComment (m_comment);
	retval->setPixmap (m_pixmap);

	return retval;
}

QSize InputInstruction::minimumSizeHint () const
{
	QSize s = label->minimumSizeHint ();
	s.rwidth () += 30;
	return s;
}

void InputInstruction::readInputValue (drzewo_skladn *t, ExecutionThread *e, ProgramVariables *vars)
{
	if (t == 0)
		return;
	
	if (t->typ == ATOM_IDENT) {
		QString nazwa = t->zmienna->ident;
		BaseValueDeterminant &d = vars->byName (nazwa)->v.bval.d;
		if (d == Long) {
			vars->byName (nazwa)->v.bval.val = 
				e->getIntegerInputValue (tr("Input"), tr("Input value for %1").arg (nazwa));
		} else if ((d == Double) || (d == Unknown)) {
			vars->byName (nazwa)->v.bval.fval = 
				e->getDoubleInputValue (tr("Input"), tr("Input value for %1").arg (nazwa));
			d = Double;
		} else {
			qDebug () << "Unexpected variable type";
		}
	} else if (t->typ == WYR_INDEKS) {
                long v = e->getDoubleInputValue (tr("Input"), tr("Input value for element of array %1").arg (t->zmienna->ident));
		drzewo_skladn tv(ATOM_LICZBA, v);
		drzewo_skladn tx(INSTR_PODSTAWIENIE, 0L);
		tx.skladnik[0] = t;
		tx.skladnik[1] = &tv;
                execute_statement (this, &tx, vars);
	} else {
		qDebug () << "Unknown expression tree node type: " << t->typ;
	}
}

Instruction* InputInstruction::execute (ExecutionThread *executor, bool *wait)
{
	Q_UNUSED(wait);

	setRunning (true);

	if (m_valid && statement) {
		ProgramVariables *vars = scheme ()->variables ();
		drzewo_skladn *t = statement->syntacticTree ();

		while (t != 0) {
			if (t->typ == LISTA_LWYR) {
				readInputValue (t->skladnik[0], executor, vars);
			} else {
				readInputValue (t, executor, vars);
			}
			t = t->skladnik[1];
		}
	}

	setRunning (false);

	return nextInstruction ();
}

bool InputInstruction::validateContents ()
{
        do_validate (QSet<typ_skladnika>() << LISTA_LWYR << ATOM_IDENT << WYR_INDEKS);

	return m_valid;
}

void InputInstruction::recursiveValidateContents ()
{
        validateContents ();
}
