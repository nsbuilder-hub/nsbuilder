#include "instructionwizard.h"

#include "mainwindow.h"
#include "instruction.h"
#include "simpleinstruction.h"
#include "inputinstruction.h"
#include "outputinstruction.h"
#include "callinstruction.h"
#include "selection.h"
#include "iteration.h"
#include "returninstruction.h"
#include "nsscheme.h"
#include "nsschemewizard.h"

InstructionWizardDialog *instructionWizard;

InstructionWizardDialog::InstructionWizardDialog(MainWindow *parent)
    :QDialog(parent)
{
    ui.setupUi(this);

    p_mainWindow = parent;

    instructionEditor = new ::InstructionEditorForm(ui.contentsFrame);
    ui.contentsFrame->layout()->addWidget(instructionEditor);

    ui.nextButton->setShortcut(QKeySequence(Qt::Key_PageDown));    
}

void InstructionWizardDialog::on_prevButton_clicked()
{
    int i = ui.tabWidget->currentIndex() - 1;

    if (i >= 0) {
        ui.tabWidget->setCurrentIndex(i);
    }
}

void InstructionWizardDialog::on_nextButton_clicked()
{
    int i = ui.tabWidget->currentIndex() + 1;

    if (i < ui.tabWidget->count()) {
        ui.tabWidget->setCurrentIndex(i);
    }
}

void InstructionWizardDialog::on_instructionReady(bool isReady)
{
    ui.finishButton->setEnabled(isReady);    
}

void InstructionWizardDialog::reset(NSScheme *s)
{
    qDebug() << "InstructionWizardDialog::reset()";
    ui.tabWidget->setCurrentIndex(0);
    instructionEditor->reset();
    ui.basicButton->setChecked(true);
    ui.returnButton->setEnabled(s->schemeType() == NSSchemeWizard::FUNCTION);
}

int InstructionWizardDialog::execute()
{
    int result;

    connect(instructionEditor, SIGNAL(instructionReady(bool)),
             this, SLOT(on_instructionReady(bool)));
    result = exec();
    disconnect(instructionEditor, SIGNAL(instructionReady(bool)),
                this, SLOT(on_instructionReady(bool)));

    return result;
}

void InstructionWizardDialog::on_finishButton_clicked()
{
    accept();
}

bool isInstruction(Instruction *i) { return(qobject_cast<SimpleInstruction*>(i) != 0); }
bool isSelection(Instruction *i) { return qobject_cast<Selection*>(i) != 0; }
bool isIteration(Instruction *i) { return qobject_cast<Iteration*>(i) != 0; }
bool isInput(Instruction *i) { return qobject_cast<InputInstruction*>(i) != 0; }
bool isOutput(Instruction *i) { return qobject_cast<OutputInstruction*>(i) != 0; }
bool isCall(Instruction *i) { return qobject_cast<CallInstruction*>(i) != 0; }
bool isReturn(Instruction *i) { return qobject_cast<ReturnInstruction*>(i) != 0; }

int InstructionWizardDialog::getInstructionType()
{
    int retval = 0;

    if (p_mainWindow->clipboardInstruction() && (ui.clipboardButton->isChecked())) {
        qDebug("clipboardButton->isChecked()");
        retval = (int) Clipboard;
        return retval;
    }

    if (ui.basicButton->isChecked()) {
        qDebug("ui.basicButton->isChecked()");
        retval = Instruction;
    } else if (ui.selectionButton->isChecked()) {
        qDebug("ui.selectionButton->isChecked()");
        retval = Selection;
    } else if (ui.iterationButton->isChecked()) {
        qDebug("ui.iterationButton->isChecked()");
        retval = Iteration;
    } else if (ui.inputButton->isChecked()) {
        qDebug("ui.inputButton->isChecked()");
        retval = Input;
    } else if (ui.outputButton->isChecked()) {
        qDebug("ui.outputButton->isChecked()");
        retval = Output;
    } else if (ui.callButton->isChecked()) {
        qDebug("ui.callButton->isChecked()");
        retval = ProcedureCall;
    } else if (ui.returnButton->isChecked()) {
        qDebug("ui.returnButton->isChecked()");
        retval = FunctionValueReturn;
    }

    return retval;
}

QPixmap InstructionWizardDialog::pixmap()
{
    return instructionEditor->pixmap();
}

void InstructionWizardDialog::setPixmap(const QPixmap &p)
{
    instructionEditor->setPixmap(p);
}

QString InstructionWizardDialog::instructionText()
{
    return instructionEditor->instructionText();
}

void InstructionWizardDialog::setInstructionText(const QString &s)
{
    instructionEditor->setInstructionText(s);
}

QString InstructionWizardDialog::instructionComment()
{
    return instructionEditor->instructionComment();
}

void InstructionWizardDialog::setInstructionComment(const QString &s)
{
    instructionEditor->setInstructionComment(s);
}

