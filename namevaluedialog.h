#ifndef NAMEVALUEDIALOG_H
#define NAMEVALUEDIALOG_H

#include <QDialog>
#include <QMap>
#include <QString>

namespace Ui
{
class NameValueDialog;
}

class NameValueDialog : public QDialog
{
    Q_OBJECT

public:
    explicit NameValueDialog(QWidget *parent = 0);
    ~NameValueDialog();

    QMap<QString, QString>& nameValueMap();

private slots:
    void on_accept();
    void on_addTB_clicked();
    void on_delTB_clicked();

private:
    Ui::NameValueDialog *ui;
    QMap<QString, QString> m_map;
};

#endif // NAMEVALUEDIALOG_H
