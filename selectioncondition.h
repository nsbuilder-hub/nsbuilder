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

#ifndef SELECTIONCONDITION_H
#define SELECTIONCONDITION_H

#include <QWidget>
#include <QLabel>
#include <QSize>

class Selection;

class SelectionCondition : public QWidget {
	Q_OBJECT

	public:
	SelectionCondition(Selection *parent);

	void setText (const QString &s);
	void setComment (const QString &s);
	
	virtual QSize minimumSizeHint () const;

	protected:
	void paintEvent (QPaintEvent *e);
	void resizeEvent (QResizeEvent *event);
	void showEvent (QShowEvent *e);

	private:
	void do_paintLabel ();

	QSize boundRect;
	QLabel *label;
	
	QString m_text;
	QString m_comment;
	
	Selection *p_parent;
};

#endif

