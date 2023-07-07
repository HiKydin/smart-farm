#include "dialog_seach.h"
#include "ui_dialog_seach.h"
#include "flatui.h"

Dialog_seach::Dialog_seach(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog_seach)
{
    ui->setupUi(this);

    this->setFixedSize(300,100);

    //使用QSS样式库美化界面
    //按钮区:
    FlatUI::setPushButtonQss(ui->btn_seach, 5, 8, "#1ABC9C", "#E6F8F5", "#2EE1C1", "#FFFFFF", "#16A086", "#A7EEE6");
    FlatUI::setPushButtonQss(ui->btn_close, 5, 8, "#E74C3C", "#FFFFFF", "#EC7064", "#FFF5E7", "#DC2D1A", "#F5A996");
    //文本框：
    FlatUI::setLineEditQss(ui->lineEdit);
}

Dialog_seach::~Dialog_seach()
{
    delete ui;
}

void Dialog_seach::on_btn_close_clicked()
{
    this->close();
}

void Dialog_seach::on_btn_seach_clicked()
{
    emit sendSeach(ui->lineEdit->text());
    this->close();
}
