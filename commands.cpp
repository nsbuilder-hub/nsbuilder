#include "commands.h"
#include <QApplication>
#include <QDebug>

Commands::Commands()
{
}

Action* Action::newAddInstruction (Sequence *s, Instruction *i, int index)
{
    Action *a = new Action;
    a->parentSequence = s;
    a->instr = i;
    a->index = index;
    a->type = ADD;
    return a;
}

Action* Action::newRemoveInstruction (Sequence *s, Instruction *i, int index)
{
    Action *a = new Action;
    a->parentSequence = s;
    a->instr = i;
    a->index = index;
    a->type = REMOVE;
    return a;
}

Action* Action::newEditInstruction (Instruction *i, QString prevContents, QString newContents, QString prevComment, QString newComment)
{
    Action *a = new Action;
    a->instr = i;
    a->prevContents = prevContents;
    a->newContents = newContents;
    a->newComment = newComment;
    a->prevComment = prevComment;
    a->type = EDIT;
    return a;
}

void Action::execute()
{
    qDebug() << "Action::execute" << type;
    switch (type) {
        case ADD:
            parentSequence->insertInstruction (instr, index);
        break;
        case REMOVE:
            instr = parentSequence->instructionAt (index);
            parentSequence->removeInstructionAt (index);
        break;
        case EDIT:
            instr->setContents (newContents);
            instr->setComment (newComment);
        break;
    }
}

void Action::reverse()
{
    switch (type) {
        case ADD:
            parentSequence->removeInstructionAt (index);
        break;
        case REMOVE:
            parentSequence->insertInstruction (instr, index);
        break;
        case EDIT:
            instr->setContents (prevContents);
        break;
    }
}

QString Action::toString()
{
    switch (type) {
        case ADD:
            return QApplication::tr("Add instruction");
        break;
        case REMOVE:
            return QApplication::tr("Remove isntruction");
        break;
        case EDIT:
            return QApplication::tr("Edit instruction");
        break;
        default:
            return QString::null;
    }
}

bool Commands::isEmpty ()
{
    return actionStack.isEmpty();
}

void Commands::doAction (Action *a)
{
    a->execute();
    actionStack.push(a);
}

void Commands::undoAction ()
{
    if (actionStack.isEmpty())
        return;

    Action *a = actionStack.pop ();
    a->reverse();
    delete a;
}

QString Commands::lastActionString ()
{
    QString result = QApplication::tr("(empty)");

    if (! actionStack.isEmpty())
        result = actionStack.top ()->toString();

    return result;
}
