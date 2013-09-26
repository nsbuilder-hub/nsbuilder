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

#ifndef __NSSCHEMEWIZARD_H
#define __NSSCHEMEWIZARD_H

#include "ui_nsschemewizard.h"

#include <QDialog>
#include <QWidget>
#include <QString>

class NSScheme;

class NSSchemeWizard : public QDialog {
	Q_OBJECT

	public:
	enum SchemeType {SEQUENCE, PROCEDURE, FUNCTION};
		
	public:
	NSSchemeWizard(QWidget *parent);
	
	void reset ();
	
	QString schemeAuthor ();
	void setSchemeAuthor (const QString& s);

	QString schemeName ();
	void setSchemeName (const QString& s);
	
	QString schemeComment ();
	void setSchemeComment (const QString& s);
	
	NSSchemeWizard::SchemeType schemeType ();
	void setSchemeType (NSSchemeWizard::SchemeType t);
	
	void setContainsPascalCode (bool v);
	bool containsPascalCode ();

	void setCheckSyntax (bool v);
	bool checkSyntax ();

	static QString typeToString (NSSchemeWizard::SchemeType t);
	static NSSchemeWizard::SchemeType stringToType (const QString &s);
	
	void fillDataFrom (NSScheme *s);
	void setDataTo (NSScheme *s);

	private slots:
	void on_sequenceRB_clicked (bool v);
	void on_procedureRB_clicked (bool v);
	void on_functionRB_clicked (bool v);
	void on_schemeNameLE_textChanged (const QString &s);
	void on_schemeCommentTE_textChanged ();
	void on_schemeAuthorLE_textChanged (const QString &s);
	void on_checkSyntaxCB_clicked (bool v);
	void on_pascalCodeGB_clicked (bool v);

	private:
	QString m_schemeAuthor;
	QString m_schemeName;
	QString m_schemeComment;
	SchemeType m_schemeType;
	bool m_pascalCode;
	bool m_checkSyntax;
	Ui::NSSchemeWizard ui;
};

extern NSSchemeWizard *schemeWizard;

#endif

