#ifndef MANAGEMENT_H
#define MANAGEMENT_H

#include <QMainWindow>
#include <QDebug>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QFileDialog>
#include <QDir>
#include <QMap>
#include <qmqtt.h>
#include <QHostInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QStringListModel>
#include <QSound>

#include "barchar.h"
#include "piechart.h"
#include "mainwindow.h"
#include "dialog_seach.h"
#include "dialog_delete.h"

#define ON 1
#define OFF 0
#define DISABLE -2

#define CONTROL 0
#define GET 1

#define FAN 0
#define LED 1
#define PUMP 2
#define BUZZER 3

#define PUBLISH_TOPIC "smart_farm_upToSub"

namespace Ui {
class Management;
}

class Management : public QMainWindow
{
    Q_OBJECT

public:
    explicit Management(QWidget *parent = 0);
    ~Management();
    void saveBarData();
    void reflashBar();
    void reflashData();
    void importData();
    void outData();
    void addItem();
    void seach();
    void deletePage();
    int connectSQL();
    void connectMQTT();
    void logread(QString msg);
    void MQTT_publish(QString value);
    QSqlDatabase db;
    QMap<int, QString> map;
    QMap<QString,int> map2;
    QMap<QString, int> pie_map;
    QMap<QString, QString> device_ip;
    QMap<QString, int> device_bind;

private slots:
    void on_btn_sql_connect_clicked();
    void on_btn_sql_disconnect_clicked();
    void on_btn_mqtt_connect_clicked();
    void on_btn_mqtt_disconnect_clicked();
    void on_btn_mqtt_subscribe_clicked();
    void mqtt_recv_msg(QMQTT::Message msg);
    void mqtt_connect_info();
    void mqtt_disconnect_info();
    void mqtt_sub_success(QString topic,quint8 qos);
    void on_btn_mqtt_publish_clicked();
    void seachData(QString name);
    void deleteData(QString name,QString date);
    void setFan(QString deviceID,bool op);
    void setLED(QString deviceID,bool op);
    void setPump(QString deviceID,bool op);
    void setBuzzer(QString deviceID,bool op);
    void on_btn_getStatus_clicked();
    int Parse_Json(QString msg);
    bool containsItem(const QString& text);
    void alert();
    QString getDeviceID();

private:
    Ui::Management *ui;
    MainWindow *addPage;
    Dialog_seach *seachPage;
    Dialog_delete *deleteDia;
    QMQTT::Client *mqtt;
    QPixmap pix;//状态图标
    barchar *bar; //条形统计图
    piechart *pie;//饼状图
    int doAlert = ON;
    int canPlay = DISABLE;
    QSound *alertSound;
    QSound *clickSound;
    QStringList fanUseList;
    QStringList ledUseList;
    QStringList pumpUseList;
    QStringList buzzerUseList;
};

#endif // MANAGEMENT_H
