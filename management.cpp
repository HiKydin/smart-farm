#include "management.h"
#include "ui_management.h"
#include "flatui.h"
#include "maskwidget.h"
/* JSON信息体定义
 * 控制类:
 * {
 *      "control": "xxx"
 * }
 * xxx为要控制的元器件:风机/水泵/太阳能面板/人工光源/摄像头/蜂鸣器
 * 信息类:
 * {
 *      "msg": "xxx",
 *      "value": "xxx"
 * }
 * msg为状态类别:温度/空气湿度/土壤湿度/二氧化碳浓度/光照强度
 * value为具体采集到的数值
 *
 *
 *
 * 上报报文
 * Hello包
 * {
 *  "msg": "hello",
 *  "deviceID": "ESP8266Client-xxx",
 *  "IP": "192.168.1.1"
 * }
 * 采集数据包
 * {
 *  "msg": "getvalue",
 *  "deviceID": "ESP8266Client-xxx",
 *  "IP": "192.168.1.1",
 *  "device": "s",
 *  "value": "1000"
 * }
 */

/* 采集设备定义
 * SOIL 0   // 土壤湿度
 * GAS 1    // 可燃性气体浓度
 * LIGHT 2  // 光照强度
 * ALL 3    // 采集所有设备
 * 控制设备定义
 * FAN 0     // 排风扇
 * LED 1     // LED灯
 * PUMP 2    // 水泵
 * BUZZER 3  // 蜂鸣器
 */

struct device_s {
    bool fanIsOn;
    bool ledIsOn;
    bool pumpIsOn;
    bool buzzerIsOn;
};

