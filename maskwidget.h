#ifndef MASKWIDGET_H
#define MASKWIDGET_H

#include <QWidget>

#ifdef quc
class Q_DECL_EXPORT MaskWidget : public QWidget
#else
class MaskWidget : public QWidget
#endif

{
    Q_OBJECT
public:
    static MaskWidget *Instance();
    explicit MaskWidget(QWidget *parent = 0);

protected:
    void showEvent(QShowEvent *);
    bool eventFilter(QObject *obj, QEvent *event);

private:
    static QScopedPointer<MaskWidget> self;

    //需要遮罩的主窗体
    QWidget *mainWidget;
    //需要弹窗的窗体对象名称集合链表
    QStringList dialogNames;

public Q_SLOTS:
    //设置需要遮罩的主窗体
    void setMainWidget(QWidget *mainWidget);
    //设置需要弹窗的窗体对象名称集合链表
    void setDialogNames(const QStringList &dialogNames);

    //设置遮罩颜色
    void setBgColor(const QColor &bgColor);
    //设置颜色透明度
    void setOpacity(double opacity);
};

#endif // MASKWIDGET_H
