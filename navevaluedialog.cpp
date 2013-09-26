#include "navevaluedialog.h"
#include "ui_navevaluedialog.h"

#include <QItemSelectionModel>

NaveValueDialog::NaveValueDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NaveValueDialog)
{
    ui->setupUi(this);
    connect(this, SIGNAL(accepted()), this, SLOT(on_accept()));
}

NaveValueDialog::~NaveValueDialog()
{
    delete ui;
}

void NaveValueDialog::on_addTB_clicked ()
{
    ui->tableWidget->setRowCount(ui->tableWidget->rowCount() + 1);
}

void NaveValueDialog::on_delTB_clicked ()
{
    QItemSelectionModel * selection = ui->tableWidget->selectionModel ();
    QModelIndexList indexes = selection->selectedIndexes ();
    if (indexes.count() > 0) {
        int index = indexes.front().row();
        ui->tableWidget->removeRow(index);
    }
}

QMap<QString, QString>& NaveValueDialog::nameValueMap ()
{
    return m_map;
}

void NaveValueDialog::on_accept ()
{
    m_map.clear();
    for (int i = 0; i < ui->tableWidget->rowCount(); i++) {
        QTableWidgetItem *keyItem = ui->tableWidget->item (i, 0);
        QTableWidgetItem *valueItem = ui->tableWidget->item (i, 1);

        m_map.insert(keyItem->text(), valueItem->text());
    }
}