Management::Management(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Management)
{
    /*******************初始化数据结构*************************/
    // 使用键值对管理植株状态
    map.insert(-1,"初始化");
    map.insert(0,"健康");
    map.insert(1,"缺水");
    map.insert(2,"可燃气体浓度过高");
    map.insert(3,"缺光照");

    map2.insert("初始化",-1);
    map2.insert("健康",0);
    map2.insert("缺水",1);
    map2.insert("可燃气体浓度过高",2);
    map2.insert("缺光照",3);

    // 创建下位机设备的结构体数组
    //    device_s device_status[3] = {
    //        {"Tom", 18, "Computer Science"},
    //        {"Jack", 20, "Mathematics"},
    //        {"Lucy", 19, "Physics"}
    //    };
    /*******************界面配置*************************/
    ui->setupUi(this);
    ui->tabWidget->setTabShape(QTabWidget::Triangular);
    ui->tabWidget->setCurrentIndex(0); //设置当前活动页
    initTray();
    this->setWindowTitle("智慧农场 - 管理界面 v0.2");
    this->setStyleSheet("*{outline:0px;}QWidget#frmFlatUI{background:#FFFFFF;}");
    this->setFixedSize(1335, 900);
    ui->lineEdit_devIP->setDisabled(true);//禁止设备IP栏被编辑，只能通过MQTT hello包修改
    ui->comboBox_devPlant->addItem("无",0);//下拉框默认值为无
    ui->spinBox_humidity->setRange(0,1024);//设置土壤湿度阈值取值范围
    //保存绑定按钮默认禁用
    FlatUI::setPushButtonQss(ui->btn_devSave, 5, 8, "#d5d8dc", "#E6F8F5", "#2EE1C1", "#FFFFFF", "#16A086", "#A7EEE6");
    ui->btn_devSave->setDisabled(true);
    //默认选中所有设备
    ui->radioButton_all_device->setChecked(true);
    ui->radioButton_d_mode->setChecked(true);
    //背景图
    QPixmap bgImg(":/img/background.jpg"); //加载图片
    // 获取当前窗口大小，用于调整背景图片大小
    QSize size = this->size();
    QPixmap scaled_bgImg = bgImg.scaled(size, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    QPalette bgPalette;
    //创建半透明的颜色
    QColor translucentColor = QColor(0, 0, 0, 204);
    QBrush translucentBrush(translucentColor, scaled_bgImg);
    bgPalette.setBrush(QPalette::Background, translucentBrush); //设置背景画笔为背景图片和半透明颜色的组合
    ui->tab_5->setAutoFillBackground(true); //允许自动填充背景
    ui->tab_5->setPalette(bgPalette); //设置背景画笔

    //获取当前时间
    QDateTime time = QDateTime::currentDateTime();
    logread("当前时间" + time.toString("yyyy/MM/dd hh:mm:ss.zzz"));

    //显示条形统计图
    bar=new barchar();
    ui->gridLayout_bar->addWidget(bar,0,0,-1,-1,0);
    //显示饼状图
    pie=new piechart();
    ui->gridLayout_pie->addWidget(pie,0,0,-1,-1,0);

    //创建表格
    QStringList list;
    list << "作物名称" << "种植时间"<< "详情" << "作物状态" ;//设置列数为2列,这句一定要写,否则表头不显示
    ui->tableWidget->setColumnCount(4);//设置表头
    ui->tableWidget->setHorizontalHeaderLabels(list);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);//列宽相同并拉伸铺满整个表格
    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);//
    //    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);//禁止编辑
    //    ui->tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);//单选模式
    ui->tableWidget->setAlternatingRowColors(true);//颜色

    /*******************资源加载*************************/
    //音频加载
    alertSound = new QSound(":/alert.wav");
    clickSound = new QSound(":/click.wav");

    //初始化监控视频
    QMovie *movie = new QMovie(":/img/noise.gif");
    movie->start();
    ui->monitor_1->setMovie(movie);
    ui->monitor_2->setMovie(movie);
    ui->monitor_3->setMovie(movie);
    ui->monitor_4->setMovie(movie);

    //使用QSS样式库美化界面
    //按钮区:
    FlatUI::setPushButtonQss(ui->btn_reflash);
    FlatUI::setPushButtonQss(ui->btn_import, 5, 8, "#1ABC9C", "#E6F8F5", "#2EE1C1", "#FFFFFF", "#16A086", "#A7EEE6");
    FlatUI::setPushButtonQss(ui->btn_outdata, 5, 8, "#3498DB", "#FFFFFF", "#5DACE4", "#E5FEFF", "#2483C7", "#A0DAFB");
    FlatUI::setPushButtonQss(ui->btn_add, 5, 8, "#E74C3C", "#FFFFFF", "#EC7064", "#FFF5E7", "#DC2D1A", "#F5A996");
    FlatUI::setPushButtonQss(ui->btn_seach);
    FlatUI::setPushButtonQss(ui->btn_del);

    FlatUI::setPushButtonQss(ui->btn_sql_connect);
    FlatUI::setPushButtonQss(ui->btn_sql_disconnect);
    FlatUI::setPushButtonQss(ui->btn_mqtt_connect);
    FlatUI::setPushButtonQss(ui->btn_mqtt_disconnect);
    FlatUI::setPushButtonQss(ui->btn_mqtt_subscribe);
    FlatUI::setPushButtonQss(ui->btn_mqtt_publish);
    FlatUI::setPushButtonQss(ui->btn_alertOn);
    FlatUI::setPushButtonQss(ui->btn_alertOff);
    FlatUI::setPushButtonQss(ui->pushButton_3);
    FlatUI::setPushButtonQss(ui->btn_mqtt_clear);

    FlatUI::setPushButtonQss(ui->btn_fanOn);
    FlatUI::setPushButtonQss(ui->btn_fanOff);
    FlatUI::setPushButtonQss(ui->btn_ledOn);
    FlatUI::setPushButtonQss(ui->btn_ledOff);
    FlatUI::setPushButtonQss(ui->btn_pumpOn);
    FlatUI::setPushButtonQss(ui->btn_pumpOff);
    FlatUI::setPushButtonQss(ui->btn_buzzerOn);
    FlatUI::setPushButtonQss(ui->btn_buzzerOff);
    FlatUI::setPushButtonQss(ui->btn_getStatus);
    FlatUI::setPushButtonQss(ui->btn_saveData);
    FlatUI::setPushButtonQss(ui->btn_11);
    FlatUI::setPushButtonQss(ui->btn_12);

    //单选框:
    FlatUI::setRadioButtonQss(ui->radioButton_qos_0, 8, "#D7DBDE", "#1ABC9C");
    FlatUI::setRadioButtonQss(ui->radioButton_qos_1, 8, "#D7DBDE", "#3498DB");
    FlatUI::setRadioButtonQss(ui->radioButton_qos_2, 8, "#D7DBDE", "#E74C3C");
    FlatUI::setRadioButtonQss(ui->radioButton_all_device, 8, "#D7DBDE", "#1ABC9C");
    FlatUI::setRadioButtonQss(ui->radioButton_choose_device, 8, "#D7DBDE", "#1ABC9C");
    FlatUI::setRadioButtonQss(ui->radioButton_a_mode, 8, "#D7DBDE", "#3498DB");
    FlatUI::setRadioButtonQss(ui->radioButton_d_mode, 8, "#D7DBDE", "#3498DB");
    //输入框:
    FlatUI::setLineEditQss(ui->lineEdit_sql_ip);
    FlatUI::setLineEditQss(ui->lineEdit_sql_name);
    FlatUI::setLineEditQss(ui->lineEdit_sql_ac);
    FlatUI::setLineEditQss(ui->lineEdit_sql_pw);

    FlatUI::setLineEditQss(ui->lineEdit_mqtt_ip);
    FlatUI::setLineEditQss(ui->lineEdit_mqtt_clientID);
    FlatUI::setLineEditQss(ui->lineEdit_mqtt_subscribe);
    FlatUI::setLineEditQss(ui->lineEdit_mqtt_publish);
    FlatUI::setLineEditQss(ui->lineEdit_mqtt_publish_value);

    FlatUI::setLineEditQss(ui->lineEdit_devIP);
    //鼠标穿透事件
    ui->label_connect_status->setAttribute(Qt::WA_TransparentForMouseEvents);
    ui->label_status_photo->setAttribute(Qt::WA_TransparentForMouseEvents);
    ui->label_status->setAttribute(Qt::WA_TransparentForMouseEvents);

    ui->lbl_sql_connect_status->setAttribute(Qt::WA_TransparentForMouseEvents);
    ui->lbl_sql_status_photo->setAttribute(Qt::WA_TransparentForMouseEvents);
    ui->lbl_sql_status->setAttribute(Qt::WA_TransparentForMouseEvents);

    /*******************功能配置*************************/

    /*************连接数据库*****************/
    db = QSqlDatabase::addDatabase("QMYSQL");
    if(!ui->lineEdit_sql_ip->text().isEmpty() && !ui->lineEdit_sql_ip->text().isNull() \
            && !ui->lineEdit_sql_name->text().isEmpty() && !ui->lineEdit_sql_name->text().isNull()\
            && !ui->lineEdit_sql_ac->text().isEmpty() && !ui->lineEdit_sql_ac->text().isNull()\
            && !ui->lineEdit_sql_pw->text().isEmpty() && !ui->lineEdit_sql_pw->text().isNull()\
            && !ui->spinBox_sql_port->text().isEmpty() && !ui->spinBox_sql_port->text().isNull())
    {
        on_btn_sql_connect_clicked();
    }

    /***********连接MQTT服务器***************/
    connectMQTT();

    /*************连接信号槽*****************/
    connect(ui->btn_reflash,&QPushButton::clicked,this,&Management::reflashData);
    connect(ui->btn_import,&QPushButton::clicked,this,&Management::importData);
    connect(ui->btn_outdata,&QPushButton::clicked,this,&Management::outData);
    connect(ui->btn_add,&QPushButton::clicked,this,&Management::addItem);
    connect(ui->btn_seach,&QPushButton::clicked,this,&Management::seach);
    connect(ui->btn_del,&QPushButton::clicked,this,&Management::deletePage);
    connect(ui->btn_mqtt_clear,&QPushButton::clicked,[=](){
        clickSound->play();
        ui->textBrowser_mqtt_log->clear();
    });
    //双击选择设备时，显示其IP、绑定作物
    connect(ui->onlineDevice,&QListWidget::itemDoubleClicked,[=](){
        QString dev = ui->onlineDevice->currentItem()->text();
        QString deviceIP = device_ip[dev];
        ui->lineEdit_devIP->setText(deviceIP);

        if (device_bind.contains(dev)) {
            //没有变化的时候禁用
            FlatUI::setPushButtonQss(ui->btn_devSave, 5, 8, "#d5d8dc", "#E6F8F5", "#2EE1C1", "#FFFFFF", "#16A086", "#A7EEE6");
            ui->btn_devSave->setDisabled(true);
            ui->comboBox_devPlant->setCurrentIndex(device_bind[dev]);
        }
        else {
            ui->comboBox_devPlant->setCurrentIndex(0);
        }
    });
    //保存设备绑定作物
    connect(ui->btn_devSave,&QPushButton::clicked,[=](){
        clickSound->play();
        QString devText = ui->comboBox_devPlant->currentText();
        int devBind = ui->comboBox_devPlant->currentData().toInt();
        QString dev = ui->onlineDevice->currentItem()->text();
        //当设备已经绑定了作物时，就要修改，反之则需插入
        if (device_bind.contains(dev)) {
            device_bind[dev] = devBind;// Modify the value to new
        }
        else {
            device_bind.insert(dev, devBind); // Insert a new key-value pair
        }
        if(devBind != 0){
            logread("设备：" + dev + "绑定为：" + devText + "园区的下位机");
        }
        else
        {
            logread("设备：" + dev + "已解绑");
        }
        //结束，禁用按钮
        FlatUI::setPushButtonQss(ui->btn_devSave, 5, 8, "#d5d8dc", "#E6F8F5", "#2EE1C1", "#FFFFFF", "#16A086", "#A7EEE6");
        ui->btn_devSave->setDisabled(true);
    });

    //下拉框改变值
    connect(ui->comboBox_devPlant,&QComboBox::currentTextChanged,[=](){
        int currentBind = ui->comboBox_devPlant->currentData().toInt();
        QString dev = ui->onlineDevice->currentItem()->text();
        if (device_bind.contains(dev)) {
            int devBind =device_bind[dev];
            if (currentBind != devBind) {
                FlatUI::setPushButtonQss(ui->btn_devSave);
                ui->btn_devSave->setDisabled(false);
            } else {
                //没有变化的时候禁用
                FlatUI::setPushButtonQss(ui->btn_devSave, 5, 8, "#d5d8dc", "#E6F8F5", "#2EE1C1", "#FFFFFF", "#16A086", "#A7EEE6");
                ui->btn_devSave->setDisabled(true);
            }
        }
    });

    //开关排风扇按钮
    connect(ui->btn_fanOn,&QPushButton::clicked,[=](){
        clickSound->play();
        QString deviceID = getDeviceID();
        if(deviceID != NULL){
            this->setFan(deviceID, ON);
        }
    });
    connect(ui->btn_fanOff,&QPushButton::clicked,[=](){
        clickSound->play();
        QString deviceID = getDeviceID();
        if(deviceID != NULL){
            this->setFan(deviceID, OFF);
        }
    });

    //开关LED按钮
    connect(ui->btn_ledOn,&QPushButton::clicked,[=](){
        clickSound->play();
        QString deviceID = getDeviceID();
        if(deviceID != NULL){
            this->setLED(deviceID, ON);
        }
    });
    connect(ui->btn_ledOff,&QPushButton::clicked,[=](){
        clickSound->play();
        QString deviceID = getDeviceID();
        if(deviceID != NULL){
            this->setLED(deviceID, OFF);
        }
    });

    //开关水泵按钮
    connect(ui->btn_pumpOn,&QPushButton::clicked,[=](){
        clickSound->play();
        QString deviceID = getDeviceID();
        if(deviceID != NULL){
            this->setPump(deviceID, ON);
        }
    });
    connect(ui->btn_pumpOff,&QPushButton::clicked,[=](){
        clickSound->play();
        QString deviceID = getDeviceID();
        if(deviceID != NULL){
            this->setPump(deviceID, OFF);
        }
    });

    //开关蜂鸣器按钮
    connect(ui->btn_buzzerOn,&QPushButton::clicked,[=](){
        clickSound->play();
        QString deviceID = getDeviceID();
        if(deviceID != NULL){
            this->setBuzzer(deviceID, ON);
        }
    });
    connect(ui->btn_buzzerOff,&QPushButton::clicked,[=](){
        clickSound->play();
        QString deviceID = getDeviceID();
        if(deviceID != NULL){
            this->setBuzzer(deviceID, OFF);
        }
    });

    //保存数据按钮
    connect(ui->btn_saveData,&QPushButton::clicked,[=](){
        clickSound->play();
        saveBarData();
    });

    //开关自动告警按钮
    connect(ui->btn_alertOn,&QPushButton::clicked,[=](){
        if (canPlay > 0) {
            clickSound->play();
        } else {
            canPlay++;
        }
        doAlert = ON;
        ui->btn_alertOn->setDisabled(true);
        ui->btn_alertOff->setDisabled(false);\
        //颜色修改
        FlatUI::setPushButtonQss(ui->btn_alertOff);
        FlatUI::setPushButtonQss(ui->btn_alertOn, 5, 8, "#d5d8dc", "#E6F8F5", "#2EE1C1", "#FFFFFF", "#16A086", "#A7EEE6");
        logread("自动告警系统已开启");
    });
    connect(ui->btn_alertOff,&QPushButton::clicked,[=](){
        clickSound->play();
        doAlert = OFF;
        ui->btn_alertOn->setDisabled(false);
        ui->btn_alertOff->setDisabled(true);
        //颜色修改
        FlatUI::setPushButtonQss(ui->btn_alertOn);
        FlatUI::setPushButtonQss(ui->btn_alertOff, 5, 8, "#d5d8dc", "#E6F8F5", "#2EE1C1", "#FFFFFF", "#16A086", "#A7EEE6");
        logread("自动告警系统已关闭");
    });

    //开启自动告警
    ui->btn_alertOn->clicked();

    /*******************读取数据*************************/
    //读取统计图表
    QFile file("C:/Users/Administrator/Desktop/deviceUse.txt");
    if (file.exists())
    {
        if (file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            while (!file.atEnd())
            {
                QByteArray line = file.readLine();
                QString str(line);
                if(str.contains("fan")){
                    fanUseList = str.split(",");
                } else if(str.contains("LED")){
                    ledUseList = str.split(",");
                } else if(str.contains("pump")){
                    pumpUseList = str.split(",");
                } else if(str.contains("buzzer")){
                    buzzerUseList = str.split(",");
                }
            }
            file.close();
            reflashBar();
        } else {
            logread("打开统计文件失败");
        }
    } else {
        //创建
        file.open( QIODevice::WriteOnly );
        file.close();

        //写入初始值
        if (file.open(QIODevice::ReadWrite | QIODevice::Text))
        {
            QTextStream stream(&file);
            stream.seek(file.size());
            stream << "fan,0,0,0,0,0,0,0,0,0,0,0,0\nLED,0,0,0,0,0,0,0,0,0,0,0,0\npump,0,0,0,0,0,0,0,0,0,0,0,0\nbuzzer,0,0,0,0,0,0,0,0,0,0,0,0\n";
            file.close();
        } else {
            logread("打开统计文件失败");
        }
    }

    //读取设备数据
    QFile devFile("C:/Users/Administrator/Desktop/deviceInfo.txt");
    if (devFile.exists())
    {
        if (devFile.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            while (!devFile.atEnd())
            {
                QByteArray line = devFile.readLine();
                QString str(line);
                QStringList devInfo = str.split(",");
                ui->onlineDevice->addItem(devInfo.at(0));
                device_ip.insert(devInfo.at(0), devInfo.at(1));
                device_bind.insert(devInfo.at(0), devInfo.at(2).toInt());
            }
            devFile.close();
        } else {
            logread("打开设备数据失败");
        }
    } else {
        //创建
        devFile.open( QIODevice::WriteOnly );
        devFile.close();
    }

    /*******************定时器设置*************************/
    // 创建定时器对象
    QTimer *timer = new QTimer(this);//自动保存的定时器
    QTimer *getValue_timer = new QTimer(this);//自动下发采集指令的定时器

    // 连接定时器信号和槽函数
    QObject::connect(timer, &QTimer::timeout,[=](){
        //定时保存统计值至文件，防止丢失
        saveBarData();
    });

    // 连接定时器信号和槽函数
    QObject::connect(timer, &QTimer::timeout,[=](){
        //定时下发采集指令
        on_btn_getStatus_clicked();
    });

    // 启动定时器，设置定时间隔为30分钟（1800000毫秒）
    timer->start(1800000);
    getValue_timer->start(1800000);
}

