#include <QGridLayout>

#include "selection.h"
#include "simpleinstruction.h"
#include "nsscheme.h"
#include "selectioncondition.h"

Selection::Selection(NSScheme *scheme, QWidget *parent, Instruction *parentInstruction, bool visible)
:Instruction(scheme, parent, parentInstruction, visible)
{
	QPalette p = palette ();
	p.setBrush (QPalette::Background, QBrush(Qt::white));
	setPalette (p);

	QVBoxLayout *vLayout = new QVBoxLayout(this);
	vLayout->setSpacing (0);
	vLayout->setMargin (0/*3*/);

	splitter = new QSplitter(parent);
	p = splitter->palette ();
	p.setBrush (QPalette::Background, QBrush(Qt::white));
	splitter->setPalette (p);

	hWidget = new QWidget(this);
	hLayout = new QGridLayout(hWidget);
	//hLayout = new QHBoxLayout(hWidget);
	hLayout->setSpacing (0);
	hLayout->setMargin (0);

	m_instructionsOnTrue = new Sequence(scheme, hWidget, this);
	m_instructionsOnFalse = new Sequence(scheme, hWidget, this);

	hLayout->addWidget (m_instructionsOnTrue, 0, 0);
        hLayout->setColumnStretch (0, 1);
	//hLayout->setStretchFactor (m_instructionsOnTrue, 0);
	hLayout->addWidget (m_instructionsOnFalse, 0, 1);
        hLayout->setColumnStretch (1, 1);
	//hLayout->setStretchFactor (m_instructionsOnFalse, 0);

	label = new SelectionCondition(this);
	label->setVisible (true);
	p = label->palette ();
	p.setBrush (QPalette::Background, QBrush(Qt::white));
	label->setPalette (p);

        splitter->setOrientation (Qt::Vertical);
	splitter->addWidget (label);
        splitter->addWidget (hWidget);
        splitter->setChildrenCollapsible (false);

        //vLayout->addWidget (label);
	//vLayout->setStretchFactor (label, 1);
	//vLayout->addWidget (hWidget);
	//vLayout->setStretchFactor (hWidget, 5);
	vLayout->addWidget (splitter);
        vLayout->setStretchFactor(splitter, 1);

	if (scheme) {
		connect (this, SIGNAL(instructionActivated(Instruction*)), 
			 scheme, SLOT(on_instructionActivated(Instruction*)));
	}
}

Selection::~Selection()
{
}

void Selection::showEvent (QShowEvent *e)
{
	Q_UNUSED(e);
#ifdef DEBUG4
	qDebug("[ENTER] Selection::showEvent");
#endif
	label->setVisible (true);
	m_instructionsOnFalse->setVisible (true);
	m_instructionsOnTrue->setVisible (true);
#ifdef DEBUG4
	qDebug("[EXIT] Selection::showEvent");
#endif
}

Sequence* Selection::instructionsOnFalse () 
{
	return m_instructionsOnFalse;
}

void Selection::setInstructionsOnFalse (Sequence* s)
{
#ifdef DEBUG4
	qDebug("[ENTER] Selection::setInstructionsOnFalse");
#endif
        if (! s)
            return;

	if (m_instructionsOnFalse) {
		QLayoutItem *p = hLayout->takeAt (1);
		delete p; 
		delete m_instructionsOnFalse;
		m_instructionsOnFalse = 0;
	}

	m_instructionsOnFalse = s;

	if (m_instructionsOnFalse) {
		m_instructionsOnFalse->setScheme (scheme ());
		m_instructionsOnFalse->setParentInstruction (this);
		m_instructionsOnFalse->setParent (hWidget);
		m_instructionsOnFalse->setVisible (false);
		hLayout->addWidget (m_instructionsOnFalse, 0, 1);
                hLayout->setColumnStretch (1, 1);
		//hLayout->setStretchFactor (m_instructionsOnFalse, 0);
		adjustSize ();
		hLayout->update ();
	} else {
		qDebug("strange, we dont have any sequence");
	}
#ifdef DEBUG4
	qDebug("[EXIT] Selection::setInstructionsOnFalse");
#endif
}

Sequence* Selection::instructionsOnTrue () 
{
	return m_instructionsOnTrue;
}

