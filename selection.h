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

#ifndef SELECTION_H
#define SELECTION_H

#include <QLabel>
#include <QSplitter>

#include "instruction.h"
#include "sequence.h"
#include "selectionwidget.h"
#include "selectioncondition.h"

class Selection : public Instruction {
	Q_OBJECT
public:
	virtual ~Selection();
	/**
	 * 
	 */
	Selection(NSScheme *scheme, QWidget *parent=0, Instruction *parentInstruction=0, bool visible=true);

	/**
	 * 
	 */
	Sequence* instructionsOnFalse ();
		
	/**
	 * 
	 */
	void setInstructionsOnFalse (Sequence* value);
	
	/**
	 * 
	 */
	Sequence* instructionsOnTrue ();
		
	/**
	 * 
	 */
	void setInstructionsOnTrue (Sequence* value);
	
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

	friend class SelectionCondition;
protected:
	virtual void showEvent (QShowEvent *e);

	double instructionsSplit ();

private:
	/**
	 * 
	 */
	//QLabel *label;
	SelectionCondition *label;
	QWidget *onTrueWidget;
	QWidget *onFalseWidget;
	Sequence *m_instructionsOnFalse;
	Sequence *m_instructionsOnTrue;
	QGridLayout *hLayout;
	//QHBoxLayout *hLayout;
	QSplitter *splitter;
	QWidget *hWidget;
};
#endif //SELECTION_H

