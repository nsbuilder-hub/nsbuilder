#ifndef __VARIABLEEDITOR_H
#define __VARIABLEEDITOR_H

class NSScheme;
class QWidget;

#include <QDialog>

#include "ui_variableeditor.h"

class VariableEditor: public QDialog {
	Q_OBJECT

	public:
	VariableEditor(QWidget *parent);

	void editSchemeVariables (NSScheme *scheme);

	public slots:
	void on_tableWidget_itemChanged (QTableWidgetItem * item);
	
	private slots:
	void on_valueChanged (const QString &varName);

	private:
	Ui::VariableEditorForm ui;

	NSScheme *p_scheme;
	void readSchemeVariables ();

        /* czy zmienna zostala zmieniona przez uruchomiony schemat (true)
         * czy przez usera recznie (false)
         * ustawia to slot on_valueChanged () */
        bool updateFromScheme;
};

#endif