void Management::initTray()
{
    //创建托盘
    QIcon icon = QIcon(":/img/logo.png");
    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setIcon(icon);
    trayIcon->setToolTip("智慧农场管理系统");
    trayIcon->show();

    //托盘事件
    minimizeAction = new QAction("最小化",this);
    connect(minimizeAction, SIGNAL(triggered()), this, SLOT(hide()));

    maximizeAction = new QAction("最大化",this);
    connect(maximizeAction, SIGNAL(triggered()), this, SLOT(showNormal()));

    quitAction = new QAction("退出", this);
    connect(quitAction, &QAction::triggered, [=](){
        //TODO关闭应用，qApp对应的是程序全局唯一指针
        qDebug()<<"退出";
    });

    //创建托盘菜单（必须先创建动作，后添加菜单项，还可以加入菜单项图标美化）
    trayIconMenu = new QMenu(this);
    trayIconMenu->addAction(minimizeAction);
    trayIconMenu->addAction(maximizeAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(quitAction);
    trayIcon->setContextMenu(trayIconMenu);

    connect(trayIcon,SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            this,SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));
}

//保存条形统计图数据、设备信息数据至文件
void Management::saveBarData()
{
    logread("正在保存数据,请勿关闭软件...");
    /*************条形统计图数据*****************/
    {
        //先清空文件内容
        QFile file("C:/Users/Administrator/Desktop/deviceUse.txt");
        if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
            logread("打开统计文件失败");
            return;
        }

        QTextStream out(&file);
        out << "";

        // 关闭文件
        file.close();

        //写入文件
        if (file.open(QIODevice::ReadWrite | QIODevice::Text))
        {
            QTextStream stream(&file);
            stream.seek(file.size());
            QString strFan = fanUseList.join(",");
            QString strLED = ledUseList.join(",");
            QString strPump = pumpUseList.join(",");
            QString strBuzzer = buzzerUseList.join(",");
            stream << strFan+strLED+strPump+strBuzzer;
            file.close();
        }
    }

    /*************设备信息数据*****************/
    {
        //先清空文件内容
        QFile file("C:/Users/Administrator/Desktop/deviceInfo.txt");
        if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
            logread("打开设备信息文件失败");
            return;
        }

        QTextStream out(&file);
        out << "";

        // 关闭文件
        file.close();

        //写入文件
        if (file.open(QIODevice::ReadWrite | QIODevice::Text))
        {
            QTextStream stream(&file);
            stream.seek(file.size());

            for(int i = 0; i < ui->onlineDevice->count(); i++) {
                QListWidgetItem* item = ui->onlineDevice->item(i);  // 获取该条目
                QString deviceID = item->text();  // 获取该条目的文本内容
                QString deviceIP = device_ip[deviceID];
                int deviceBind = device_bind[deviceID];
                stream << deviceID + "," + deviceIP + "," + QString::number(deviceBind) + "\n";
            }
            file.close();
        }
    }

    logread("自动保存数据成功！");
}

