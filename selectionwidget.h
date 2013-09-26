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

#ifndef SELECTIONWIDGET_H
#define SELECTIONWIDGET_H

#include <QLabel>
#include <QWidget>

class SelectionWidget : public QWidget {
	public:
	SelectionWidget(QWidget *parent=0, const char * name=0);

	QLabel *label;
	QWidget *onTrueWidget;
	QWidget *onFalseWidget;
};

#endif

