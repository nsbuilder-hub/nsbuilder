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

#ifndef INSTRUCTIONEDITORFORM_H
#define INSTRUCTIONEDITORFORM_H

#include "ui_instructioneditor.h"

class InstructionEditorForm : public QWidget {
	Q_OBJECT

	public:
	InstructionEditorForm(QWidget *parent);

	QPixmap pixmap ();
	void setPixmap (const QPixmap &p);
	QString instructionText ();
	void setInstructionText (const QString &s);
	QString instructionComment ();
	void setInstructionComment (const QString &s);

	void reset ();

	public slots:
	void on_instructionEdit_textChanged (const QString & text);
	void on_commentEdit_textChanged (const QString & text);

	signals:
	void instructionTextChanged (const QString &text);
	void instructionCommentChanged (const QString &text);
	void instructionReady (bool isReady);

	private:
	Ui::InstructionEditorForm ui;
	QString m_instructionText;
	QString m_instructionComment;
	QPixmap m_pixmap;
};

extern InstructionEditorForm *editorForm;

#endif

