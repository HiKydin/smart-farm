#ifndef BARCHAR_H
#define BARCHAR_H

#include <QWidget>
#include <QChart>
#include <QChartView>
#include <QBarSeries>
#include <QBarSet>
#include <QValueAxis>
#include <QBarCategoryAxis>
#include <QTime>
#include <QBarLegendMarker>
#include <QtDebug>

QT_CHARTS_USE_NAMESPACE
class barchar : public QWidget
{
    Q_OBJECT
public:
    explicit barchar(QWidget *parent = nullptr);
    ~barchar();
    void initUI();
    void initBarSet();
    void initConnect();
    void add_BarSet(QStringList strList);

signals:

public slots:
    void handleMarkerClicked();

private:
    QChart *m_ptrChart;
    QChartView *m_ptrChartview;
    QBarSeries *m_ptrBarSeries;
    QValueAxis *m_ptrAxisY;
    QBarCategoryAxis *m_ptrAxisX;
    QList<QBarSet *> m_listBarSet;
    QBarSet *set0;
    QBarSet *set1;
    QBarSet *set2;
    QBarSet *set3;

//protected:
//    void mousePressEvent(QMouseEvent *event);
};

#endif // BARCHAR_H
