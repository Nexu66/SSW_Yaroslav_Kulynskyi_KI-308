#ifndef ACTIVITYCHART_H
#define ACTIVITYCHART_H

#include <QWidget>
#include <QPainter>
#include <QPointF>
#include <QMap>

class ActivityChart : public QWidget {
    Q_OBJECT
public:
    explicit ActivityChart(QWidget *parent = nullptr);
    void setData(const QMap<int, double>& data);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QMap<int, double> m_data;
};

#endif // ACTIVITYCHART_H