//刷新条形统计图
void Management::reflashBar()
{
    logread("刷新条形统计图");
    bar->add_BarSet(fanUseList);
    bar->add_BarSet(ledUseList);
    bar->add_BarSet(pumpUseList);
    bar->add_BarSet(buzzerUseList);
}

//日志输出接口
void Management::logread(QString msg)
{
    QDateTime time = QDateTime::currentDateTime();   //获取当前时间
    ui->textBrowser_log->append(time.toString("yyyy-MM-dd hh:mm:ss ")+msg);
}

//mqtt连接函数
void Management::connectMQTT()
{
    QHostInfo info = QHostInfo::fromName(ui->lineEdit_mqtt_ip->text());
    QString mqtt_ip = info.addresses().first().toString();
    mqtt = new QMQTT::Client(QHostAddress(mqtt_ip),ui->spinBox_mqtt_port->text().toInt());
    mqtt->setCleanSession(true);
    mqtt->setClientId(ui->lineEdit_mqtt_clientID->text());
    mqtt->connectToHost(); //连接mqtt
    logread("连接mqtt服务器中...");
    logread("IP地址:" + mqtt_ip + ",端口:" + ui->spinBox_mqtt_port->text() + ",名称:" + ui->lineEdit_mqtt_clientID->text());

    //连接信号槽
    connect(mqtt, SIGNAL(connected()), this, SLOT(mqtt_connect_info()));
    connect(mqtt, SIGNAL(disconnected()), this, SLOT(mqtt_disconnect_info()));
    connect(mqtt, SIGNAL(received(QMQTT::Message)), this, SLOT(mqtt_recv_msg(QMQTT::Message)));
    connect(mqtt, SIGNAL(subscribed(QString,quint8)), this, SLOT(mqtt_sub_success(QString,quint8)));
}

void Management::mqtt_sub_success(QString topic,quint8 qos) //订阅成功
{
    QString msg = "订阅主题:";
    msg += topic + " QoS=" + QString::number(qos) + " 成功";
    logread(msg);
    ui->textBrowser_mqtt_log->append(msg);
}

void Management::mqtt_recv_msg(QMQTT::Message msg) //接收消息处理
{
    QString recv_msg = "收到主题:";
    recv_msg += msg.topic() + " 内容:" + msg.payload();
    ui->textBrowser_mqtt_log->append(recv_msg);
    Management::Parse_Json(msg.payload());
}

int Management::connectSQL()
{
    //127.0.0.1 3306 farm_info root 123456
    db.setHostName(ui->lineEdit_sql_ip->text());
    db.setPort(ui->spinBox_sql_port->text().toInt());
    db.setDatabaseName(ui->lineEdit_sql_name->text());
    db.setUserName(ui->lineEdit_sql_ac->text());
    db.setPassword(ui->lineEdit_sql_pw->text());

    if (db.open()){
        logread("成功连接MySQL数据库");
        return true;
    }
    else {
        logread("MySQL数据库连接失败！");
        return false;
    }
}

//刷新数据库
void Management::reflashData()
{
    if (canPlay > 0) {
        clickSound->play();
    } else {
        canPlay++;
    }
    logread("刷新数据库列表");
    delete pie;
    pie=new piechart();
    ui->gridLayout_pie->addWidget(pie,0,0,-1,-1,0);

    //清除表格再添加数据
    ui->tableWidget->clearContents();
    ui->tableWidget->setRowCount(0);
    //ui->comboBox_devPlant->clear();
    //ui->comboBox_devPlant->addItem("无",0);//下拉框默认值为无
    pie_map.clear();
    if (db.open()){
        QSqlQuery query("select * from plant_info");//查询表的内容
        while (query.next()) {
            //添加新行
            ui->tableWidget->insertRow(ui->tableWidget->rowCount());
            //给第0行的0,1列插入数据
            ui->tableWidget->setItem(ui->tableWidget->rowCount()-1, 0, new QTableWidgetItem(query.value(0).toString()));
            ui->tableWidget->setItem(ui->tableWidget->rowCount()-1, 1, new QTableWidgetItem(query.value(1).toString()));
            ui->tableWidget->setItem(ui->tableWidget->rowCount()-1, 2, new QTableWidgetItem(query.value(2).toString()));
            ui->tableWidget->setItem(ui->tableWidget->rowCount()-1, 3, new QTableWidgetItem(map[query.value(3).toInt()]));
            //使用键值对统计数量
            pie_map[query.value(0).toString()] = pie_map[query.value(0).toString()] + 1;
            //添加至设备绑定作物的下拉框中
            int index = ui->comboBox_devPlant->findText((QString::number(ui->tableWidget->rowCount()) + "号" + query.value(0).toString()));
            if (index == -1) {
                ui->comboBox_devPlant->addItem((QString::number(ui->tableWidget->rowCount()) + "号" + query.value(0).toString()),ui->tableWidget->rowCount());
            }
        }
        //传递键值对更新饼状图
        QMap<QString, int>::iterator iter = pie_map.begin();
        while (iter != pie_map.end())
        {
            pie->add_PieSeries(iter.key(),iter.value());
            iter++;
        }
        logread("刷新饼状图");
        pie->updateData();
    }
    else
    {
        logread("数据库未连接");
    }
}

