#include "namevaluedialog.h"
#include "ui_namevaluedialog.h"

#include <QItemSelectionModel>

NameValueDialog::NameValueDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NameValueDialog)
{
    ui->setupUi(this);
    connect(this, SIGNAL(accepted()), this, SLOT(on_accept()));
}

NameValueDialog::~NameValueDialog()
{
    delete ui;
}

void NameValueDialog::on_addTB_clicked()
{
    ui->tableWidget->setRowCount(ui->tableWidget->rowCount() + 1);
}

void NameValueDialog::on_delTB_clicked()
{
    QItemSelectionModel * selection = ui->tableWidget->selectionModel();
    QModelIndexList indexes = selection->selectedIndexes();
    if (indexes.count() > 0) {
        int index = indexes.front().row();
        ui->tableWidget->removeRow(index);
    }
}

QMap<QString, QString>& NameValueDialog::nameValueMap()
{
    return m_map;
}

void NameValueDialog::on_accept()
{
    m_map.clear();
    for (int i = 0; i < ui->tableWidget->rowCount(); i++) {
        QTableWidgetItem *keyItem = ui->tableWidget->item(i, 0);
        QTableWidgetItem *valueItem = ui->tableWidget->item(i, 1);

        m_map.insert(keyItem->text(), valueItem->text());
    }
}

