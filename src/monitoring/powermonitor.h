#ifndef POWERMONITOR_H
#define POWERMONITOR_H

#include <QObject>
#include <QTimer>
#include <windows.h>

class PowerMonitor : public QObject {
    Q_OBJECT
public:
    explicit PowerMonitor(QObject *parent = nullptr);
    void start();
    void stop();

    void processWindowsMessage(UINT msg, WPARAM wParam);

private slots:
    void checkBattery();

private:
    QTimer *m_timer;
    int m_lastBatteryPercent = -1;
    bool m_wasCharging = false;
};

#endif // POWERMONITOR_H
