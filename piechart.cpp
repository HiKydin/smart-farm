#include "piechart.h"
#include <QDebug>

piechart::piechart(QWidget *parent)
    : QWidget(parent)
    , m_ptrChart(new QChart)
    , m_ptrChartview(new QChartView(m_ptrChart, this))
    , m_ptrPieSeries(new QPieSeries)
{
    initUI();
    updateData();
    initConnect();
}

void piechart::add_PieSeries(QString name,int num)
{
    QPieSlice *slice = new QPieSlice(name, num);
    slice->setLabelVisible(true);
    m_ptrPieSeries->append(slice);
}

void piechart::initUI()
{
    m_ptrChart->addSeries(m_ptrPieSeries);
    m_ptrChart->setTitle("种植作物统计图");
    m_ptrChart->legend()->hide();

    m_ptrChartview->setRenderHint(QPainter::Antialiasing);
    this->setFixedSize(500, 400);
    m_ptrChartview->setFixedSize(500, 400);
}

void piechart::updateData()
{
    m_listColor.append(QBrush(QColor("#f15b6c")));
    m_listColor.append(QBrush(QColor("#faa755")));
    m_listColor.append(QBrush(QColor("#bed742")));
    m_listColor.append(QBrush(QColor("#afb4db")));
    m_listColor.append(QBrush(QColor("#4e72b8")));
    m_listColor.append(QBrush(QColor("#66FF99")));
    m_listColor.append(QBrush(QColor("#6633FF")));
    m_listColor.append(QBrush(QColor("#CC99FF")));

    for (int i = 0; i < m_ptrPieSeries->slices().count(); i++) {
        m_ptrPieSeries->slices()[i]->setBrush(m_listColor.at(i));
    }
}

void piechart::initConnect()
{
    connect(m_ptrPieSeries, SIGNAL(clicked(QPieSlice *)), this,
            SLOT(clickedItem(QPieSlice *)));
}

void piechart::clickedItem(QPieSlice *slice)
{
    int i = m_ptrPieSeries->slices().lastIndexOf(slice);

    if (!slice->isExploded()) {
        slice->setExploded();
        slice->setPen(QPen(Qt::darkGreen, 2));
        slice->setBrush(Qt::green);
    } else {
        slice->setExploded(false);
        slice->setPen(QPen(Qt::white, 1));
        slice->setBrush(m_listColor.at(i));
    }
}
