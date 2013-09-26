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

#ifndef SEQUENCE_H
#define SEQUENCE_H

#include <QString>
#include <QVBoxLayout>
#include <QPaintEvent>

class NSScheme;

#include "instruction.h"

class Sequence : public Instruction {
	Q_OBJECT
public:
	/**
	 * 
	 */
	Sequence(NSScheme *scheme, QWidget *parent=0, Instruction *parentInstruction=0, bool visible=true);
	/**
	 * 
	 */
	int count ();
	
	/**
	 * 
	 * @param num 
	 */
	Instruction* instructionAt (int num);
		
	/**
	 * 
	 * @param i 
	 */
	void appendInstruction (Instruction *i);
		
	/**
	 * 
	 * @param i 
	 * @param index 
	 */
	void insertInstruction (Instruction *i, int index=0);
	
	/**
	 *
	 */
	void removeInstructionAt (int index=0);
	
	/**
	 *
	 */
	void removeInstruction (Instruction *i);
	
	/**
	 *
	 */
	int indexOf (Instruction *i);

	/**
	 *
	 */
	Instruction* nextInstruction (Instruction *i);

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
	
	/**
	 * Zwraca i ustawia zawsze prawdę
	 */
	virtual bool validateContents ();
        virtual void recursiveValidateContents ();
protected:
	/**
	 * 
	 */
	virtual void paintEvent (QPaintEvent *e);
private:
	/**
	 * 
	 */
	QVBoxLayout *m_layout;
	QVector<Instruction*> m_instructions;
};
#endif //SEQUENCE_H

