#include <QPainter>

#include "iteration.h"
#include "nsscheme.h"

Iteration::Iteration(NSScheme *scheme, Variant v, QWidget *parent, Instruction *parentInstruction, bool visible)
:Instruction(scheme, parent, parentInstruction, visible), m_variant(v)
{
	QPalette p = palette ();
	p.setBrush (QPalette::Background, QBrush(Qt::white));
	setPalette (p);

	QVBoxLayout *vLayout = new QVBoxLayout(this);
	vLayout->setSpacing (0);
	vLayout->setMargin (0);

	label = new QLabel(this);
	label->setIndent (4);
	label->setText (tr("<empty>"));
	label->setVisible (false);
	label->setEnabled (false);
	p = label->palette ();
	p.setBrush (QPalette::Background, QBrush(Qt::white));
	label->setPalette (p);

	iWidget = new QWidget(this);
	hLayout = new QHBoxLayout(iWidget);
	hLayout->setSpacing (0);
	hLayout->setMargin (0);
	QWidget *w = new QWidget(iWidget);
	w->setMinimumSize (40, 1);
	p = w->palette ();
	p.setBrush (QPalette::Background, QBrush(Qt::white));
	w->setPalette (p);
	hLayout->addWidget (w);

	m_instructions = new Sequence(scheme, iWidget, this);

	hLayout->addWidget (m_instructions);
	hLayout->setStretchFactor (m_instructions, 1);
	
	vLayout->addWidget (label);
	vLayout->setStretchFactor (label, 0);
	vLayout->addWidget (iWidget);
	vLayout->setStretchFactor (iWidget, 1);
	
	if (scheme) {
		connect (this, SIGNAL(instructionActivated(Instruction*)), 
			 scheme, SLOT(on_instructionActivated(Instruction*)));
	}
}

void Iteration::showEvent (QShowEvent *e)
{
	Q_UNUSED(e);
	label->setVisible (true);
}

Sequence* Iteration::instructions () 
{
	return m_instructions;
}

void Iteration::setInstructions (Sequence* s) 
{
	if (m_instructions) {
		// usuwamy z layoutu na którym leży
		QLayoutItem *p = hLayout->takeAt (1); 
		// i niszczymy obiekt
		delete p;
		delete m_instructions;
		m_instructions = 0;
	}

	m_instructions = s;

	if (m_instructions) {
		m_instructions->setScheme (scheme ());
		m_instructions->setParentInstruction (this);
		m_instructions->setParent (iWidget);
		hLayout->addWidget (m_instructions);
		hLayout->setStretchFactor (m_instructions, 1);
		adjustSize ();
		hLayout->update ();
	}
}

void Iteration::setContents (QString newContents)
{
	m_contents = newContents;
	label->setText (labelTemplate
			.arg (m_comment.isEmpty () ? QString::null : escape (m_comment).append ("<br/>"))
			.arg (escape (m_contents)));
}

void Iteration::setComment (QString newComment)
{
	m_comment = newComment;
	label->setText (labelTemplate
			.arg (m_comment.isEmpty () ? QString::null : escape (m_comment).append ("<br/>"))
			.arg (escape (m_contents)));
}

void Iteration::setPixmap (QPixmap newPixmap)
{
	m_pixmap = newPixmap;
}

void Iteration::formatXMLNode (QDomDocument& document, QDomNode& parent)
{
	QDomText textContents = document.createTextNode (m_contents);
	QDomElement text = document.createElement ("text");
	text.appendChild (textContents);

	QDomText commentContents = document.createTextNode (m_comment);
	QDomElement comment = document.createElement ("comment");
	comment.appendChild (commentContents);

	QDomElement condition = document.createElement ("condition");
	condition.appendChild (text);
	condition.appendChild (comment);
	
	QDomElement loop = document.createElement ("loop");
	m_instructions->formatXMLNode (document, loop);
		
	QDomElement iteration = document.createElement ("iteration");
	iteration.appendChild (condition);
	iteration.appendChild (loop);

	parent.appendChild (iteration);
}

