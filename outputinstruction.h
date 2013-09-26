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

#ifndef OUTPUTINSTRUCTION_H
#define OUTPUTINSTRUCTION_H

class QWidget;
class NSScheme;

#include "instruction.h"

class OutputInstruction : public Instruction {
	Q_OBJECT

	public:
	OutputInstruction(NSScheme *scheme, QWidget *parent=0, Instruction *parentInstruction=0, bool visible=true);

	virtual void setContents (QString newContents);
	virtual void setComment (QString newComment);
	virtual void setPixmap (QPixmap newImage);

	virtual void formatXMLNode (QDomDocument& document, QDomNode& parent);
	virtual bool setAsXMLNode (QDomNode& element);
	virtual void formatSVGNode (QDomDocument& document, QDomNode& parent);

	virtual QSize minimumSizeHint () const;

	/**
	 * Wykonuje instrukcję i zwraca następną instrukcję do wykonania.
	 */
	virtual Instruction* execute (ExecutionThread *executor, bool *wait);

	virtual bool validateContents ();
        virtual void recursiveValidateContents ();

        QString syntacticTreeValueToString (drzewo_skladn *t, ProgramVariables *vars);
	
	protected:
	void paintEvent (QPaintEvent *e);
	void showEvent (QShowEvent *e);

	virtual Instruction * copyOf ();

	private:
	QLabel *label;
};

#endif

