#include "dialog_delete.h"
#include "ui_dialog_delete.h"
#include "flatui.h"

Dialog_delete::Dialog_delete(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog_delete)
{
    ui->setupUi(this);

    this->setFixedSize(300,120);

    //使用QSS样式库美化界面
    //按钮区:
    FlatUI::setPushButtonQss(ui->btn_delete, 5, 8, "#1ABC9C", "#E6F8F5", "#2EE1C1", "#FFFFFF", "#16A086", "#A7EEE6");
    FlatUI::setPushButtonQss(ui->btn_close, 5, 8, "#E74C3C", "#FFFFFF", "#EC7064", "#FFF5E7", "#DC2D1A", "#F5A996");
    //文本框：
    FlatUI::setLineEditQss(ui->lineEdit_name);
    FlatUI::setLineEditQss(ui->lineEdit_date);
}

Dialog_delete::~Dialog_delete()
{
    delete ui;
}

void Dialog_delete::on_btn_close_clicked()
{
    this->close();
}

void Dialog_delete::on_btn_delete_clicked()
{
    emit sendDelete(ui->lineEdit_name->text(),ui->lineEdit_date->text());
    this->close();
}
