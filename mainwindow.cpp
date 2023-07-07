 #include "mainwindow.h"
#include "ui_mainwindow.h"
#include "qrcode/qrencode.h"
#include "flatui.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    /*******************界面配置*************************/

    ui->setupUi(this);
    this->setWindowTitle("植物百科");
    //工程文件中写了RC_ICONS就不需要添加页面的icon
    //this->setWindowIcon(QIcon("://icon.png"));
    this->ui->btn_add->setDisabled(true);

    //使用QSS样式库美化界面
    //按钮区:
    FlatUI::setPushButtonQss(ui->pushButton);
    FlatUI::setPushButtonQss(ui->btn_add, 5, 8, "#d5d8dc", "#E6F8F5", "#2EE1C1", "#FFFFFF", "#16A086", "#A7EEE6");
    //文本框：
    FlatUI::setLineEditQss(ui->lineEdit);
    FlatUI::setLineEditQss(ui->lineEdit_date);

    //创建图表
    QStringList list;
    list << "参数" << "值" ;//设置列数为2列,这句一定要写,否则表头不显示
    ui->tableWidget->setColumnCount(2);//设置表头
    ui->tableWidget->setHorizontalHeaderLabels(list);
    ui->tableWidget->verticalHeader()->setVisible(false);   //隐藏列表头
    ui->tableWidget->horizontalHeader()->setVisible(false); //隐藏行表头
    //列宽都一样并拉伸铺满整个表格
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    //行宽都一样并拉伸铺满整个表格
    //ui->tableWidget->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    //拉伸最后一行铺满整个表格
    ui->tableWidget->verticalHeader()->setStretchLastSection(true);
    //总行数增加1,这句每加一条数据写一次
    ui->tableWidget->insertRow(ui->tableWidget->rowCount());
    ui->tableWidget->insertRow(ui->tableWidget->rowCount());
    //给第0行的0,1列插入数据
    ui->tableWidget->setItem(0, 0, new QTableWidgetItem("名称："));
    ui->tableWidget->setItem(0, 1, new QTableWidgetItem(""));
    //给第1行的0,1列插入数据
    ui->tableWidget->setItem(1, 0, new QTableWidgetItem("百度百科："));
    ui->tableWidget->setItem(1, 1, new QTableWidgetItem(""));

    // 第0行第0,1列设置水平,垂直都居中对齐
    ui->tableWidget->item(0, 0)->setTextAlignment(Qt::AlignCenter);
    ui->tableWidget->item(0, 1)->setTextAlignment(Qt::AlignCenter);
    // 第1行第0,1列设置水平,垂直都居中对齐
    ui->tableWidget->item(1, 0)->setTextAlignment(Qt::AlignCenter);
    ui->tableWidget->item(1, 1)->setTextAlignment(Qt::AlignCenter);

    /*******************功能配置*************************/

    manager = new QNetworkAccessManager(this);
    manager_download = new QNetworkAccessManager(this);
    manager_photo = new QNetworkAccessManager(this);

    //连接请求完成和读取数据的槽
    connect(manager,&QNetworkAccessManager::finished,this,&MainWindow::readdata_slot);
    connect(manager_download,&QNetworkAccessManager::finished,this,&MainWindow::download_slot);
    connect(manager_photo,&QNetworkAccessManager::finished,this,&MainWindow::photo_slot);

    //绑定快捷键
    ui->pushButton->setShortcut(Qt::Key_Return);
}

/* 查询按钮 */
void MainWindow::on_pushButton_clicked()
{
    if(ui->lineEdit->text().isEmpty() || ui->lineEdit->text().isNull())
    {
        return;
    }
    QMovie *movie = new QMovie(":/img/running.gif");
    QSize size(ui->lbl_photo->width(),ui->lbl_photo->height());
    movie->setScaledSize(size);
    movie->start();
    ui->lbl_photo->setMovie(movie);

    QMovie *movie1 = new QMovie(":/img/running.gif");
    QSize size1(ui->lbl_qrcode->width(),ui->lbl_qrcode->height());
    movie1->setScaledSize(size1);
    movie1->start();
    ui->lbl_qrcode->setMovie(movie1);

    //动态显示文字信息
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(update()));
    timer->start(1000);

    //获取url
    QString url_str = "https://api.wer.plus/api/dub?t=" + ui->lineEdit->text();
    http_rst(url_str,manager);
}

/* 更新文字动画 */
void MainWindow::update()
{
    switch (dot%4) {
    case 1:
        ui->label->setText("扫描左侧二维码获取更多内容.");
        break;
    case 2:
        ui->label->setText("扫描左侧二维码获取更多内容..");
        break;
    case 3:
        ui->label->setText("扫描左侧二维码获取更多内容...");
        break;
    default:
        ui->label->setText("扫描左侧二维码获取更多内容");
        break;
    }
    dot++;
}

/* 发送http请求 */
void MainWindow::http_rst(QString url_str, QNetworkAccessManager *manager)
{
    //获取url
    QUrl url(url_str);
    //创建http请求
    QNetworkRequest request(url);
    //发送http请求 ----- GET
    request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
    manager->get(request);
}

