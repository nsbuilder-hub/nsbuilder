#ifndef NAVEVALUEDIALOG_H
#define NAVEVALUEDIALOG_H

#include <QDialog>
#include <QMap>
#include <QString>

namespace Ui {
    class NaveValueDialog;
}

class NaveValueDialog : public QDialog
{
    Q_OBJECT

public:
    explicit NaveValueDialog(QWidget *parent = 0);
    ~NaveValueDialog();

    QMap<QString, QString>& nameValueMap ();

private slots:
    void on_accept ();
    void on_addTB_clicked ();
    void on_delTB_clicked ();

private:
    Ui::NaveValueDialog *ui;
    QMap<QString, QString> m_map;
};

#endif // NAVEVALUEDIALOG_H
