#include "returninstruction.h"
#include "nsscheme.h"
#include "executionthread.h"
#include "imp_yacc.h"

#include <QHBoxLayout>
#include <QPainter>

ReturnInstruction::ReturnInstruction(NSScheme *scheme, QWidget *parent, Instruction *parentInstruction, bool visible)
    :Instruction(scheme, parent, parentInstruction, visible)
{
    setObjectName ("function value instruction");

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

    layout->addSpacing (40);
    layout->addWidget (label);

    if (scheme) {
            connect (this, SIGNAL(instructionActivated(Instruction*)),
                     scheme, SLOT(on_instructionActivated(Instruction*)));
    }

}

void ReturnInstruction::setContents (QString newContents)
{
    m_contents = newContents;
    label->setText (labelTemplate
                    .arg (m_comment.isEmpty () ? QString::null : escape (m_comment).append ("<br/>"))
                    .arg (escape (m_contents)));
    label->adjustSize ();
    adjustSize ();
    layout ()->update ();
}

void ReturnInstruction::setComment (QString newComment)
{
    m_comment = newComment;
    label->setText (labelTemplate
                    .arg (m_comment.isEmpty () ? QString::null : escape (m_comment).append ("<br/>"))
                    .arg (escape (m_contents)));
    label->adjustSize ();
    adjustSize ();
    layout ()->update ();
}

void ReturnInstruction::setPixmap (QPixmap newImage)
{
    m_pixmap = newImage;
}

void ReturnInstruction::formatXMLNode (QDomDocument& document, QDomNode& parent)
{
    QDomText textContents = document.createTextNode (m_contents);
    QDomElement text = document.createElement ("text");
    text.appendChild (textContents);

    QDomText commentContents = document.createTextNode (m_comment);
    QDomElement comment = document.createElement ("comment");
    comment.appendChild (commentContents);

    QDomElement e = document.createElement ("returninstruction");
    e.appendChild (text);
    e.appendChild (comment);
    parent.appendChild (e);
}

bool ReturnInstruction::setAsXMLNode (QDomNode& node)
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

void ReturnInstruction::formatSVGNode (QDomDocument& document, QDomNode& parent)
{
    QPoint p = positionInScheme ();
    QDomElement g = document.createElement ("g");
    g.appendChild (createSVGTextNode (document, p.x () + 20, this->height () / 2 + p.y () + label->fontInfo ().pixelSize () / 2, m_contents));
    if (! m_comment.isEmpty ())
            g.appendChild (createSVGTextNode (document, p.x () + 20, this->height () / 2 + p.y () - label->fontInfo ().pixelSize () / 2, m_comment));
    g.appendChild (createSVGLine (document, p.x (), p.y () + this->height() / 2, p.x () + 40, p.y ()));
    g.appendChild (createSVGLine (document, p.x (), p.y () + this->height() / 2, p.x () + 40, p.y () + this->height ()));
    g.appendChild (createSVGRect (document));
    parent.appendChild (g);
}

Instruction* ReturnInstruction::copyOf ()
{
    ReturnInstruction *retval = new ReturnInstruction(0,0,0,false);

    retval->setContents (m_contents);
    retval->setComment (m_comment);
    retval->setPixmap (m_pixmap);

    return retval;
}

Instruction* ReturnInstruction::execute (ExecutionThread *executor, bool *wait)
{
    Q_UNUSED(wait);
    Q_UNUSED(executor);

    setRunning (true);

    if (m_valid && statement) {
            ProgramVariables *vars = scheme ()->variables ();
            drzewo_skladn *t = statement->syntacticTree ();
            if (t) {
                result = execute_statement(this, t, vars);
                scheme ()->setFunctionReturnValue (result);
                qDebug() << "return value=" << result.toString();
            }
    }

    setRunning (false);

    return nextInstruction ();
}

bool ReturnInstruction::validateContents ()
{
        do_validate (QSet<typ_skladnika>() << ATOM_LICZBA << ATOM_IDENT << OPER_ARYTM << WYR_INDEKS << FUNC_CALL << LISTA_WARTOSCI << LISTA_LWYR);
        if (m_valid && statement && statement->syntacticTree ()) {
                //statement->syntacticTree ()->typ = ATOM_LICZBA;
        }

        return m_valid;
}

void ReturnInstruction::recursiveValidateContents ()
{
        validateContents ();
}

void ReturnInstruction::paintEvent (QPaintEvent *e)
{
    Instruction::paintEvent (e);

    QPainter p(this);
    p.drawLine (0, this->height () / 2, 40, 0);
    p.drawLine (0, this->height () / 2, 40, this->height ());
}
