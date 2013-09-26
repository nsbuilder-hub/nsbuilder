#ifndef RETURNINSTRUCTION_H
#define RETURNINSTRUCTION_H

class QWidget;
class NSScheme;

#include "instruction.h"

class ReturnInstruction : public Instruction {
    Q_OBJECT

    public:
    ReturnInstruction(NSScheme *scheme, QWidget *parent=0, Instruction *parentInstruction=0, bool visible=true);

    virtual void setContents (QString newContents);
    virtual void setComment (QString newComment);
    virtual void setPixmap (QPixmap newImage);

    virtual void formatXMLNode (QDomDocument& document, QDomNode& parent);
    virtual bool setAsXMLNode (QDomNode& element);
    virtual void formatSVGNode (QDomDocument& document, QDomNode& parent);

    virtual Instruction* copyOf ();

    virtual Instruction* execute (ExecutionThread *executor, bool *wait);

    virtual bool validateContents ();
    virtual void recursiveValidateContents ();

    protected:
    void paintEvent (QPaintEvent *e);

    private:
    QLabel *label;
    BaseValue result;
};

#endif
