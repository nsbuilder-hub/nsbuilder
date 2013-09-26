#include "questiondialog.h"
#include "instructionwizard.h"
#include "nsschemewizard.h"

#include <QDebug>

QuestionDialog *editorFormQD;
QuestionDialog *schemeWizardQD;

QuestionDialog::QuestionDialog(QWidget *parent)
:QDialog(parent)
{
	ui.setupUi (this);
}

void QuestionDialog::setWidget (QWidget *w)
{
	ui.contentsFrame->addWidget (w);
	ui.contentsFrame->setCurrentWidget (w);
}

void QuestionDialog::initWidgets (QWidget *parent)
{
	editorForm = new InstructionEditorForm(0);
	editorForm->setVisible (false);
	editorFormQD = new QuestionDialog(parent);
	editorFormQD->setWindowTitle (tr("Edit instruction"));
	editorFormQD->setWidget (editorForm);
	//editorFormQD->hide ();

	schemeWizard = new NSSchemeWizard(0);
	schemeWizard->setVisible (false);
	schemeWizardQD = new QuestionDialog(parent);
	schemeWizardQD->setWindowTitle (tr("NS scheme wizard"));
	schemeWizardQD->setWidget (schemeWizard);
	//schemeWizardQD->hide ();
}

int QuestionDialog::showEditorForm (const QString &title)
{
	int result;
#ifdef DEBUG4
	qDebug()<< "[ENTER] QuestionDialog::showEditorForm";
#endif
	editorForm->setVisible (true);
	if (! title.isEmpty ()) {
		QString t = editorFormQD->windowTitle ();
		editorFormQD->setWindowTitle (title);
		result = editorFormQD->exec ();
		editorFormQD->setWindowTitle (t);
	} else
		result = editorFormQD->exec ();
#ifdef DEBUG4
	qDebug()<< "[EXIT] QuestionDialog::showEditorForm";
#endif
	return result;
}

int QuestionDialog::showSchemeWizard (const QString &title)
{
	int result;
#ifdef DEBUG4
	qDebug()<< "[ENTER] QuestionDialog::showSchemeWizard";
#endif
	schemeWizard->setVisible (true);
	if (! title.isEmpty ()) {
		QString t = schemeWizardQD->windowTitle ();
		schemeWizardQD->setWindowTitle (title);
		result = schemeWizardQD->exec ();
		schemeWizardQD->setWindowTitle (t);
	} else
		result = schemeWizardQD->exec ();
#ifdef DEBUG4
	qDebug()<< "[EXIT] QuestionDialog::showSchemeWizard";
#endif
	return result;
}

