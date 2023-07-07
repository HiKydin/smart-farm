#ifndef DIALOG_SEACH_H
#define DIALOG_SEACH_H

#include <QDialog>

namespace Ui {
class Dialog_seach;
}

class Dialog_seach : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog_seach(QWidget *parent = 0);
    ~Dialog_seach();

private slots:
    void on_btn_close_clicked();

    void on_btn_seach_clicked();

signals:
    void sendSeach(QString name);

private:
    Ui::Dialog_seach *ui;
};

#endif // DIALOG_SEACH_H
