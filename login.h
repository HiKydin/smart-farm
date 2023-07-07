#ifndef LOGIN_H
#define LOGIN_H

#include <QWidget>
#include <QString>
#include <QDebug>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QMessageBox>
#include <QPainter>

const QString Account = "admin";
const QString Passwd = "admin";

class Login : public QWidget
{
    Q_OBJECT
public:
    explicit Login(QWidget *parent = nullptr);
    void paintEvent(QPaintEvent *);//重写绘图事件
};

#endif // LOGIN_H
