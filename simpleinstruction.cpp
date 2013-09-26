#include "simpleinstruction.h"
#include "nsscheme.h"

#include <QMessageBox>
#include <QVBoxLayout>
#include <QPainter>

SimpleInstruction::SimpleInstruction(NSScheme *scheme, QWidget *parent, Instruction *parentInstruction, bool visible)
:Instruction(scheme, parent, parentInstruction, visible)
{
	setObjectName ("simple instruction");
	
	QPalette p = palette ();
	p.setBrush (QPalette::Background, QBrush(Qt::white));
	setPalette (p);

	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->setSpacing (0);
	layout->setMargin (4);

	label = new QLabel(this);
	label->setTextFormat (Qt::RichText);
	label->setVisible (false);
	label->setEnabled (false);
	p = label->palette ();
	p.setBrush (QPalette::Background, QBrush(Qt::white));
	label->setPalette (p);

	layout->addWidget (label);
	layout->setStretchFactor (label, 0);

	if (scheme) {
		connect (this, SIGNAL(instructionActivated(Instruction*)), 
			 scheme, SLOT(on_instructionActivated(Instruction*)));
	}
}

void SimpleInstruction::showEvent (QShowEvent *e)
{
	Q_UNUSED(e);
	label->setVisible (true);
}

void SimpleInstruction::setContents (QString newContents)
{
	m_contents = newContents;
	label->setText (labelTemplate
			.arg (m_comment.isEmpty () ? QString::null : escape (m_comment).append ("<br/>"))
			.arg (escape (m_contents)));
	/*label->adjustSize ();
	adjustSize ();
	layout ()->update ();*/
}

void SimpleInstruction::setComment (QString newComment)
{
	m_comment = newComment;
	label->setText (labelTemplate
			.arg (m_comment.isEmpty () ? QString::null : escape (m_comment).append ("<br/>"))
			.arg (escape (m_contents)));
/*	label->adjustSize ();
	adjustSize ();
	layout ()->update ();*/
}

void SimpleInstruction::setPixmap (QPixmap newPixmap)
{
	m_pixmap = newPixmap;
}

void SimpleInstruction::formatXMLNode (QDomDocument& document, QDomNode& parent)
{
	QDomText textContents = document.createTextNode (m_contents);
	QDomElement text = document.createElement ("text");
	text.appendChild (textContents);

	QDomText commentContents = document.createTextNode (m_comment);
	QDomElement comment = document.createElement ("comment");
	comment.appendChild (commentContents);

	QDomElement e = document.createElement ("simpleinstruction");
	e.appendChild (text);
	e.appendChild (comment);
	parent.appendChild (e);
}

void SimpleInstruction::formatSVGNode (QDomDocument& document, QDomNode& parent)
{
	QPoint p = positionInScheme ();
	QDomElement g = document.createElement ("g");
	g.appendChild (createSVGTextNode (document, p.x () + 10, this->height () / 2 + p.y () + label->fontInfo ().pixelSize () / 2, m_contents));
	if (! m_comment.isEmpty ())
		g.appendChild (createSVGTextNode (document, p.x () + 10, this->height () / 2 + p.y () - label->fontInfo ().pixelSize () / 2, m_comment));
	g.appendChild (createSVGRect (document));
	parent.appendChild (g);
}

bool SimpleInstruction::setAsXMLNode (QDomNode& node)
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

		validateContents ();
	} else {
		// tekst, komentarz i pixmapa puste
	}

	return true;
}

void SimpleInstruction::paintEvent (QPaintEvent *e)
{
	Instruction::paintEvent (e);
}

Instruction* SimpleInstruction::copyOf ()
{
	SimpleInstruction *retval = new SimpleInstruction(0,0,0,false);

	retval->setContents (m_contents);
	retval->setComment (m_comment);
	retval->setPixmap (m_pixmap);

	return retval;
}

Instruction* SimpleInstruction::execute (ExecutionThread *executor, bool *wait)
{
	Q_UNUSED(executor);
	Q_UNUSED(wait);

	setRunning (true);
	
	if (m_valid && statement) {
            execute_statement (this, statement->syntacticTree (), scheme ()->variables ());
	}

	setRunning (false);

	return nextInstruction ();
}

bool SimpleInstruction::validateContents ()
{
	do_validate (QSet<typ_skladnika>() << INSTR_PODSTAWIENIE);

	return m_valid;
}

void SimpleInstruction::recursiveValidateContents ()
{
        validateContents ();
}
