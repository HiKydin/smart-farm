#ifndef DIALOG_DELETE_H
#define DIALOG_DELETE_H

#include <QDialog>

namespace Ui {
class Dialog_delete;
}

class Dialog_delete : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog_delete(QWidget *parent = 0);
    ~Dialog_delete();

private slots:
    void on_btn_close_clicked();
    void on_btn_delete_clicked();

signals:
    void sendDelete(QString name,QString date);

private:
    Ui::Dialog_delete *ui;
};

#endif // DIALOG_DELETE_H