void Iteration::formatSVGNode (QDomDocument& document, QDomNode& parent)
{
	QDomElement g = document.createElement ("g");

	QPoint p = positionInScheme ();
	int h = m_instructions->positionInScheme ().y () - p.y ();
	g.appendChild (createSVGTextNode (document, p.x () + 20, p.y () + h / 2 + label->fontInfo ().pixelSize () / 2, m_contents));
	if (! m_comment.isEmpty ())
		g.appendChild (createSVGTextNode (document, p.x () + 20, p.y () + h / 2 - label->fontInfo ().pixelSize () / 2, m_comment));
	g.appendChild (createSVGRect (document));
	
	if (m_instructions)
		m_instructions->formatSVGNode (document, g);
	
	parent.appendChild (g);

}
bool Iteration::setAsXMLNode (QDomNode& node)
{
	bool retval = true;
	
	if (node.hasChildNodes ()) {
		QDomNodeList nodeList = node.childNodes ();
		
		for (unsigned i = 0; retval && (i < nodeList.length ()); i++) {
			QDomElement e = nodeList.item (i).toElement ();

			if (! e.isNull ()) {
				if (e.tagName () == "condition") {
					QDomNodeList conditionNodeList = e.childNodes ();

					for (unsigned j = 0; j < conditionNodeList.length (); j++) {
						QDomElement el = conditionNodeList.item (j).toElement ();

						if (! el.isNull ()) {
							if (el.tagName () == "text") {
								QDomNode t = el.firstChild ();
								setContents (t.nodeValue ());
							} else if (el.tagName () == "comment") {
								QDomNode t = el.firstChild ();
								setComment (t.nodeValue ());
							}
						}
					}

					validateContents ();
				} else if (e.tagName () == "loop") {
					if (e.hasChildNodes () && e.firstChild ().nodeName () == "sequence") {
						Sequence *s = new Sequence(scheme (), 0, 0);
						setInstructions (s);

						QDomNode n = e.firstChild ();
						retval = retval && s->setAsXMLNode (n);
					}
				}
			}
		}
	}

	return retval;
}

void Iteration::paintEvent (QPaintEvent *e)
{
        Instruction::paintEvent(e);
//	QPainter p(this);
//	p.setClipRect (e->rect ());
//	p.drawRect (0, 0, this->width () - 1, this->height () - 1);
//	p.fillRect (QRect(1, 1, this->width () - 2, this->height () - 2), backgroundColor ());
}

Instruction* Iteration::copyOf ()
{
        Iteration *retval = new Iteration(0, m_variant, 0, 0, false);

	retval->setContents (m_contents);
	retval->setComment (m_comment);
	retval->setPixmap (m_pixmap);

	retval->setInstructions (qobject_cast<Sequence*> (m_instructions->copyOf ()));

	return retval;
}

void Iteration::setScheme (NSScheme *scheme)
{
	Instruction::setScheme (scheme);

	m_instructions->setScheme (scheme);
}

QSize Iteration::minimumSizeHint () const
{
	int w, h;
	
	w = qMax (label->minimumSizeHint().width (), 40 + m_instructions->minimumSizeHint ().width ());
	h = label->minimumSizeHint().height () + m_instructions->minimumSizeHint ().height ();
	
	return QSize (w, h);
}

Instruction* Iteration::execute (ExecutionThread *executor, bool *wait)
{
	Q_UNUSED(executor);
	Q_UNUSED(wait);

	setRunning (true);
	
	BaseValue b;
	 
	if (m_valid && statement) {
		// Oblicz wartość wyrażenia logicznego
                b = execute_statement (this, statement->syntacticTree (), scheme ()->variables ());
	}

	Instruction *next = this;
	
	if ((b.d == Long) ? b.val : b.fval) {
		if (m_instructions->count () > 0) {
			next = m_instructions->instructionAt (0);
		}
	} else {
		// Wyznacz instrukcję do wykonania	
		next = nextInstruction ();
	}

	setRunning (false);

	return next;
}

bool Iteration::validateContents ()
{
	do_validate (QSet<typ_skladnika>() << BWYR << ATOM_LOG << OPER_NOT << OPER_LOG << OPER_REL);

	return m_valid;
}

void Iteration::recursiveValidateContents ()
{
        validateContents ();
        m_instructions->recursiveValidateContents ();
}
