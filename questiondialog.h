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

#include "ui_questiondialog.h"

class QuestionDialog : public QDialog {
	Q_OBJECT

	public:
	QuestionDialog(QWidget *parent = 0);

	void setWidget (QWidget *w);

	static void initWidgets (QWidget *parent);
	static int showEditorForm (const QString &title = QString::null);
	static int showSchemeWizard (const QString &title = QString::null);
	
	private:
	Ui::QuestionDialog ui;
};

