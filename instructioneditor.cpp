#include "instructioneditor.h"

#include <QMessageBox>

InstructionEditorForm *editorForm = 0;

InstructionEditorForm::InstructionEditorForm(QWidget *parent)
:QWidget(parent)
{
	ui.setupUi (this);
}

void InstructionEditorForm::on_instructionEdit_textChanged (const QString & text)
{
	m_instructionText = text;
	emit instructionTextChanged (text);
	bool ready = (! ui.instructionEdit->text ().isEmpty ()) 
		|| (! ui.commentEdit->text ().isEmpty ()) 
		|| (! m_pixmap.isNull ());
	emit instructionReady (ready);
}

void InstructionEditorForm::on_commentEdit_textChanged (const QString & text)
{
	m_instructionComment = text;
	emit instructionCommentChanged (text);
	bool ready = ! ui.instructionEdit->text ().isEmpty () 
		|| ! ui.commentEdit->text ().isEmpty () 
		|| ! m_pixmap.isNull ();
	emit instructionReady (ready);
}

QPixmap InstructionEditorForm::pixmap ()
{
	return m_pixmap;
}

void InstructionEditorForm::setPixmap (const QPixmap &p)
{
	m_pixmap = p;
	bool ready = ! ui.instructionEdit->text ().isEmpty () 
		|| ! ui.commentEdit->text ().isEmpty () 
		|| ! m_pixmap.isNull ();
	emit instructionReady (ready);
}

QString InstructionEditorForm::instructionText ()
{
	/* TAK NAPRAWDE NIE MAM POJECIA O LEXie I YACCu ... */
	return m_instructionText.replace (QRegExp(",([^ ])"), ", \\1");
}

void InstructionEditorForm::setInstructionText (const QString &s)
{
	m_instructionText = s;
	ui.instructionEdit->setText (s);
}

QString InstructionEditorForm::instructionComment ()
{
	return m_instructionComment;
}

void InstructionEditorForm::setInstructionComment (const QString &s)
{
	m_instructionComment = s;
	ui.commentEdit->setText (s);
}

void InstructionEditorForm::reset ()
{
	setInstructionText ("");
	setInstructionComment ("");
	setPixmap (QPixmap());
}