//导入数据库
void Management::importData()
{
    clickSound->play();
    qDebug() << "import";
    logread("导入数据库");

    QString fileName = QFileDialog::getOpenFileName(0, "选择文件");
    if (fileName.isEmpty()) {
        return;
    }

    QFile file(fileName);
    bool ok = file.open(QIODevice::ReadOnly | QFile::Text);
    if (!ok) {
        logread("打开导入文件失败！");
        return;
    }

    //先读取第一行判断列数是否和目标列数一致,不一致则返回
    QString line = QString::fromUtf8(file.readLine());
    QStringList list = line.split(",");
    if (list.count() != 5) {
        logread("文件格式错误！");
        return;
    }

    //先删除原来的数据
    QString sql = QString("delete from plant_info");
    logread("源数据库已删除！");
    QSqlQuery query;
    query.exec(sql);

    //cvs格式需要gbk编码才能正常
    QTextStream in(&file);
    in.seek(0);
    if (fileName.endsWith(".csv")) {
#if (QT_VERSION < QT_VERSION_CHECK(6,0,0))
        in.setCodec("gbk");
#endif
    }

    //开启数据库事务加速处理
    QSqlDatabase::database().transaction();

    bool isremoveTitle = false;
    while (!in.atEnd()) {
        QString line = in.readLine();
        QStringList list = line.split(',');

        //如果存在标题则不需要处理第一行标题
        if (!isremoveTitle) {
            isremoveTitle = true;
            continue;
        }
        qint8 columnCount = 5;
        //列数必须完全一致才行
        if (list.count() == columnCount) {
            sql = "INSERT INTO plant_info (pName,pTime,pInfo,pStatus) VALUES ('";

            for (int i = 0; i < columnCount - 1; i++) {
                if(i == columnCount - 2)
                {
                    sql = sql + QString("%1)").arg(map2[list.at(i)]);
                }
                else if(i == columnCount - 3)
                {
                    sql = sql + list.at(i).trimmed() + "',";
                }
                else
                {
                    sql = sql + list.at(i).trimmed() + "','";
                }
            }
            query.clear();
            query.exec(sql);
        }
    }

    file.close();
    logread("导入数据库成功！");
    reflashData();
}

//导出数据库
void Management::outData()
{
    clickSound->play();
    //1.选择导出的csv文件保存路径
    QString csvFile = QFileDialog::getExistingDirectory(this);
    if(csvFile.isEmpty())
        return;

    //2.文件名采用系统时间戳生成唯一的文件
    QDateTime current_date_time =QDateTime::currentDateTime();
    QString current_date =current_date_time.toString("yyyy_MM_dd_hh_mm_ss");
    csvFile += tr("/%1_export_%2.csv").arg("plant_info").arg(current_date);

    //3.用QFile打开.csv文件 如果不存在则会自动新建一个新的文件
    QFile file(csvFile);
    if (file.exists())
    {
        //如果文件存在执行的操作，此处为空，因为文件不可能存在
    }
    file.open( QIODevice::ReadWrite | QIODevice::Text );
    QTextStream out(&file);
    //    out.setCodec("utf-8");
    //导出编码格式为gbk，这样导入的时候才不会失败
    out.setCodec("gbk");

    //4.获取数据 创建第一行
    out<<tr("pName,")<<tr("pTime,")<<tr("pInfo,")<<tr("pStatus,\n");//表头
    //其他数据可按照这种方式进行添加即可
    for(int i = 0; i < ui->tableWidget->rowCount(); i++)
    {
        out<<ui->tableWidget->item(i,0)->text()+","<<ui->tableWidget->item(i,1)->text()+","<<ui->tableWidget->item(i,2)->text()+","<<ui->tableWidget->item(i,3)->text()+",\n";//表头
    }
    //5.写完数据需要关闭文件
    file.close();

    QMessageBox::information(this,"success","导出成功！");
    logread("导出数据库，文件路径:"+csvFile);
}

//添加数据库页面
void Management::addItem()
{
    clickSound->play();
    addPage = new MainWindow();
    //应用程序级模态对话框，阻塞整个应用程序的所有窗口。
    //addPage->setWindowModality(Qt::ApplicationModal);
    addPage->show();
    connect(addPage,&MainWindow::reflash,this,&Management::reflashData);
}

//根据名称查询数据库
void Management::seachData(QString name)
{
    clickSound->play();
    logread("搜素数据库：" + name);

    //清除表格再添加数据
    ui->tableWidget->clearContents();
    ui->tableWidget->setRowCount(0);
    if (db.open()){
        //查询指定的内容
        QString strSql = "select * from plant_info where pName=\"" + name + "\"";
        QSqlQuery query(strSql);
        while (query.next()) {
            //添加新行
            ui->tableWidget->insertRow(ui->tableWidget->rowCount());
            //给第0行的0,1列插入数据
            ui->tableWidget->setItem(ui->tableWidget->rowCount()-1, 0, new QTableWidgetItem(query.value(0).toString()));
            ui->tableWidget->setItem(ui->tableWidget->rowCount()-1, 1, new QTableWidgetItem(query.value(1).toString()));
            ui->tableWidget->setItem(ui->tableWidget->rowCount()-1, 2, new QTableWidgetItem(query.value(2).toString()));
            ui->tableWidget->setItem(ui->tableWidget->rowCount()-1, 3, new QTableWidgetItem(map[query.value(3).toInt()]));
        }
    }
    else
    {
        qDebug()<<"db is no open";
    }
}

//搜索页面
void Management::seach()
{
    clickSound->play();
    seachPage = new Dialog_seach();
    connect(seachPage,SIGNAL(sendSeach(QString)),this,SLOT(seachData(QString)));

    //应用程序级模态对话框，阻塞整个应用程序的所有窗口。
    seachPage->setWindowModality(Qt::ApplicationModal);
    //遮蔽层动画
    MaskWidget::Instance()->setMainWidget(this->topLevelWidget());
    MaskWidget::Instance()->setDialogNames(QStringList() << "seachPage");
    seachPage->setObjectName("seachPage");
    seachPage->setWindowTitle("搜索植株");
    seachPage->exec();
}

