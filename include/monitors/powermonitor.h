#ifndef POWEMONITOR_H
#define POWEMONITOR_H

#include <QObject>
#include <QTimer>

class PowerMonitor : public QObject {
    Q_OBJECT
public:
    explicit PowerMonitor(QObject *parent = nullptr);
    void start();
    void stop();

private slots:
    void checkPowerState();

private:
    QTimer *m_timer;
    int m_lastBatteryPercent;
};

#endif // POWEMONITOR_H
