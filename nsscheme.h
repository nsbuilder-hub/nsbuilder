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

#ifndef NSSCHEME_H
#define NSSCHEME_H

#include <QString>
#include <QWidget>
#include <QMessageBox>
#include <QHttp>

class MainWindow;

#include "instruction.h"
#include "sequence.h"
#include "mainwindow.h"
#include "nsschemewizard.h"
#include "exprtree.h"
#include "commands.h"

class NSScheme : public QWidget {
	Q_OBJECT
	Q_PROPERTY (QString schemeAuthor READ schemeAuthor() WRITE setSchemeAuthor())
	Q_PROPERTY (QString schemeName READ schemeName() WRITE setSchemeName())
	Q_PROPERTY (QString schemeComment READ schemeComment() WRITE setSchemeComment())
	Q_PROPERTY (bool containsPascalCode READ containsPascalCode() WRITE setContainsPascalCode())
	Q_PROPERTY (bool checkSyntax READ checkSyntax() WRITE setCheckSyntax())
        Q_PROPERTY (NSSchemeWizard::SchemeType schemeType READ schemeType() WRITE setSchemeType())
	Q_ENUMS(NSSchemeWizard::SchemeType)
	
	public:
	NSScheme(MainWindow *parent);
	~NSScheme();

        bool isClosing ();

	QString schemeAuthor () const;
	void setSchemeAuthor (const QString& author);
			
	QString schemeName () const;
	void setSchemeName (QString name);

	QString schemeComment () const;
	void setSchemeComment (QString newComment);

	bool containsPascalCode ();
	void setContainsPascalCode (bool v);

	bool checkSyntax ();
	void setCheckSyntax (bool v);

	NSSchemeWizard::SchemeType schemeType () const;
	void setSchemeType (NSSchemeWizard::SchemeType t);

	Sequence* instructions ();
	void setInstructions (Sequence *s);
	
	void appendInstruction ();
	void insertInstruction ();
	void editInstruction ();
	void removeInstruction ();

        bool hasUndoneCommands ();

	void editVariables ();
	void exportScheme ();
	void exportSchemeSVG ();
	void putSchemeSVG ();
	void putSchemeNSS ();
	void emailScheme ();

	QDomDocument toXMLDocument ();
	QDomDocument toSVGDocument ();
	bool fromXMLDocument (QDomDocument& document);

	void newFile ();
	QString currentFile () const;
	QString userFriendlyCurrentFile () const;
	bool loadFile (const QString& fileName);
	bool save ();
	bool saveAs ();
	bool modified ();

        void undo ();
	void cut ();
	void copy ();
	void paste ();

	QColor invalidColor ();
	QColor selectionColor ();
	QColor executionColor ();

	ProgramVariables* variables ();
	/* Zwraca argumenty dla schematu będącego procedurą lub funkcją. Dla
         * sekwencji instrukcji zwraca pustą listę
         */
	QStringList schemeArguments ();
        /* Zwraca prawdę jeśli argument jest przekazywany przez referencję (zmienną)
         */
        bool isArgumentByRef (QString name);

        /* Ustawia typ wartości dla argumentów przekazywanych przez referencję */
        void assertArgumentsRefValues ();

	void assignArraySizes ();

        QString argumentsType();
        void setFunctionReturnValue (BaseValue bv);
        BaseValue functionValue ();

	signals:
	void closing ();
	
	public slots:
	void on_instructionActivated (Instruction *i);
	void getInput(ExecutionThread *executor, const QString& type, const QString& caption, const QString& message);
	void showMessage (ExecutionThread *executor, const QString& caption, const QString& message);
	void on_executionThread_finished ();     
	
	protected:
	void setModified (bool v);
	virtual void closeEvent (QCloseEvent * e);

	private slots:
	void showSyntaxError ();
	void httpRequestFinished (int, bool);
	void onDataReadProgress (int done, int total);
	void onDataSendProgress (int done, int total);
        void on_indexOutOfBounds (const QString& varName);

	private:
	void updateHeaderLabel ();
        void updateProceduralLabel ();
	Sequence* findParentSequence ();
	Instruction* createInstructionFromWizard ();
	void initQHttp ();
	bool do_svgExport (QString svgFileName, QString &errorString);
	void startPutFile (QString file);
        QString nameValueMapToAppURL ();

	Instruction *p_activeInstruction;
	MainWindow *p_mainParent;
	QString m_schemeName;
	QString m_schemeComment;
	NSSchemeWizard::SchemeType m_schemeType;
	
	QLabel *headerLabel;
        QLabel *proceduralLabel;
	Sequence *m_instructions;
	ProgramVariables *m_variables;
        BaseValue functionReturnValue;
        Commands commands;

	QString m_fileName;
	bool m_isUntitled;
	bool m_modified;
        bool m_closing;

	QString m_author;

	bool m_pascalCode;
	bool m_checkSyntax;

	static int sequenceNumber;
	
	QFile *putFile;
	int reqID;
	QHttp *wwwput;
};

QString escape (const QString s);

const QString headerTemplate = "<html><head><meta name=\"qrichtext\" content=\"1\" /></head><body style=\"white-space: pre-wrap; font-family:Lucida Sans; font-weight:400; font-style:normal; text-decoration:none;\"><p style=\" font-style:italic; color:#000000; align=right\">%1</p><p style=\"color:#222222;\">%2</p></body></html>";
const QString labelTemplate = "<html><head><meta name=\"qrichtext\" content=\"1\" /></head><body style=\"white-space: pre-wrap; font-family:Lucida Sans; font-weight:400; font-style:normal; text-decoration:none;\"><span style=\" font-style:italic; color:#a9a9a9;\">%1</span><span style=\"color:#000000;\">%2</span></body></html>";
const QString proceduralTemplate = "<html><head><meta name=\"qrichtext\" content=\"1\" /></head><body style=\"white-space: pre-wrap; font-family:Lucida Sans; font-weight:400; font-style:normal; text-decoration:none;\"><p style=\" font-style:italic; color:#000000; align=right\">%1</p><p style=\"color:#222222;\">%2</p><p style=\"background-color: white; color:#000000; border: 1px solid black; padding-top: 3px; pading-bottom: 3px\">%3 <span style=\"font-weight:bold\">%4</span></p></body></html>";
#endif