//删除数据库字段
void Management::deleteData(QString name,QString date)
{
    qDebug()<<"delete"<<name<<date;
}

//删除页面
void Management::deletePage()
{
    clickSound->play();
    deleteDia = new Dialog_delete();
    connect(deleteDia,SIGNAL(sendDelete(QString,QString)),this,SLOT(deleteData(QString,QString)));

    deleteDia->setWindowTitle("删除植株");
    deleteDia->show();
}

Management::~Management()
{
    db.close();
    delete ui;
}

//sql 连接
void Management::on_btn_sql_connect_clicked()
{
    if (canPlay > 0) {
        clickSound->play();
    } else {
        canPlay++;
    }
    if(connectSQL())
    {
        reflashData();

        //状态图片切换
        pix.load(":/img/running.png");
        pix = pix.scaled(pix.width()*0.2,pix.height()*0.2);
        ui->lbl_sql_status_photo->setScaledContents(false);
        ui->lbl_sql_status_photo->setPixmap(pix);

        //启用配置栏
        ui->lbl_sql_connect_status->setText("已连接");
        ui->lineEdit_sql_ip->setDisabled(true);
        ui->lineEdit_sql_name->setDisabled(true);
        ui->lineEdit_sql_ac->setDisabled(true);
        ui->lineEdit_sql_pw->setDisabled(true);
        ui->spinBox_sql_port->setDisabled(true);

        ui->btn_sql_connect->setDisabled(true);
        ui->btn_sql_disconnect->setDisabled(false);
        ui->btn_sql_disconnect->setFocus();

        //颜色修改
        FlatUI::setPushButtonQss(ui->btn_sql_disconnect);
        FlatUI::setPushButtonQss(ui->btn_sql_connect, 5, 8, "#d5d8dc", "#E6F8F5", "#2EE1C1", "#FFFFFF", "#16A086", "#A7EEE6");
    }
    else
    {
        QMessageBox::critical(this,"error","连接数据库失败！");
        ui->lineEdit_sql_ip->setFocus();
    }
}

//sql 断开连接
void Management::on_btn_sql_disconnect_clicked()
{
    clickSound->play();
    if(db.open())
    {
        db.close();
        logread("断开mysql数据库");
    }

    //状态图片切换
    pix.load(":/img/stop.png");
    pix = pix.scaled(pix.width()*0.2,pix.height()*0.2);
    ui->lbl_sql_status_photo->setScaledContents(false);
    ui->lbl_sql_status_photo->setPixmap(pix);
    ui->lbl_sql_connect_status->setText("未连接");

    //启用配置栏
    ui->lineEdit_sql_ip->setDisabled(false);
    ui->lineEdit_sql_name->setDisabled(false);
    ui->lineEdit_sql_ac->setDisabled(false);
    ui->lineEdit_sql_pw->setDisabled(false);
    ui->spinBox_sql_port->setDisabled(false);

    ui->btn_sql_connect->setDisabled(false);
    ui->btn_sql_connect->setFocus();
    ui->btn_sql_disconnect->setDisabled(true);

    //颜色修改
    FlatUI::setPushButtonQss(ui->btn_sql_connect);
    FlatUI::setPushButtonQss(ui->btn_sql_disconnect, 5, 8, "#d5d8dc", "#E6F8F5", "#2EE1C1", "#FFFFFF", "#16A086", "#A7EEE6");
}

//mqtt 连接按钮
void Management::on_btn_mqtt_connect_clicked()
{
    clickSound->play();
    connectMQTT();
}

//mqtt 断开连接按钮
void Management::on_btn_mqtt_disconnect_clicked()
{
    clickSound->play();
    mqtt->disconnectFromHost();
}

//mqtt 成功连接显示
void Management::mqtt_connect_info()
{
    //输出日志
    logread("成功连接MQTT服务器");
    ui->textBrowser_mqtt_log->append("成功连接MQTT服务器");
    //状态图片切换
    pix.load(":/img/running.png");
    pix = pix.scaled(pix.width()*0.2,pix.height()*0.2);
    ui->label_status_photo->setScaledContents(false);
    ui->label_status_photo->setPixmap(pix);
    ui->label_connect_status->setText("已连接");

    //禁用配置栏
    ui->lineEdit_mqtt_ip->setDisabled(true);
    ui->lineEdit_mqtt_clientID->setDisabled(true);
    ui->spinBox_mqtt_port->setDisabled(true);

    ui->btn_mqtt_connect->setDisabled(true);
    ui->btn_mqtt_disconnect->setDisabled(false);
    ui->btn_mqtt_disconnect->setFocus();
    ui->btn_mqtt_publish->setDisabled(false);
    ui->btn_mqtt_subscribe->setDisabled(false);

    //颜色修改
    FlatUI::setPushButtonQss(ui->btn_mqtt_disconnect);
    FlatUI::setPushButtonQss(ui->btn_mqtt_publish);
    FlatUI::setPushButtonQss(ui->btn_mqtt_subscribe);
    FlatUI::setPushButtonQss(ui->btn_mqtt_connect, 5, 8, "#d5d8dc", "#E6F8F5", "#2EE1C1", "#FFFFFF", "#16A086", "#A7EEE6");

    //自动订阅主题
    mqtt->subscribe(ui->lineEdit_mqtt_subscribe->text(),1);
}

//mqtt 断开连接显示
void Management::mqtt_disconnect_info()
{
    //输出日志
    logread("MQTT服务器断开连接");
    ui->textBrowser_mqtt_log->append("MQTT服务器断开连接");
    //状态图片切换
    pix.load(":/img/stop.png");
    pix = pix.scaled(pix.width()*0.2,pix.height()*0.2);
    ui->label_status_photo->setScaledContents(false);
    ui->label_status_photo->setPixmap(pix);
    ui->label_connect_status->setText("未连接");

    //启用配置栏
    ui->lineEdit_mqtt_ip->setDisabled(false);
    ui->lineEdit_mqtt_clientID->setDisabled(false);
    ui->spinBox_mqtt_port->setDisabled(false);

    ui->btn_mqtt_connect->setDisabled(false);
    ui->btn_mqtt_connect->setFocus();
    ui->btn_mqtt_disconnect->setDisabled(true);
    ui->btn_mqtt_publish->setDisabled(true);
    ui->btn_mqtt_subscribe->setDisabled(true);

    //颜色修改
    FlatUI::setPushButtonQss(ui->btn_mqtt_connect);
    FlatUI::setPushButtonQss(ui->btn_mqtt_disconnect, 5, 8, "#d5d8dc", "#E6F8F5", "#2EE1C1", "#FFFFFF", "#16A086", "#A7EEE6");
    FlatUI::setPushButtonQss(ui->btn_mqtt_publish, 5, 8, "#d5d8dc", "#E6F8F5", "#2EE1C1", "#FFFFFF", "#16A086", "#A7EEE6");
    FlatUI::setPushButtonQss(ui->btn_mqtt_subscribe, 5, 8, "#d5d8dc", "#E6F8F5", "#2EE1C1", "#FFFFFF", "#16A086", "#A7EEE6");
}