/* http请求回调函数 */
void MainWindow::readdata_slot(QNetworkReply *reply)
{
    QByteArray arr = reply->readAll();
    //qDebug() << arr;

    //解析Json
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(arr,&error);

    //判断json数据是否完整
    if(error.error!=QJsonParseError::NoError)
    {
        //设置弹窗
        QMessageBox box;
        box.setText("Json数据错误!");
        box.setStandardButtons(QMessageBox::Ok);
        box.exec();
        return;
    }

    //将文本对象转换成json对象
    QJsonObject obj = doc.object();

    QJsonObject json_data = obj.value("data").toObject();
    QString json_text = json_data.value("text").toString();
    //qDebug () <<json_text;
    // 提取出学名
    // ****************************

    ui->tableWidget->setItem(0, 1, new QTableWidgetItem(ui->lineEdit->text()));
    ui->tableWidget->setItem(1, 1, new QTableWidgetItem(json_text));

    //生成植物百科二维码
    GenerateQRcode("https://www.iplant.cn/info/"+ui->lineEdit->text());

    QString img_path = json_data.value("img_url").toString();

    //显示
    //qDebug() << img_path;
    if(!img_path.isEmpty() && img_path != "None") {
        http_rst(img_path,manager_download);
    } else {
        QString url_str = "http://image.so.com/j?q=" + ui->lineEdit->text() + "&src=srp&correct=" + ui->lineEdit->text() + "&sn=1&pn=1";
        http_rst(url_str,manager_photo);
    }
}

/* 下载图片回调函数 */
void MainWindow::photo_slot(QNetworkReply *reply)
{
    QByteArray arr = reply->readAll();
    //qDebug() << arr;

    //解析Json
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(arr,&error);

    //判断json数据是否完整
    if(error.error!=QJsonParseError::NoError)
    {
        //设置弹窗
        QMessageBox box;
        box.setText("Json数据错误!");
        box.setStandardButtons(QMessageBox::Ok);
        box.exec();
        return;
    }

    //将文本对象转换成json对象
    QJsonObject obj = doc.object();
    //在json对象中根据key获取value
    //解析数组
    QJsonArray json_arr = obj.value("list").toArray();
    QJsonObject json_obj = json_arr.at(0).toObject();
    QString img_path = json_obj.value("img").toString();
    http_rst(img_path,manager_download);
}

/* 图片下载函数
 * 参数：http请求
 */
void MainWindow::download_slot(QNetworkReply *reply)
{
    if (reply->error() == QNetworkReply::NoError)
    {
        QByteArray bytes = reply->readAll();
        QPixmap pixmap;
        pixmap.loadFromData(bytes);

        int with = ui->lbl_photo->width();
        int height = ui->lbl_photo->height();
        QPixmap fitpixmap = pixmap.scaled(with, height, Qt::IgnoreAspectRatio, Qt::SmoothTransformation); // 饱满填充
        //QPixmap fitpixmap = pixmap.scaled(with, height, Qt::KeepAspectRatio, Qt::SmoothTransformation); // 按比例缩放

        // 显示信息
        ui->lbl_photo->setPixmap(fitpixmap);

        // 缓存到本地
//        QFile file("C:/head.jpg");
//        if (file.open(QIODevice::Append))
//            file.write(bytes);
//        file.close();
    } else {
        //显示错误图片
    }
}

/* 二维码生成函数
 * 参数：需要生成的字符串
 */
void MainWindow::GenerateQRcode(QString tempstr)
{
    QRcode *qrcode;
    qrcode=QRcode_encodeString(tempstr.toStdString().c_str(),2,QR_ECLEVEL_Q,QR_MODE_8,1);
    qint32 temp_width=ui->lbl_qrcode->height()/2;
    qint32 temp_height=ui->lbl_qrcode->height()/2;

    qint32 qrcode_width=qrcode->width>0?qrcode->width:1;
    double scale_x=(double)temp_width/(double)qrcode_width;
    double scale_y=(double)temp_height/(double)qrcode_width;
    QImage mainimg=QImage(temp_width,temp_height,QImage::Format_ARGB32);
    QPainter painter(&mainimg);
    QColor background(Qt::white);
    painter.setBrush(background);
    painter.setPen(Qt::NoPen);
    painter.drawRect(0,0,temp_width,temp_height);
    QColor foreground(Qt::black);
    painter.setBrush(foreground);
    for(qint32 y=0;y<qrcode_width;y++)
    {
        for(qint32 x=0;x<qrcode_width;x++)
        {
            unsigned char b=qrcode->data[y*qrcode_width+x];
            if(b &0x01)
            {
                QRectF r(x*scale_x,y*scale_y,scale_x,scale_y);
                painter.drawRects(&r,1);
            }
        }
    }

    QPixmap mainmap=QPixmap::fromImage(mainimg);
    ui->lbl_qrcode->setPixmap(mainmap);
    ui->lbl_qrcode->setVisible(true);
}

MainWindow::~MainWindow()
{
    delete ui;
}

//点击添加按钮
void MainWindow::on_btn_add_clicked()
{
    if(!this->ui->lineEdit_date->text().isEmpty() && !this->ui->lineEdit_date->text().isNull())
    {

        QString table_name = ui->tableWidget->item(0,1)->text();//取出字符串
        qDebug() << table_name;
        QString table_info = ui->tableWidget->item(1,1)->text();//取出字符串
        qDebug() << table_info;
        QString table_date = ui->lineEdit_date->text();
        qDebug() << table_date;
        if(table_date.contains("上午"))
        {
            table_date = table_date.left(table_date.size() - 2);
            table_date += date_am;
            qDebug() << table_date;
        }
        else
        {
            table_date = table_date.left(table_date.size() - 2);
            table_date += date_pm;
            qDebug() << table_date;
        }

        //写入数据库
        QString str = QString("insert into plant_info(pName, pTime, pInfo, pStatus) values('%1', '%2', '%3', '%4')").arg(table_name).arg(table_date).arg(table_info).arg(-1);
        QSqlQuery query;
        query.exec(str);
        emit reflash();
    }
    else
    {
        QMessageBox::critical(this,"error","请选择种植日期！");
    }
}

void MainWindow::on_lineEdit_editingFinished()
{
    this->ui->btn_add->setDisabled(false);
    FlatUI::setPushButtonQss(ui->btn_add);
}
