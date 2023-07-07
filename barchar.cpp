#include "barchar.h"
barchar::barchar(QWidget *parent)
    : QWidget(parent)
    , m_ptrChart(new QChart)
    , m_ptrChartview(new QChartView(m_ptrChart, this))
    , m_ptrBarSeries(new QBarSeries)
    , m_ptrAxisY(new QValueAxis)
    , m_ptrAxisX(new QBarCategoryAxis)
{
    qsrand(static_cast<uint>(QTime(0, 0, 0).secsTo(QTime::currentTime())));
    initUI();
    initConnect();
}

barchar::~barchar()
{
    foreach (auto item, m_listBarSet) {
        delete item;
    }
    m_listBarSet.clear();
}

void barchar::initUI()
{
    m_ptrChart->addSeries(m_ptrBarSeries);
    m_ptrChartview->setFixedSize(1200, 400);
    m_ptrAxisX->append(tr("一月"));
    m_ptrAxisX->append(tr("二月"));
    m_ptrAxisX->append(tr("三月"));
    m_ptrAxisX->append(tr("四月"));
    m_ptrAxisX->append(tr("五月"));
    m_ptrAxisX->append(tr("六月"));
    m_ptrAxisX->append(tr("七月"));
    m_ptrAxisX->append(tr("八月"));
    m_ptrAxisX->append(tr("九月"));
    m_ptrAxisX->append(tr("十月"));
    m_ptrAxisX->append(tr("十一月"));
    m_ptrAxisX->append(tr("十二月"));
    m_ptrAxisX->setLabelsColor(QColor(7, 28, 96));
    m_ptrAxisY->setRange(0, 100);
    m_ptrAxisY->setTickCount(11);
    m_ptrChart->setAxisX(m_ptrAxisX, m_ptrBarSeries);
    m_ptrChart->setAxisY(m_ptrAxisY, m_ptrBarSeries);

    m_ptrChart->setTitle(tr("设备使用次数"));
    m_ptrChart->legend()->setVisible(true);
    //是否允许对legend进行设置，就相当于一个总开关，只有打开了才能对legend进行操作
    m_ptrChart->legend()->setAlignment(Qt::AlignBottom);  //设置位置
    m_ptrChart->legend()->setBackgroundVisible(true);
    m_ptrChart->legend()->setAutoFillBackground(true);
    m_ptrChart->legend()->setColor(QColor(222, 233, 251));  //设置颜色
    m_ptrChart->legend()->setLabelColor(QColor(0, 100, 255));
    //设置标签颜色 m_ptrChart->legend()->setMaximumHeight(50);
    //设置最大高度

    initBarSet();
}

void barchar::add_BarSet(QStringList strList)
{
    if(strList.at(0) == "fan"){
        for(int i = 1; i < strList.length(); i++){
            m_listBarSet.at(0)->replace(i-1,strList.at(i).toInt());
        }
    } else if(strList.at(0) == "LED"){
        for(int i = 1; i < strList.length(); i++){
            m_listBarSet.at(1)->replace(i-1,strList.at(i).toInt());
        }
    } else if(strList.at(0) == "pump"){
        for(int i = 1; i < strList.length(); i++){
            m_listBarSet.at(2)->replace(i-1,strList.at(i).toInt());
        }
    } else if(strList.at(0) == "buzzer"){
        for(int i = 1; i < strList.length(); i++){
            m_listBarSet.at(3)->replace(i-1,strList.at(i).toInt());
        }
    }
}

void barchar::initBarSet()
{
    set0 = new QBarSet("排风扇");
    set1 = new QBarSet("LED");
    set2 = new QBarSet("水泵");
    set3 = new QBarSet("蜂鸣器");

    *set0 << 0 << 0 << 0 << 0 << 0 << 0 << 0 << 0 << 0 << 0 << 0 << 0;
    *set1 << 0 << 0 << 0 << 0 << 0 << 0 << 0 << 0 << 0 << 0 << 0 << 0;
    *set2 << 0 << 0 << 0 << 0 << 0 << 0 << 0 << 0 << 0 << 0 << 0 << 0;
    *set3 << 0 << 0 << 0 << 0 << 0 << 0 << 0 << 0 << 0 << 0 << 0 << 0;
    m_listBarSet.append(set0);
    m_listBarSet.append(set1);
    m_listBarSet.append(set2);
    m_listBarSet.append(set3);

    foreach (auto item, m_listBarSet) {
        m_ptrBarSeries->append(item);
    }
}

void barchar::initConnect()
{
    const auto markers = m_ptrChart->legend()->markers();
    for (QLegendMarker *marker : markers) {
        // Disconnect possible existing connection to avoid multiple connections
        QObject::disconnect(marker, &QLegendMarker::clicked, this,
                            &barchar::handleMarkerClicked);
        QObject::connect(marker, &QLegendMarker::clicked, this,
                         &barchar::handleMarkerClicked);
    }
}

//点击隐藏
void barchar::handleMarkerClicked()
{
    QBarLegendMarker *marker = qobject_cast<QBarLegendMarker *>(sender());
    //断言
    Q_ASSERT(marker);
    switch (marker->type()) {
    case QLegendMarker::LegendMarkerTypeBar: {
        //控序列隐藏/显示
        // Toggle visibility of series
        // marker->setVisible(false);
        // marker->series()->setVisible(!marker->series()->isVisible());

        // Turn legend marker back to visible, since hiding series also
        // hides the marker and we don't want it to happen now.
        // 获取当前选中的BarSet.我们可以通过修改barSet的透明度设置其隐藏
        QBarSet *barset = marker->barset();
        QColor barsetColor(barset->color());
        qreal alpha = barsetColor.alphaF();
        marker->setVisible(true);

        //修改图例
        // Dim the marker, if series is not visible

        if (1 == alpha) {
            alpha = 0.1;
        } else {
            alpha = 1;
        }

        QColor color;
        QBrush brush = marker->labelBrush();
        color = brush.color();
        color.setAlphaF(alpha);
        brush.setColor(color);
        marker->setLabelBrush(brush);

        brush = marker->brush();
        color = brush.color();
        color.setAlphaF(alpha);
        brush.setColor(color);
        marker->setBrush(brush);

        QPen pen = marker->pen();
        color = pen.color();
        color.setAlphaF(alpha);
        pen.setColor(color);
        marker->setPen(pen);

        barsetColor.setAlphaF(alpha);
        barset->setColor(barsetColor);

        break;
    }
    default: {
        qInfo() << "Unknown marker type";
        break;
    }
    }
}

//void barchar::mousePressEvent(QMouseEvent *)
//{
//    foreach (auto item, m_listBarSet) {
//        for (int i = 0; i < item->count(); i++) {
//            item->remove(i);
//        }
//    }
//    foreach (auto item, m_listBarSet) {
//        for (int i = 0; i < 5; i++) {
//            item->append(qrand() % 600);
//        }
//    }
//}