void Selection::setInstructionsOnTrue (Sequence* s)
{
#ifdef DEBUG4
	qDebug("[ENTER] Selection::setInstructionsOnTrue");
#endif
        if (! s)
            return;

	if (m_instructionsOnTrue) {
		QLayoutItem *p = hLayout->takeAt (0);
		delete p;
		delete m_instructionsOnTrue;
		m_instructionsOnTrue = 0;
	}

	m_instructionsOnTrue = s;

	if (m_instructionsOnTrue) {
		m_instructionsOnTrue->setScheme (scheme ());
		m_instructionsOnTrue->setParentInstruction (this);
		m_instructionsOnTrue->setParent (hWidget);
		m_instructionsOnTrue->setVisible (false);
		hLayout->addWidget (m_instructionsOnTrue, 0, 0);
		//hLayout->insertWidget (0, m_instructionsOnTrue);
                hLayout->setColumnStretch (0, 1);
		//hLayout->setStretchFactor (m_instructionsOnTrue, 0);
		adjustSize ();
		hLayout->update ();
	} else {
		qDebug("strange, we dont have any sequence");
	}
#ifdef DEBUG4
	qDebug("[EXIT] Selection::setInstructionsOnTrue");
#endif
}

void Selection::setContents (QString newContents)
{
	m_contents = newContents;
	label->setText (m_contents);
        if ((m_instructionsOnFalse->count() == 0) && (m_instructionsOnTrue->count () == 0)) {
            splitter->setSizes(QList<int> () << label->minimumSizeHint().height() << height() - label->minimumSizeHint().height());
        }
}

void Selection::setComment (QString newComment)
{
	m_comment = newComment;
	label->setComment (m_comment);
        if ((m_instructionsOnFalse->count() == 0) && (m_instructionsOnTrue->count () == 0)) {
            splitter->setSizes(QList<int> () << label->minimumSizeHint().height() << height() - label->minimumSizeHint().height());
        }
}

void Selection::setPixmap (QPixmap newImage)
{
	m_pixmap = newImage;
}

void Selection::formatXMLNode (QDomDocument& document, QDomNode& parent)
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
	
	QDomElement ontrue = document.createElement ("ontrue");
	m_instructionsOnTrue->formatXMLNode (document, ontrue);

	QDomElement onfalse = document.createElement ("onfalse");
	m_instructionsOnFalse->formatXMLNode (document, onfalse);

	QList<int> heights = splitter->sizes ();
	QDomAttr cheight = document.createAttribute ("conditionheight");
	cheight.setValue (QString::number (heights.at (0)));
	QDomAttr iheight = document.createAttribute ("instructionsheight");
	iheight.setValue (QString::number (heights.at (1)));
	
	QDomElement selection = document.createElement ("selection");
	selection.setAttributeNode (cheight);
	selection.setAttributeNode (iheight);
	selection.appendChild (condition);
	selection.appendChild (ontrue);
	selection.appendChild (onfalse);

	parent.appendChild (selection);
}

void Selection::formatSVGNode (QDomDocument& document, QDomNode& parent)
{
	int h = splitter->sizes ().at (0) + splitter->handleWidth ();
	QPoint p = positionInScheme ();
	QDomElement g = document.createElement ("g");
	if (! m_comment.isEmpty ())
		g.appendChild (createSVGTextNode (document, p.x () + 20, h / 3 + p.y () + label->fontInfo ().pixelSize () / 2, m_comment));
	g.appendChild (createSVGTextNode (document, p.x () + this->width () / 2, h / 3 + p.y () + (m_comment.isEmpty () ? 1 : 2) * label->fontInfo ().pixelSize () / 2, m_contents, Qt::AlignHCenter));
	g.appendChild (createSVGTextNode (document, p.x () + 10, p.y () + h - label->fontInfo ().pixelSize () / 2, SelectionCondition::tr("YES")));
	g.appendChild (createSVGTextNode (document, p.x () + this->width () - 10, p.y () + h - label->fontInfo ().pixelSize () / 2, SelectionCondition::tr("NO"), Qt::AlignRight));
	g.appendChild (createSVGLine (document, p.x (), p.y (), (int)(p.x () + instructionsSplit () * this->width ()), p.y () + h));
	g.appendChild (createSVGLine (document, (int)(p.x () + instructionsSplit () * this->width ()), p.y () + h, p.x () + this->width (), p.y ()));
	g.appendChild (createSVGLine (document, p.x (), p.y () + h, p.x () + this->width (), p.y () + h));
	g.appendChild (createSVGLine (document, p.x () + this->width () / 2, p.y () + h, p.x () + this->width () / 2, p.y () + this->height ()));
	g.appendChild (createSVGRect (document));
	
	if (m_instructionsOnTrue)
		m_instructionsOnTrue->formatSVGNode (document, g);
	if (m_instructionsOnFalse)
		m_instructionsOnFalse->formatSVGNode (document, g);
	
	parent.appendChild (g);
}

