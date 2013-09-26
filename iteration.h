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

#ifndef ITERATION_H
#define ITERATION_H

#include <QString>
#include <QWidget>
#include <QLabel>

#include "iterationwidget.h"
#include "instruction.h"
#include "sequence.h"

class Iteration : public Instruction {
	Q_OBJECT
public:
        enum Variant {WHILEDO, REPEATUNTIL};
	/**
	 * 
	 */
        Iteration(NSScheme *scheme, Variant var=WHILEDO, QWidget *parent=0, Instruction *parentInstruction=0, bool visible=true);
	
	Sequence* instructions ();
	
	void setInstructions (Sequence* s);
	
	/**
	 * 
	 * @param newContents 
	 */
	virtual void setContents (QString newContents);

	/**
	 * 
	 * @param newComment 
	 */
	virtual void setComment (QString newComment);

	/**
	 * 
	 * @param newImage 
	 */
	virtual void setPixmap (QPixmap newImage);

	virtual void formatXMLNode (QDomDocument& document, QDomNode& parent);
	virtual bool setAsXMLNode (QDomNode& node);
	virtual void formatSVGNode (QDomDocument& document, QDomNode& parent);

	virtual Instruction * copyOf ();

	virtual void setScheme (NSScheme *scheme);
	
	virtual QSize minimumSizeHint () const;

	/**
	 * Wykonuje instrukcję i zwraca następną instrukcję do wykonania.
	 */
	virtual Instruction* execute (ExecutionThread *executor, bool *wait);

	virtual bool validateContents ();
        virtual void recursiveValidateContents ();
protected:
	virtual void paintEvent (QPaintEvent *e);
	virtual void showEvent (QShowEvent *e);
private:
	/**
	 * 
	 */
	Sequence *m_instructions;
	QLabel *label;
	QHBoxLayout *hLayout;
	QWidget *iWidget;
        Variant m_variant;
};

#endif //ITERATION_H

