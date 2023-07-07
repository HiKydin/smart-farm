#ifndef QLINEEDITCALENDAR_H
#define QLINEEDITCALENDAR_H

#include <QLineEdit>
#include <QWidget>
#include <QMouseEvent>
#include "lunarcalendarwidget.h"

class QLineEditCalendar : public QLineEdit
{
    Q_OBJECT
public:
    explicit QLineEditCalendar(QWidget *parent = nullptr);
private:
    LunarCalendarWidget *pLunarCalendarWidget;
public slots:
    void slotCurrData(QString data);
protected:
    virtual void mousePressEvent(QMouseEvent *event);
};

#endif // QLINEEDITCALENDAR_H
