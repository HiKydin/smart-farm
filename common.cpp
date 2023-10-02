#include <QDateTime>

#include "common.h"

Common::Common(QObject *parent) : QObject(parent)
{

}

void Common::setLogLevel(qint32 level){

}

//日志输出接口
void Common::logread(QTextBrowser* textBrowser,qint32 level,QString msg){
    QDateTime time = QDateTime::currentDateTime();   //获取当前时间
    textBrowser->append(time.toString("yyyy-MM-dd hh:mm:ss ")+msg);
}
