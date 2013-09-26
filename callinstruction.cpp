#include "callinstruction.h"
#include "nsscheme.h"
#include "executionthread.h"
#include "imp_yacc.h"

#include <QMessageBox>
#include <QHBoxLayout>
#include <QPainter>

CallInstruction::CallInstruction(NSScheme *scheme, QWidget *parent, Instruction *parentInstruction, bool visible)
:Instruction(scheme, parent, parentInstruction, visible)
{
	setObjectName ("call instruction");
	
	QPalette p = palette ();
	p.setBrush (QPalette::Background, QBrush(Qt::white));
	setPalette (p);

	QHBoxLayout *layout = new QHBoxLayout(this);
	layout->setSpacing (0);
	layout->setMargin (0/*4*/);

	label = new QLabel(this);
	label->setTextFormat (Qt::RichText);
	label->setText (tr("empty"));
	label->setEnabled (false);
	label->setVisible (visible);
	p = label->palette ();
	p.setBrush (QPalette::Background, QBrush(Qt::white));
	label->setPalette (p);

	layout->addSpacing (15);
	layout->addWidget (label);
	layout->addSpacing (15);
	layout->setStretchFactor (label, 0);

	if (scheme) {
		connect (this, SIGNAL(instructionActivated(Instruction*)), 
			 scheme, SLOT(on_instructionActivated(Instruction*)));
	}
}

void CallInstruction::setContents (QString newContents)
{
	m_contents = newContents;
	label->setText (labelTemplate
			.arg (m_comment.isEmpty () ? QString::null : escape (m_comment).append ("<br/>"))
			.arg (escape (m_contents)));
	label->adjustSize ();
	adjustSize ();
	layout ()->update ();
}

void CallInstruction::setComment (QString newComment)
{
	m_comment = newComment;
	label->setText (labelTemplate
			.arg (m_comment.isEmpty () ? QString::null : escape (m_comment).append ("<br/>"))
			.arg (escape (m_contents)));
	label->adjustSize ();
	adjustSize ();
	layout ()->update ();
}

void CallInstruction::setPixmap (QPixmap newImage)
{
	m_pixmap = newImage;
}

void CallInstruction::formatXMLNode (QDomDocument& document, QDomNode& parent)
{
	QDomText textContents = document.createTextNode (m_contents);
	QDomElement text = document.createElement ("text");
	text.appendChild (textContents);

	QDomText commentContents = document.createTextNode (m_comment);
	QDomElement comment = document.createElement ("comment");
	comment.appendChild (commentContents);

	QDomElement e = document.createElement ("callinstruction");
	e.appendChild (text);
	e.appendChild (comment);
	parent.appendChild (e);
}

void CallInstruction::formatSVGNode (QDomDocument& document, QDomNode& parent)
{
	QPoint p = positionInScheme ();
	QDomElement g = document.createElement ("g");
	g.appendChild (createSVGTextNode (document, p.x () + 20, this->height () / 2 + p.y () + label->fontInfo ().pixelSize () / 2, m_contents));
	if (! m_comment.isEmpty ())
		g.appendChild (createSVGTextNode (document, p.x () + 20, this->height () / 2 + p.y () - label->fontInfo ().pixelSize () / 2, m_comment));
	g.appendChild (createSVGLine (document, p.x () + 10, p.y () + 0, p.x () + 10, p.y () + this->height ()));
	g.appendChild (createSVGLine (document, p.x () + this->width () - 10, p.y () + 0, p.x () + this->width () - 10, p.y () + this->height ()));
	g.appendChild (createSVGRect (document));
	parent.appendChild (g);
}

bool CallInstruction::setAsXMLNode (QDomNode& node)
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

void CallInstruction::paintEvent (QPaintEvent *e)
{
	Instruction::paintEvent (e);

	QPainter p(this);
        //p.fillRect (QRect(1, 1, this->width () - 2, this->height () - 2), Qt::white);
	p.drawLine (10, 0, 10, this->height ());
	p.drawLine (this->width () - 10, 0, this->width () - 10, this->height ());
}

Instruction* CallInstruction::copyOf ()
{
	CallInstruction *retval = new CallInstruction(0,0,0,false);

	retval->setContents (m_contents);
	retval->setComment (m_comment);
	retval->setPixmap (m_pixmap);

	return retval;
}

