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

#ifndef ITERATIONWIDGET_H
#define ITERATIONWIDGET_H

#include <QWidget>
#include <QLabel>

class IterationWidget : public QWidget {
	public:
	IterationWidget(QWidget *parent=0, const char * name=0);
	QLabel *contentsLabel;
	QWidget *instructionsWidget;
};

#endif