//订阅消息
void Management::on_btn_mqtt_subscribe_clicked()
{
    clickSound->play();
    if(ui->radioButton_qos_0->isChecked())
    {
        mqtt->subscribe(ui->lineEdit_mqtt_subscribe->text(),0);
    }
    else if(ui->radioButton_qos_1->isChecked())
    {
        mqtt->subscribe(ui->lineEdit_mqtt_subscribe->text(),1);
    }
    else if(ui->radioButton_qos_2->isChecked())
    {
        mqtt->subscribe(ui->lineEdit_mqtt_subscribe->text(),2);
    }
}

void Management::MQTT_publish(QString value)
{
    QMQTT::Message msg;
    msg.setTopic(ui->lineEdit_mqtt_publish->text());
    msg.setPayload(value.toLocal8Bit());
    mqtt->publish(msg);
}

//发布消息
void Management::on_btn_mqtt_publish_clicked()
{
    clickSound->play();
    MQTT_publish(ui->lineEdit_mqtt_publish_value->text());
}

//获取园区状态
void Management::on_btn_getStatus_clicked()
{
    clickSound->play();
    logread("正在下发采集指令...");
    //获取园区状态，下发采集传感器的指令
    QMQTT::Message msg;
    QString str = "{\"deviceID\" : \"all\",\"action\" : \"1\",\"device\" : \"3\"}";
    msg.setTopic(PUBLISH_TOPIC);
    msg.setPayload(str.toLocal8Bit());
    mqtt->publish(msg);
    logread("采集指令下发成功，等待响应报文...");
}

//操作排风扇
void Management::setFan(QString deviceID, bool op)
{
    //设置排风扇状态
    QMQTT::Message msg;
    QString str = "{\"deviceID\" : \"" + deviceID + "\",\"action\" : \"0\",\"device\" : \"0\",\"value\" : \"" + QString::number(op) + "\"}";

    msg.setTopic(PUBLISH_TOPIC);
    msg.setPayload(str.toLocal8Bit());
    mqtt->publish(msg);
    if (op == 1) {
        logread("开启排气扇");
        //获取当前月份
        QDate currentDate = QDate::currentDate();
        int currentMonth = currentDate.month();
        //列表值自增1
        fanUseList.replace(currentMonth,QString::number((fanUseList.at(currentMonth).toInt())+1));
        //图表数值更新
        bar->add_BarSet(fanUseList);
    } else {
        logread("关闭排气扇");
    }
    ui->textBrowser_mqtt_log->append("发送MQTT数据:" + str);
}

//操作LED灯
void Management::setLED(QString deviceID, bool op)
{
    //设置LED状态
    QMQTT::Message msg;
    QString str = "{\"deviceID\" : \"" + deviceID + "\",\"action\" : \"0\",\"device\" : \"1\",\"value\" : \"" + QString::number(op) + "\"}";
    msg.setTopic(PUBLISH_TOPIC);
    msg.setPayload(str.toLocal8Bit());
    mqtt->publish(msg);
    if (op == 1) {
        logread("开启LED");
        //获取当前月份
        QDate currentDate = QDate::currentDate();
        int currentMonth = currentDate.month();
        //列表值自增1
        ledUseList.replace(currentMonth,QString::number((ledUseList.at(currentMonth).toInt())+1));
        //图表数值更新
        bar->add_BarSet(ledUseList);
    } else {
        logread("关闭LED");
    }
    ui->textBrowser_mqtt_log->append("发送MQTT数据:" + str);
}

//操作水泵
void Management::setPump(QString deviceID, bool op)
{
    //设置水泵状态
    QMQTT::Message msg;
    QString str = "{\"deviceID\" : \"" + deviceID + "\",\"action\" : \"0\",\"device\" : \"2\",\"time\" : \"5\",\"value\" : \"" + QString::number(op) + "\"}";
    msg.setTopic(PUBLISH_TOPIC);
    msg.setPayload(str.toLocal8Bit());
    mqtt->publish(msg);
    if (op == 1) {
        logread("开启水泵");
        //获取当前月份
        QDate currentDate = QDate::currentDate();
        int currentMonth = currentDate.month();
        //列表值自增1
        pumpUseList.replace(currentMonth,QString::number((pumpUseList.at(currentMonth).toInt())+1));
        //图表数值更新
        bar->add_BarSet(pumpUseList);
    } else {
        logread("关闭水泵");
    }
    ui->textBrowser_mqtt_log->append("发送MQTT数据:" + str);
}

//操作蜂鸣器
void Management::setBuzzer(QString deviceID, bool op)
{
    //设置蜂鸣器状态
    QMQTT::Message msg;
    QString str = "{\"deviceID\" : \"" + deviceID + "\",\"action\" : \"0\",\"device\" : \"3\",\"value\" : \"" + QString::number(op) + "\"}";
    msg.setTopic(PUBLISH_TOPIC);
    msg.setPayload(str.toLocal8Bit());
    mqtt->publish(msg);
    if (op == 1) {
        logread("开启蜂鸣器");
        //获取当前月份
        QDate currentDate = QDate::currentDate();
        int currentMonth = currentDate.month();
        //列表值自增1
        buzzerUseList.replace(currentMonth,QString::number((buzzerUseList.at(currentMonth).toInt())+1));
        //图表数值更新
        bar->add_BarSet(buzzerUseList);
    } else {
        logread("关闭蜂鸣器");
    }
    ui->textBrowser_mqtt_log->append("发送MQTT数据:" + str);
}

