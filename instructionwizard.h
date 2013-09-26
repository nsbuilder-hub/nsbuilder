/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation and appearing in the file LICENSE.GPL    *
 *   included in the packaging of this file.                               *
 *                                                                         *
 ***************************************************************************/

/*
 * \author Aleksander Wojdyga <awojdyga@pluton.pol.lublin.pl>
 *
 * http://pluton.pol.lublin.pl/~awojdyga/nsbuilder/
 */

class MainWindow;
class NSScheme;

#include "ui_instructionwizard.h"
#include "instructioneditor.h"

class InstructionWizardDialog : public QDialog {
	Q_OBJECT

	public:
	enum InstructionType { Clipboard = 1, Instruction = 2, Selection = 4, Iteration = 8, 
                               Input = 16, Output = 32, ProcedureCall = 64, FunctionValueReturn = 128};
		
	InstructionWizardDialog(MainWindow *parent = 0);

	QPixmap pixmap ();
	void setPixmap (const QPixmap &p);

	QString instructionText ();
	void setInstructionText (const QString &s);

	QString instructionComment ();
	void setInstructionComment (const QString &s);

	int getInstructionType ();

        void reset (NSScheme *s);

	public slots:
	int execute ();

	private slots:
	void on_prevButton_clicked ();
	void on_nextButton_clicked ();
	void on_instructionReady (bool isReady);
	void on_finishButton_clicked ();

	private:
	Ui::InstructionWizardDialog ui;
	::InstructionEditorForm *instructionEditor;

	MainWindow *p_mainWindow;
};

extern ::InstructionWizardDialog *instructionWizard;

