#ifndef COMMANDS_H
#define COMMANDS_H

#include <QString>
#include <QStack>

#include "sequence.h"
#include "instruction.h"

class Action {
public:
    static Action* newAddInstruction (Sequence *s, Instruction *i, int index);
    static Action* newRemoveInstruction (Sequence *s, Instruction *i, int index);
    static Action* newEditInstruction (Instruction *i, QString prevContents, QString newContents, QString prevComment, QString newComment);

    void execute();
    void reverse();
    QString toString();
private:
    Sequence *parentSequence;
    Instruction *instr;
    int index;
    QString newContents;
    QString prevContents;
    QString prevComment;
    QString newComment;
    enum ActionType { ADD, REMOVE, EDIT };
    ActionType type;
};

class Commands
{
public:
    Commands();
    bool isEmpty ();
    void doAction (Action *a);
    void undoAction ();
    QString lastActionString ();
private:
    QStack<Action*> actionStack;
};

#endif // COMMANDS_H