// JSON解析函数
int Management::Parse_Json(QString msg)
{
    QJsonParseError err_rpt;
    QJsonDocument root_Doc = QJsonDocument::fromJson(msg.toLocal8Bit(), &err_rpt); // 字符串格式化为JSON

    if (err_rpt.error != QJsonParseError::NoError) {
        qDebug() << "JSON format error";
        return -1;
    } else {
        QJsonObject root_Obj = root_Doc.object();
        QString msg = root_Obj.value("msg").toString();

        if (msg == "all_callBack") {
            QJsonValue arrValue = root_Obj.value("value");
            if(arrValue.isArray()) // 判断获取的QJsonValue对象是不是数组结构
            {
                QJsonArray array = arrValue.toArray();
                for(int i=0;i<array.size();i++)
                {
                    QJsonValue index = array.at(i);
                    QJsonObject idObject = index.toObject();
                    QString device = idObject["device"].toString();
                    QString value = idObject["value"].toString();
                    qDebug() <<device<<value;
                }
            }
        } else if (msg == "hello") {
            QString deviceID = root_Obj.value("deviceID").toString();
            QString IP = root_Obj.value("IP").toString();
            // 精确查找是否已经添加在列表中了
            QListWidgetItem* match = ui->onlineDevice->findItems(deviceID, Qt::MatchExactly | Qt::MatchCaseSensitive).value(0);
            if (match != nullptr) {
                //qDebug() << "Found '123' in the list widget";
                return 0;
            }
            //加入在线设备表格
            ui->onlineDevice->addItem(deviceID);
            device_ip.insert(deviceID, IP);
            logread("检测到设备上线！ID：" + deviceID + "，IP地址：" + IP);

            device_bind.insert(deviceID, 0);//默认没有绑定作物
        } else if (msg == "keepLive") {

        } else if (msg == "getvalue") {
            QString deviceID = root_Obj.value("deviceID").toString();
            QString IP = root_Obj.value("IP").toString();
            QString device = root_Obj.value("device").toString();
            QString value = root_Obj.value("value").toString();

            //在线设备列表没有的->没发送hello包注册
            if (!containsItem(deviceID)) {
                qDebug()<<"no find in onlinelist";
                return -1;
            }

            //没有绑定作物园区或者绑定为“无”的
            if (!device_bind.contains(deviceID) || device_bind[deviceID] == 0) {
                logread("检测到设备" + deviceID + "未绑定作物园区，请前往系统配置页面绑定！");
                return -1;
            }

            int index = device_bind[deviceID];
            QString deviceBind = ui->comboBox_devPlant->itemText(index);
            logread("收到" + deviceBind + "园区的下位机采集数据，ID号为" + deviceID + "，IP地址" + IP + "，" + device + "传感器,当前数值：" + value);

            if(device.compare("soil") == 0){
                if (value.toInt() >= ui->spinBox_humidity->text().toInt()) {
                    //土壤干旱
                    logread("当前土壤干旱！");
                    //数据表更新
                    QTableWidgetItem *item = new QTableWidgetItem("缺水");
                    ui->tableWidget->setItem(index - 1, 3, item);
                    //自动处理
                    if(ui->btn_alertOn->isEnabled() == false){
                        //开启水泵
                        setPump(deviceID, ON);
                    }
                } else {
                    //数据表更新
                    QTableWidgetItem *item = new QTableWidgetItem("健康");
                    ui->tableWidget->setItem(index - 1, 3, item);
                }
            } else if(device.compare("light") == 0){
                //模拟量模式
                if(ui->radioButton_a_mode->isChecked()) {
                    if(value.toInt() >= ui->spinBox_light->text().toInt()){
                        //环境光照不足
                        //数据表更新
                        QTableWidgetItem *item = new QTableWidgetItem("缺光照");
                        ui->tableWidget->setItem(index - 1, 3, item);
                        //自动处理
                        if(ui->btn_alertOn->isEnabled() == false){
                            //开启LED
                            setLED(deviceID, ON);
                        }
                    } else {
                        //环境光照充足
                        //数据表更新
                        QTableWidgetItem *item = new QTableWidgetItem("健康");
                        ui->tableWidget->setItem(index - 1, 3, item);
                        //自动处理
                        if(ui->btn_alertOn->isEnabled() == false){
                            //关闭LED
                            setLED(deviceID, OFF);
                        }
                    }
                } else {
                    //数字量模式
                    //环境光照不足
                    if(value.toInt() == 1){
                        logread("当前环境光照不足！");
                        //数据表更新
                        QTableWidgetItem *item = new QTableWidgetItem("缺光照");
                        ui->tableWidget->setItem(index - 1, 3, item);
                        //自动处理
                        if(ui->btn_alertOn->isEnabled() == false){
                            //开启LED
                            setLED(deviceID, ON);
                        }
                    } else {
                        //环境光照充足
                        //数据表更新
                        QTableWidgetItem *item = new QTableWidgetItem("健康");
                        ui->tableWidget->setItem(index - 1, 3, item);
                        //自动处理
                        if(ui->btn_alertOn->isEnabled() == false){
                            //关闭LED
                            setLED(deviceID, OFF);
                        }
                    }
                }
            } else if(device.compare("gas") == 0){
                //模拟量模式
                if(ui->radioButton_a_mode->isChecked()) {
                    if(value.toInt() >= ui->spinBox_gas->text().toInt()){
                        //数据表更新
                        QTableWidgetItem *item = new QTableWidgetItem("可燃气体浓度过高");
                        ui->tableWidget->setItem(index - 1, 3, item);
                        //自动处理
                        if(ui->btn_alertOn->isEnabled() == false){
                            //开启排风扇、LED、蜂鸣器
                            setFan(deviceID, ON);
                            setLED(deviceID, ON);
                            setBuzzer(deviceID, ON);
                            logread("启用告警系统，开启排风扇！");
                        }
                        //弹窗
                        QMessageBox::critical(this,"warnning","检测到可燃气体");
                    } else {
                        //数据表更新
                        QTableWidgetItem *item = new QTableWidgetItem("健康");
                        ui->tableWidget->setItem(index - 1, 3, item);
                        //自动处理
                        if(ui->btn_alertOn->isEnabled() == false){
                            //关闭排风扇、LED、蜂鸣器
                            setFan(deviceID, OFF);
                            setLED(deviceID, OFF);
                            setBuzzer(deviceID, OFF);
                        }
                    }
                } else {
                    //数字量模式
                    //浓度高
                    if(value.toInt() == 0){
                        //数据表更新
                        QTableWidgetItem *item = new QTableWidgetItem("可燃气体浓度过高");
                        ui->tableWidget->setItem(index - 1, 3, item);
                        //弹窗
                        QMessageBox::critical(this,"warnning","检测到可燃气体");
                        //自动处理
                        if(ui->btn_alertOn->isEnabled() == false){
                            //开启排风扇、LED、蜂鸣器
                            setFan(deviceID, ON);
                            setLED(deviceID, ON);
                            setBuzzer(deviceID, ON);
                            logread("启用告警系统，开启排风扇！");
                        }
                    } else {
                        //数据表更新
                        QTableWidgetItem *item = new QTableWidgetItem("健康");
                        ui->tableWidget->setItem(index - 1, 3, item);
                        //自动处理
                        if(ui->btn_alertOn->isEnabled() == false){
                            //关闭排风扇、LED、蜂鸣器
                            setFan(deviceID, OFF);
                            setLED(deviceID, OFF);
                            setBuzzer(deviceID, OFF);
                        }
                    }
                }
            }
        }
        return 0;
    }
}

bool Management::containsItem(const QString& text)
{
    for (int i = 0; i < ui->onlineDevice->count(); ++i)
    {
        QListWidgetItem* item = ui->onlineDevice->item(i);
        if (item && item->text().contains(text))
        {
            return true;
        }
    }
    return false;
}

void Management::alert(void) {
    if(doAlert == ON) {
        alertSound->setLoops(5);
        alertSound->play();
    }
}

//获取设备ID
QString Management::getDeviceID(void) {
    QString deviceID;
    if(ui->radioButton_all_device->isChecked())
    {
        deviceID = "all";
    } else {
        QListWidgetItem *item = ui->onlineDevice->currentItem();
        if (item != nullptr) {
            deviceID = item->text();
        } else {
            logread("操作失败，请先选择设备！");
            return NULL;
        }
    }
    return deviceID;
}
