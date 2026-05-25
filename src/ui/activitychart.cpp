#include "activitychart.h"
#include <QPainter>
#include <QPaintEvent>
#include <algorithm>

ActivityChart::ActivityChart(QWidget *parent) : QWidget(parent) {
    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(true);
}

void ActivityChart::setData(const QMap<int, double>& data) {
    m_data = data;
    update(); // Trigger repaint
}

void ActivityChart::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    int w = width();
    int h = height();
    int padding = 40;

    // Draw axes
    painter.setPen(Qt::black);
    painter.drawLine(padding, h - padding, w - padding, h - padding); // X axis
    painter.drawLine(padding, padding, padding, h - padding); // Y axis

    if (m_data.isEmpty()) return;

    // Find max value for scaling
    double maxVal = 0;
    for (auto it = m_data.begin(); it != m_data.end(); ++it) {
        if (it.value() > maxVal) maxVal = it.value();
    }
    if (maxVal == 0) maxVal = 1;

    // Draw data line
    painter.setPen(QPen(Qt::blue, 2));
    QPointF lastPoint;
    bool first = true;

    for (int hour = 0; hour < 24; ++hour) {
        double val = m_data.value(hour, 0.0);
        float x = padding + (hour * (w - 2 * padding) / 23.0f);
        float y = (h - padding) - (val / maxVal * (h - 2 * padding));

        QPointF currentPoint(x, y);
        if (!first) {
            painter.drawLine(lastPoint, currentPoint);
        }
        lastPoint = currentPoint;
        first = false;

        // Draw hour labels
        painter.setPen(Qt::gray);
        painter.drawText(x - 10, h - padding + 20, QString::number(hour) + ":00");
        painter.setPen(QPen(Qt::blue, 2));
    }
}
