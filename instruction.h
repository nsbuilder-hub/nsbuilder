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

#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include <QString>
#include <QPixmap>
#include <QLabel>
#include <QDomNode>

#include "bisonproxy.h"
#include "exprtree.h"

class NSScheme;
class ExecutionThread;

class Instruction : public QWidget {
	Q_OBJECT
	Q_PROPERTY (QString contents READ contents() WRITE setContents())
	Q_PROPERTY (QString comment READ comment() WRITE setComment())
	Q_PROPERTY (QPixmap pixmap READ pixmap() WRITE setPixmap())
	Q_PROPERTY (NSScheme* scheme READ scheme() WRITE setScheme())
	Q_PROPERTY (bool active READ isActive() WRITE setActive())
	Q_PROPERTY (Instruction* parentInstruction READ parentInstruction() WRITE setParentInstruction())	
	Q_PROPERTY (bool running READ isRunning() WRITE setRunning())
	
public:
	/**
	 * 
	 */
	Instruction(NSScheme *scheme, QWidget *parent=0, Instruction *parentInstruction=0, bool visible=false);
	/**
	 * 
	 */
	virtual QString contents ();
		
	/**
	 * 
	 * @param newContents 
	 */
	virtual void setContents (QString newContents) = 0;
	
	/**
	 * 
	 */
	virtual QString comment ();
	
	/**
	 * 
	 * @param newComment 
	 */
	virtual void setComment (QString newComment) = 0;
		
	/**
	 * 
	 */
	virtual QPixmap& pixmap ();
		
	/**
	 * 
	 * @param newImage 
	 */
	virtual void setPixmap (QPixmap newImage) = 0;
	
	NSScheme* scheme () const;

	virtual void setScheme (NSScheme *scheme);

	Instruction* parentInstruction () const;

	void setParentInstruction (Instruction *parent);
	
	/**
	 * 
	 * @param parent
	 */
	virtual void formatXMLNode (QDomDocument& document, QDomNode& parent) = 0;
	
	/**
	 * 
	 * @param element
	 */
	virtual bool setAsXMLNode (QDomNode& element) = 0;
	
	/**
	 *
	 * @param parent
	 * @param document
	 */
	virtual void formatSVGNode (QDomDocument& document, QDomNode& parent) = 0;
	
	bool isActive () const;

	void setActive (bool v);

	virtual Instruction* copyOf () = 0;

	/**
	 * Wykonuje instrukcję i zwraca następną instrukcję do wykonania.
	 * @param executor wątek wykonujący tą instrukcję
	 * @param wait czy instrukcja życzy sobie zatrzymać wykonanie (np.
	 * przy instrukcji wejścia)
	 */
	virtual Instruction* execute (ExecutionThread *executor, bool *wait) = 0;

	/**
	 * Sprawdza czy ustawiona zawartość jest poprawna dla tego typu
	 * instrukcji, ustawia pole valid
	 */
	virtual bool validateContents () = 0;

        /**
         * Sprawdza rekurencyjnie poprawność tej instrukcji oraz instrukcji zagnieżdżonych.
         * Instrukcje proste nie mają instrukcji zagnieżdżonych.
         */
        virtual void recursiveValidateContents () = 0;

	Instruction* nextInstruction ();

	bool valid ();

	QString toString ();

	bool isRunning ();
	void setRunning (bool b);
	
	QPoint positionInScheme ();
        QString alignmentToAnchor (Qt::AlignmentFlag f);
        QDomElement createSVGTextNode (QDomDocument &document, int x, int y, const QString& text, Qt::AlignmentFlag f = Qt::AlignLeft);
        QDomElement createSVGRect (QDomDocument &document);
        QDomElement createSVGLine (QDomDocument &document, int x1, int y1, int x2, int y2);

public slots:
        void on_newCompoundStatement (const QString &name);
        void on_compoundStatementDeleted (const QString &name);

signals:
	void instructionActivated (Instruction *i);
	void runningChanged ();

protected:
	QString m_contents;
	QString m_comment;
	QPixmap m_pixmap;

	/**
	 * 
	 */
	virtual void mousePressEvent (QMouseEvent *e);
	virtual void mouseDoubleClickEvent (QMouseEvent * event);
	virtual void paintEvent (QPaintEvent *e);

	void do_validate (const QSet<enum typ_skladnika> &types);
	
	ProgramStatement *statement;

	bool m_valid;

	bool m_running;
	
        bool isFirstInSequence ();

	QColor backgroundColor ();
private:
	/**
	 * 
	 */
	NSScheme *p_scheme;
	Instruction *p_parentInstruction;
	bool m_active;

private slots:
	void on_runningChanged ();
};
#endif //INSTRUCTION_H

