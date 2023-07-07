#include "login.h"
#include "management.h"
#include "flatui.h"

Login::Login(QWidget *parent) : QWidget(parent)
{
    /*******************界面配置*************************/
    //登录界面配置
    this->setFixedSize(500,500);
    this->setWindowTitle("智慧农场 - 登陆界面 v0.2");

    //添加按钮
    QPushButton *btn_Signin = new QPushButton("注册");
    btn_Signin->setParent(this);
    btn_Signin->setFixedSize(100,50);
    btn_Signin->move((this->width()/2)-btn_Signin->width()-25,this->height()/2+100);
    btn_Signin->setDisabled(true);

    QPushButton *btn_Login = new QPushButton("登录");
    btn_Login->setParent(this);
    btn_Login->setFixedSize(100,50);
    btn_Login->move((this->width()/2+25),this->height()/2+100);

    //使用QSS样式库美化界面
    //按钮区:
    FlatUI::setPushButtonQss(btn_Login);
    FlatUI::setPushButtonQss(btn_Signin, 5, 8, "#d5d8dc", "#E6F8F5", "#2EE1C1", "#FFFFFF", "#16A086", "#A7EEE6");

    //创建文本框
    QLineEdit *usernameText = new QLineEdit(this);
    QLineEdit *userpwdText = new QLineEdit(this);
    usernameText->resize(200,usernameText->height());
    userpwdText->resize(200,userpwdText->height());
    usernameText->move(165,200);
    userpwdText->move(165,250);
    userpwdText->setEchoMode(QLineEdit::Password);

    usernameText->setStyleSheet("font: 25 14pt '微软雅黑 Light';" //字体
                                "color: rgb(31,31,31);"		//字体颜色
                                "padding-left:20px;"       //内边距-字体缩进
                                "background-color: rgb(255, 255, 255);" //背景颜色
                                "border:2px solid rgb(0,0,0);border-radius:15px;");//边框粗细-颜色-圆角设置
    userpwdText->setStyleSheet("font: 25 14pt '微软雅黑 Light';" //字体
                               "color: rgb(31,31,31);"		//字体颜色
                               "padding-left:20px;"       //内边距-字体缩进
                               "background-color: rgb(255, 255, 255);" //背景颜色
                               "border:2px solid rgb(0,0,0);border-radius:15px;");//边框粗细-颜色-圆角设置

    usernameText->setPlaceholderText("请输入用户名");
    usernameText->setClearButtonEnabled(true);
    userpwdText->setPlaceholderText("请输入密码");
    userpwdText->setClearButtonEnabled(true);

    //创建文本
    QFont lbl_font;
    lbl_font.setFamily("微软雅黑 Light");
    lbl_font.setPixelSize(14);

    QLabel *usernamelbl = new QLabel(this);
    usernamelbl->setFont(lbl_font);
    usernamelbl->setText("管理员账号：");
    QLabel *userpwdlbl = new QLabel(this);
    userpwdlbl->setFont(lbl_font);
    userpwdlbl->setText("管理员密码：");

    usernamelbl->move(85,205);
    userpwdlbl->move(85,255);

    //水平垂直居中
    usernamelbl->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    userpwdlbl->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    //鼠标穿透事件
    usernamelbl->setAttribute(Qt::WA_TransparentForMouseEvents);
    userpwdlbl->setAttribute(Qt::WA_TransparentForMouseEvents);

    //设置主标题
    QLabel *maintitle = new QLabel(this);
    maintitle->setText("智慧农场");
    QFont font;
    font.setFamily("华文彩云");
    font.setPixelSize(36);
    maintitle->setFont(font);
    //自适应大小
    maintitle->adjustSize();
    maintitle->move(210,90);

    /*******************功能配置*************************/

    Management *managePage = new Management();

    connect(btn_Login,&QPushButton::clicked,[=](){
        if(usernameText->text() == Account && userpwdText->text() == Passwd)
        {
            this->hide();
            //managePage->setGeometry(this->geometry());
            managePage->show();
        }
        else if(usernameText->text()=="" || userpwdText->text()== "")
        {
            QMessageBox::critical(this,"error","请输入用户名和密码");
            //点击弹窗后要设置焦点
            usernameText->setFocus();
        }
        else
        {
            QMessageBox::critical(this,"error","请检查您的账号密码");
            usernameText->clear();
            userpwdText->clear();
            //点击弹窗后要设置焦点
            usernameText->setFocus();
        }
    });

    // 绑定回车登录
    btn_Login->setShortcut(Qt::Key_Return);//对应键盘上面大的回车键
}

void Login::paintEvent(QPaintEvent *)
{
    //绘制主图
    QPainter painter(this);

    //初始化pix资源
    QPixmap pix;
    QString pixPath = ":/img/home.png";
    pix.load(pixPath);
    pix = pix.scaled(pix.width()*0.4,pix.height()*0.4);

    //设置抗锯齿
    painter.setRenderHint(QPainter::Antialiasing);

    painter.drawPixmap(110,60,pix);
}