void associateArguments (ProgramVariables *callVariables, drzewo_skladn *call, NSScheme *procScheme, NSScheme *callerScheme)
{
    QStringList schemeArgs = procScheme->schemeArguments ();
    ProgramVariables *schemeVars = procScheme->variables ();
    drzewo_skladn *d = call;

    if (procScheme != callerScheme)
        procScheme->assertArgumentsRefValues();

    foreach (QString n, schemeArgs) {
            drzewo_skladn *x=0;
            if (d->typ == LISTA_LWYR
                || d->typ == LISTA_WARTOSCI) {
                    x = d->skladnik[0];
            } else /*if (d->typ == ATOM_IDENT
                       || d->typ == WYR_INDEKS)*/ {
                    x = d;
            }

            if (procScheme->isArgumentByRef(n)) {
                if (x->typ == ATOM_IDENT) {
                    if (x && x->zmienna && !(x->zmienna->ident.isEmpty ()) && (procScheme != callerScheme)) {
                        // tu się odbywa łączenie zmiennych
                        ident_val_t *zm = callVariables->byName (x->zmienna->ident);
                        //schemeVars->map ()->insert (n, zm);
                        schemeVars->byName(n)->setValue(zm);
#ifdef DEBUGX
                        qDebug(QString("act arg name=%1 (addr=%3 value=%4) formal arg name=%2")
                                        .arg(x->zmienna->ident)
                                        .arg(n)
                                        .arg((ulong) (void *) (x->zmienna), 0, 16)
                                        .arg(zm->valueToString ())
                                        .toLocal8Bit ());
#endif
                    } else {
                        qDebug(QString("no value for argument %1 or this scheme is called").arg(n).toLocal8Bit());
                    }
                } else if (x->typ == WYR_INDEKS) {
                    BaseValue ind = execute_statement(0, x->skladnik[2], callVariables);
                    VarIndex i = VarIndex::integerIndex (ind.toInt());
                    qDebug() << "get indexed variable value" << x->skladnik[0] << ind.toInt();
                    BaseValue& refbv = callVariables->getIndexedVariableValue(x->zmienna->ident, i);
                    BaseValue bv(Ref, &refbv);
                    //schemeVars->map ()->value (n)->v.bval.setValue (bv);
                    schemeVars->byName (n)->setValue (bv);
                }
            } else {
                    BaseValue bv = execute_statement(0, x, callVariables);
                    schemeVars->setVariableValue(n, bv);
#ifdef DEBUGX
                    qDebug() << QString("variable %1 passed by value %2").arg(n).arg(schemeVars->byName (n)->valueToString());
#endif
            }
            d = d->skladnik[1];
    }
#ifdef DEBUGX
    IdentsMap::const_iterator it = schemeVars->map ()->constBegin();
    while (it != schemeVars->map ()->constEnd()) {
            qDebug () << it.key() << "@" << it.value() << "value" << it.value ()->valueToString ();
            ++it;
    }
#endif
}

void CallInstruction::showEvent (QShowEvent *e)
{
        Q_UNUSED(e);
        label->setVisible (true);
}

Instruction* CallInstruction::execute (ExecutionThread *executor, bool *wait)
{
	Q_UNUSED(wait);

	Instruction *i = 0;

        if (m_valid && statement) {
                // znajdz schemat o ktorym mowa
                NSScheme *procScheme;
                qDebug(QString("have to find scheme %1").arg(statement->syntacticTree ()->zmienna->ident).toLocal8Bit ());
                //if ((procScheme = mainWin->findSchemeForName (statement->syntacticTree ()->zmienna->ident))) {
                if ((procScheme = functionMap.getProcedureSchemeForName(statement->syntacticTree ()->zmienna->ident))) {
                        setRunning (true);
                        // zachowaj nastepna instrukcje, zapamiataj stare wskazniki -> utworz ramke w watku wykonania
                        executor->saveExecutionFrame (nextInstruction (), this, procScheme);                        

                        associateArguments(this->scheme ()->variables (), statement->syntacticTree ()->skladnik[2], procScheme, scheme());

                        // Znajdz instrukcja ktorej dotyczy wywolanie
                        i = procScheme->instructions ()->instructionAt (0);
                } else {
                        qDebug("no scheme procedure to call !");
                }
        }

	// Zwroc instrukcje do wykonania
	return i;
}

bool CallInstruction::validateContents ()
{
	do_validate (QSet<typ_skladnika>() << INSTR_PROC << ATOM_IDENT);
        if (m_valid && statement && statement->syntacticTree ()) {
		statement->syntacticTree ()->typ = INSTR_PROC;
	}

	return m_valid;
}

void CallInstruction::recursiveValidateContents ()
{
        validateContents ();
}
