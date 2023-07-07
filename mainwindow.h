#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QCoreApplication>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QByteArray>
#include <QDebug>
#include <QMessageBox>
#include <QJsonParseError>
#include <QVariantMap>
#include <QVariant>
#include <QBuffer>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QFile>
#include <QPainter>
#include <QColor>
#include <QRectF>
#include <QImage>
#include <QPixmap>
#include <QMovie>
#include <QTimer>
#include <QSqlQuery>
#include <QDateTime>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_pushButton_clicked();
    void http_rst(QString url_str, QNetworkAccessManager *manager);
    void readdata_slot(QNetworkReply *reply);
    void download_slot(QNetworkReply *reply);
    void photo_slot(QNetworkReply *reply);
    void GenerateQRcode(QString tempstr);
    void update();
    void on_btn_add_clicked();
    void on_lineEdit_editingFinished();

signals:
    reflash();

private:
    Ui::MainWindow *ui;
    QNetworkAccessManager *manager;             //Http管理对象
    QNetworkAccessManager *manager_download;
    QNetworkAccessManager *manager_photo;
    QTimer *timer;
    int dot = 0;
    QString date_am = "am";
    QString date_pm = "pm";
};

#endif // MAINWINDOW_H
