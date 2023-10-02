#ifndef COMMON_H
#define COMMON_H

#include <QObject>
#include <QTextBrowser>

#define DEBUG 7
#define INFO 6

class Common : public QObject
{
    Q_OBJECT
public:
    explicit Common(QObject *parent = nullptr);
    static void logread(QTextBrowser* textBrowser,qint32 level,QString msg);
    static void setLogLevel(qint32 level);

signals:

public slots:
};

#endif // COMMON_H