bool Selection::setAsXMLNode (QDomNode& node)
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
				} else if (e.tagName () == "ontrue") {
					if (e.hasChildNodes () && e.firstChild ().nodeName () == "sequence") {
						QDomNode n = e.firstChild ();
						retval = m_instructionsOnTrue->setAsXMLNode (n);
					}
				} else if (e.tagName () == "onfalse") {
					if (e.hasChildNodes () && e.firstChild ().nodeName () == "sequence") {
						QDomNode n = e.firstChild ();
						retval = m_instructionsOnFalse->setAsXMLNode (n);
					}
				}
			}
		}
	}

	QList<int> sizes;
	QString ch = node.toElement ().attributeNode ("conditionheight").value ();
	if (! ch.isEmpty ()) {
		sizes.append (ch.toInt ());
	} else {
		sizes.append (label->minimumSizeHint ().height ());
	}
	QString ih = node.toElement ().attributeNode ("instructionsheight").value ();
	if (! ih.isEmpty ()) {
		sizes.append (ih.toInt ());
	} else {
		int h = m_instructionsOnTrue->minimumSizeHint ().height ();
		if (m_instructionsOnFalse->minimumSizeHint ().height () > h)
			h = m_instructionsOnFalse->minimumSizeHint ().height ();
		sizes.append (h);
	}
	splitter->setSizes (sizes);

	return retval;
}

Instruction* Selection::copyOf ()
{
	Selection *retval = new Selection(0,0,0,false);

	retval->setContents (m_contents);
	retval->setComment (m_comment);
	retval->setPixmap (m_pixmap);

	Instruction *i;
	i = m_instructionsOnFalse->copyOf ();
	//qDebug(i->metaObject ()->className ());
	retval->setInstructionsOnFalse (dynamic_cast<Sequence*>(i));
	i = m_instructionsOnTrue->copyOf ();
	//qDebug(i->metaObject ()->className ());
	retval->setInstructionsOnTrue (dynamic_cast<Sequence*>(i));

	retval->splitter->setSizes (splitter->sizes ());

	return retval;
}

void Selection::setScheme (NSScheme *scheme)
{
	Instruction::setScheme (scheme);

	/*for (int i = 0; i < m_instructionsOnTrue->count (); i++) {
		m_instructionsOnTrue->instructionAt (i)->setScheme (scheme);
	}
	
	for (int i = 0; i < m_instructionsOnFalse->count (); i++) {
		m_instructionsOnFalse->instructionAt (i)->setScheme (scheme);
	}*/
	m_instructionsOnTrue->setScheme (scheme);
	m_instructionsOnFalse->setScheme (scheme);
}

QSize Selection::minimumSizeHint () const
{
	QSize ontrue, onfalse;
	
	ontrue = m_instructionsOnTrue->minimumSizeHint ();
	onfalse = m_instructionsOnFalse->minimumSizeHint ();
	
	int w = qMax(ontrue.width () + onfalse.width (), label->minimumSizeHint ().width ());
	int h = qMax(ontrue.height (), onfalse.height ()) + label->minimumSizeHint ().height ();
	
	return QSize(w, h);
}

double Selection::instructionsSplit ()
{
	/*qDebug(QString("onFalse=%1 onTrue=%2")
			.arg((long)(void*)m_instructionsOnFalse, 0, 16)
			.arg((long)(void*)m_instructionsOnTrue, 0, 16)
			.toLocal8Bit ());*/

	int falseWidth = 1, trueWidth = 1;
	
	if (m_instructionsOnFalse != 0)
		falseWidth = m_instructionsOnFalse->width ();

	if (m_instructionsOnTrue != 0)
		trueWidth = m_instructionsOnTrue->width ();

	return 1.0 *  trueWidth / (trueWidth + falseWidth);
}

Instruction* Selection::execute (ExecutionThread *executor, bool *wait)
{
	Q_UNUSED(executor);
	Q_UNUSED(wait);

	setRunning (true);
	
	Instruction *i = 0;
	
	if (m_valid && statement) {
		// Wyznacz wartosc wyrazenia logicznego
                BaseValue b = execute_statement (this, statement->syntacticTree (), scheme ()->variables ());
		
		i = ((b.d == Long) ? b.val : b.fval) ? m_instructionsOnTrue : m_instructionsOnFalse;
	}

	setRunning (false);

	return i;
}

bool Selection::validateContents ()
{
	do_validate (QSet<typ_skladnika>() << BWYR << ATOM_LOG << OPER_NOT << OPER_LOG << OPER_REL);

	return m_valid;
}

void Selection::recursiveValidateContents ()
{
        validateContents ();
        m_instructionsOnFalse->recursiveValidateContents ();
        m_instructionsOnTrue->recursiveValidateContents ();
}
