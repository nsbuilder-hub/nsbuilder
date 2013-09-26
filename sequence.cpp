#include "sequence.h"
#include "simpleinstruction.h"
#include "inputinstruction.h"
#include "outputinstruction.h"
#include "callinstruction.h"
#include "returninstruction.h"
#include "selection.h"
#include "iteration.h"
#include "nsscheme.h"

#include <QVBoxLayout>
#include <QMessageBox>
#include <QPainter>

Sequence::Sequence(NSScheme *scheme, QWidget *parent, Instruction *parentInstruction, bool visible)
:Instruction(scheme, parent, parentInstruction, visible)
{
	QPalette p = palette ();
	p.setBrush (QPalette::Background, QBrush(Qt::white));
	setPalette (p);

	m_layout = new QVBoxLayout(this);
	m_layout->setSpacing (0);
	m_layout->setMargin (0/*1*/);
	
	if (scheme) {
		connect (this, SIGNAL(instructionActivated(Instruction*)), 
			 scheme, SLOT(on_instructionActivated(Instruction*)));
	}

	m_valid = true;
}

int Sequence::count () 
{
	return m_layout->count ();
}

Instruction* Sequence::instructionAt (int num) 
{
	return m_instructions.at (num);
}

void Sequence::appendInstruction (Instruction* i) 
{
	i->setParentInstruction (this);
	m_layout->addWidget (i);
	m_instructions.append (i);

	adjustSize ();
	m_layout->update ();
}

void Sequence::insertInstruction (Instruction* i, int index) 
{
	i->setParentInstruction (this);
	m_layout->insertWidget (index, i);
	m_instructions.insert (index, i);
        i->setVisible(true);

	adjustSize ();
	m_layout->update ();
}

void Sequence::removeInstructionAt (int index)
{
	if (index < 0) 
		return;

	QLayoutItem *i = m_layout->takeAt (index);
	if (i) {
		QWidget *widget = i->widget ();
		
		if (widget) {
			widget->hide ();
		}
		
                delete i;

		m_instructions.remove (index);

		adjustSize ();
		m_layout->update ();
	}
}

void Sequence::setContents (QString newContents)
{
	Q_UNUSED(newContents);
}

void Sequence::setComment (QString newComment)
{
	Q_UNUSED(newComment);
}

void Sequence::setPixmap (QPixmap newPixmap)
{
	Q_UNUSED(newPixmap);
}

void Sequence::formatXMLNode (QDomDocument& document, QDomNode& parent)
{
	QDomElement sequence = document.createElement ("sequence");

	for (int i = 0; i < m_instructions.count (); i++) {
		m_instructions.at (i)->formatXMLNode (document, sequence);
	}

	parent.appendChild (sequence);
}

void Sequence::formatSVGNode (QDomDocument& document, QDomNode& parent)
{
	QDomElement g = document.createElement ("g");

	for (int i = 0; i < m_instructions.count (); i++) {
		m_instructions.at (i)->formatSVGNode (document, g);
	}

	parent.appendChild (g);
}

bool Sequence::setAsXMLNode (QDomNode& node)
{
	bool retval = true;
	QDomNodeList nodeList = node.childNodes ();

	for (unsigned i = 0; retval && (i < nodeList.length ()); i++) {
		QDomElement e = nodeList.item (i).toElement ();

		if (! e.isNull ()) {
			Instruction *i = 0;
			
			if (e.tagName () == "simpleinstruction") {
				SimpleInstruction *si = new SimpleInstruction(scheme (), 0, 0, false);
				i = si;
			} else if (e.tagName () == "selection") {
				Selection *s = new Selection(scheme (), 0, 0);
				i = s;
			} else if (e.tagName () == "iteration") {
                                Iteration *it = new Iteration(scheme (), Iteration::WHILEDO, 0, 0);
				i = it;
			} else if (e.tagName () == "sequence") {
				Sequence *s = new Sequence(scheme (), 0, 0);
				i = s;
			} else if (e.tagName () == "inputinstruction") {
				InputInstruction *ii = new InputInstruction(scheme (), 0, 0);
				i = ii;
			} else if (e.tagName () == "outputinstruction") {
				OutputInstruction *oi = new OutputInstruction(scheme (), 0, 0);
				i = oi;
			} else if (e.tagName () == "callinstruction") {
				CallInstruction *ci = new CallInstruction(scheme (), 0, 0);
				i = ci;
                        } else if (e.tagName() == "returninstruction") {
                                ReturnInstruction *ri = new ReturnInstruction(scheme(), 0, 0);
                                i = ri;
                        }

			if (i) {
				retval = retval && i->setAsXMLNode (e);

				appendInstruction (i);
			}
		}
	}

	return retval;
}

void Sequence::paintEvent (QPaintEvent *e)
{
	Q_UNUSED(e);

	QPainter p(this);
	p.drawRect (0, 0, this->width () - 1, this->height () - 1);
}

Instruction* Sequence::copyOf ()
{
	Sequence* retval = new Sequence(0,0,0,false);

	for (int i = 0; i < m_instructions.count (); i++) {
		retval->appendInstruction (m_instructions.at (i)->copyOf ());
#ifdef DEBUG5
qDebug(m_instructions.at (i)->contents ().toUtf8 ());
#endif
        }

	return retval;
}

void Sequence::setScheme (NSScheme *scheme)
{
	Instruction::setScheme (scheme);

	for (int i = 0; i < m_instructions.count (); i++) {
		m_instructions.at (i)->setScheme (scheme);
	}
}

QSize Sequence::minimumSizeHint () const
{
        int w = 20, h = 0;
	
	for (int i = 0; i < m_instructions.count (); i++) {
		QSize tmp = m_instructions.at (i)->minimumSizeHint ();
		
		w = qMax(w, tmp.width ());
		h += tmp.height ();
	}
	
	return QSize(w, h);
}

int Sequence::indexOf (Instruction *i)
{
	return m_instructions.indexOf (i);
}

Instruction* Sequence::nextInstruction (Instruction *i)
{
	int ind = indexOf (i);

	if (ind >= 0) {
		if (ind < count () - 1) {
			return instructionAt (ind + 1);
		} else {
			Instruction *i = parentInstruction ();

			if (i == 0) {
				return 0;
			} else if (qobject_cast<Iteration*> (i)) {
				return i;
			} else {
				Sequence *seq = qobject_cast<Sequence*> (i->parentInstruction ());

				if (seq) {
					return seq->nextInstruction (i);
				} else {
					return 0;
				}
			}
		}
	} else {
		return 0;
	}
}

Instruction* Sequence::execute (ExecutionThread *executor, bool *wait)
{
	Q_UNUSED(executor);
	Q_UNUSED(wait);

	setRunning (false);
	
	Instruction *next = 0;
	
	if (m_instructions.count () > 0) {
		next = m_instructions.at (0);//->execute (executor, wait);
	} else if (parentInstruction ()) {
		if (parentInstruction ()->parentInstruction ()) {
			if (Sequence *seq = qobject_cast<Sequence*> (parentInstruction ()->parentInstruction ()))
				next = seq->nextInstruction (parentInstruction ());
		}
	}

	return next;
}

bool Sequence::validateContents ()
{
	return m_valid = true;
}

void Sequence::recursiveValidateContents ()
{
        validateContents ();
        for (int i = 0; i < m_instructions.count (); i++) {
                m_instructions.at (i)->recursiveValidateContents ();
        }
}
