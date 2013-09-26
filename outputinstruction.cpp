#include "outputinstruction.h"
#include "nsscheme.h"
#include "executionthread.h"

#include <QMessageBox>
#include <QHBoxLayout>
#include <QPainter>

OutputInstruction::OutputInstruction(NSScheme *scheme, QWidget *parent, Instruction *parentInstruction, bool visible)
:Instruction(scheme, parent, parentInstruction, visible)
{
	setObjectName ("output instruction");
	
	QPalette p = palette ();
	p.setBrush (QPalette::Background, QBrush(Qt::white));
	setPalette (p);

	QHBoxLayout *layout = new QHBoxLayout(this);
	layout->setSpacing (0);
	layout->setMargin (4);

	label = new QLabel(this);
	label->setTextFormat (Qt::RichText);
	label->setText (tr("empty"));
	label->setVisible (false);
	label->setEnabled (false);    
	p = label->palette ();
	p.setBrush (QPalette::Background, QBrush(Qt::white));
	label->setPalette (p);

	layout->addWidget (label);
	layout->addSpacing (30);
	layout->setStretchFactor (label, 0);

	if (scheme) {
		connect (this, SIGNAL(instructionActivated(Instruction*)), 
			 scheme, SLOT(on_instructionActivated(Instruction*)));
	}
}

void OutputInstruction::showEvent (QShowEvent *e)
{
	Q_UNUSED(e);
	label->setVisible (true);
}

void OutputInstruction::setContents (QString newContents)
{
	m_contents = newContents;
        label->setText (labelTemplate
                        .arg (m_comment.isEmpty () ? QString::null : escape (m_comment).append ("<br/>"))
                        .arg (escape (m_contents)));
	label->adjustSize ();
	adjustSize ();
	layout ()->update ();
}

void OutputInstruction::setComment (QString newComment)
{
	m_comment = newComment;
        label->setText (labelTemplate
                        .arg (m_comment.isEmpty () ? QString::null : escape (m_comment).append ("<br/>"))
                        .arg (escape (m_contents)));
	label->adjustSize ();
	adjustSize ();
	layout ()->update ();
}

void OutputInstruction::setPixmap (QPixmap newImage)
{
	m_pixmap = newImage;
}

void OutputInstruction::formatXMLNode (QDomDocument& document, QDomNode& parent)
{
	QDomText textContents = document.createTextNode (m_contents);
	QDomElement text = document.createElement ("text");
	text.appendChild (textContents);

	QDomText commentContents = document.createTextNode (m_comment);
	QDomElement comment = document.createElement ("comment");
	comment.appendChild (commentContents);

	QDomElement e = document.createElement ("outputinstruction");
	e.appendChild (text);
	e.appendChild (comment);
	parent.appendChild (e);
}

void OutputInstruction::formatSVGNode (QDomDocument& document, QDomNode& parent)
{
	QPoint p = positionInScheme ();
	QDomElement g = document.createElement ("g");
	g.appendChild (createSVGTextNode (document, p.x () + 10, this->height () / 2 + p.y () + label->fontInfo ().pixelSize () / 2, m_contents));
	if (! m_comment.isEmpty ())
		g.appendChild (createSVGTextNode (document, p.x () + 10, this->height () / 2 + p.y () - label->fontInfo ().pixelSize () / 2, m_comment));
	g.appendChild (createSVGRect (document));
	g.appendChild (createSVGLine (document, p.x () +  this->width () - 20, p.y () + this->height (), p.x () + this->width (), p.y () + this->height () - 20));
	parent.appendChild (g);
}

bool OutputInstruction::setAsXMLNode (QDomNode& node)
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

void OutputInstruction::paintEvent (QPaintEvent *e)
{
        Instruction::paintEvent (e);

        QPainter p(this);
	p.fillRect (QRect(this->width () - 34, 4, 30, this->height () - 8), backgroundColor ());
	p.drawLine (this->width () - 20, this->height (), this->width (), this->height () - 20);
}

Instruction* OutputInstruction::copyOf ()
{
	OutputInstruction *retval = new OutputInstruction(0,0,0,false);

	retval->setContents (m_contents);
	retval->setComment (m_comment);
	retval->setPixmap (m_pixmap);

	return retval;
}

QSize OutputInstruction::minimumSizeHint () const
{
	QSize s = label->minimumSizeHint ();
	s.rwidth () += 20;
	return s;
}

QString OutputInstruction::syntacticTreeValueToString (drzewo_skladn *t, ProgramVariables *vars)
{
	QString info;

	if (t == 0)
		return info;
	
	if ((t->typ == ATOM_IDENT)) {
		info = /*tr("Variable %1 has value %2")
			.arg (t->zmienna->ident)
			.arg */(vars->byName (t->zmienna->ident)->v.bval.toString ());
	} else if (t->typ == WYR_INDEKS) {
                BaseValue b = execute_statement (this, t, vars);
		info = /*tr("Array element has value %1")
			.arg */(b.toString ());
	} else if (t->typ == ATOM_NAPIS) {
		info = t->val.toString ();
	} else if (t->typ == ATOM_LICZBA) {
		info = t->val.toString ();
	}

	return info;
}

Instruction* OutputInstruction::execute (ExecutionThread *executor, bool *wait)
{
	Q_UNUSED(wait);

	setRunning (true);
	
	if (m_valid && statement) {
		ProgramVariables *vars = scheme ()->variables ();
		drzewo_skladn *t = statement->syntacticTree ();
		QString s;
		while (t) {
			QString info;

			qDebug() << "Tree type: " << t->typ;
			if ((t->typ == LISTA_WARTOSCI) || (t->typ == LISTA_LWYR)) {
				info = syntacticTreeValueToString (t->skladnik[0], vars);
			} else {
				info = syntacticTreeValueToString (t, vars);
			}
			qDebug(info.toLocal8Bit ());
			if (! info.isEmpty ())
				s.append (" ").append (info);

			t = t->skladnik[1];
		}
		executor->showMessage (tr("Output"), s);
	}
	
	setRunning (false);

	return nextInstruction ();
}

bool OutputInstruction::validateContents ()
{
	do_validate (QSet<typ_skladnika>() << LISTA_WARTOSCI << LISTA_LWYR << ATOM_LICZBA << ATOM_IDENT << WYR_INDEKS << ATOM_NAPIS);

	return m_valid;
}

void OutputInstruction::recursiveValidateContents ()
{
        validateContents ();
}
