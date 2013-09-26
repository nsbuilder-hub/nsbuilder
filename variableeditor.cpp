#include "variableeditor.h"

#include "nsscheme.h"
#include "bisonproxy.h"

VariableEditor::VariableEditor(QWidget *parent)
:QDialog(parent)
{
	ui.setupUi (this);
	p_scheme = 0;
}

void VariableEditor::editSchemeVariables (NSScheme *scheme)
{
	// poprzedni schemat
	if (p_scheme) {
		disconnect (p_scheme->variables (), SIGNAL(valueChanged(const QString&)), this, SLOT(on_valueChanged(const QString&)));
	}

	p_scheme = scheme;
	
	readSchemeVariables ();

	connect (p_scheme->variables (), SIGNAL(valueChanged(const QString&)), this, SLOT(on_valueChanged(const QString&)));

	show ();
}

void VariableEditor::readSchemeVariables ()
{
	if (p_scheme == 0)
		return;

	IdentsMap *map = p_scheme->variables ()->map ();

	ui.tableWidget->setRowCount (map->count ());
	ui.tableWidget->clear ();
	
	int row = 0;
	foreach (QString n, map->keys ()) {
		ident_val_t *id = map->value (n);
		Q_ASSERT(id != 0);
		if ((id->t.tc == Variable) 
		     || (id->t.tc == Array)) {
			QTableWidgetItem *col0 = new QTableWidgetItem(n);
			col0->setFlags (col0->flags () & ! Qt::ItemIsEditable);
			ui.tableWidget->setItem (row, 0, col0);
			QTableWidgetItem *col1 = new QTableWidgetItem(id->t.toString ());
			col1->setFlags (col1->flags () & ! Qt::ItemIsEditable);
			ui.tableWidget->setItem (row, 1, col1);
			ui.tableWidget->setItem (row, 2, new QTableWidgetItem(id->valueToString ()));
			++row;
		}
	}
}

void VariableEditor::on_valueChanged (const QString &varName)
{
	int i;
	for (i = 0; (i < ui.tableWidget->rowCount ()) && (ui.tableWidget->item (i, 0)->text () != varName); ++i)
		;
	if (i < ui.tableWidget->rowCount ()) {
                updateFromScheme = true;
                ui.tableWidget->setItem (i, 2, new QTableWidgetItem(p_scheme->variables ()->byName (varName)->valueToString ()));
        }
}

void VariableEditor::on_tableWidget_itemChanged (QTableWidgetItem * item)
{
        if (! isVisible () || updateFromScheme) {
                updateFromScheme = false;
                return;
        }

	int row = ui.tableWidget->row (item);
	int col = ui.tableWidget->column (item);
	
	if (col == 2) {
		ident_val_t *id = p_scheme->variables ()->map ()->value (
					ui.tableWidget->item (row, 0)->text ()
				  );

		if (! id->setValueFromString (item->text ())) {
			QMessageBox::warning (this, tr("Conversion error"), tr("There is problem with converting value"));
		}
	}
}

