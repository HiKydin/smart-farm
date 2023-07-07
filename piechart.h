#ifndef PIECHART_H
#define PIECHART_H

#include <QObject>
#include <QWidget>
#include <QChart>
#include <QChartView>
#include <QPieSeries>
#include <QPieSlice>

QT_CHARTS_USE_NAMESPACE
class piechart : public QWidget{
    Q_OBJECT

public:
    piechart(QWidget *parent = nullptr);
    void initUI();
    void initConnect();

public slots:
    void clickedItem(QPieSlice *slice);
    void add_PieSeries(QString name,int num);
    void updateData();

private:
    QList<QBrush> m_listColor;
    QChart *m_ptrChart;
    QChartView *m_ptrChartview;
    QPieSeries *m_ptrPieSeries;
};

#endif // PIECHART_H
