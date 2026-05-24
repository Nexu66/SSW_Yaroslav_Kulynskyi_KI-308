#ifndef USBMONITOR_H
#define USBMONITOR_H

#include <QObject>
#include <QDateTime>
#include <QString>
#include <windows.h>

class UsbMonitor : public QObject {
    Q_OBJECT
public:
    explicit UsbMonitor(QObject *parent = nullptr);
    void start();
    void stop();
    void processDeviceChange(WPARAM wParam, LPARAM lParam);

private:
    bool m_running = false;

    // Дедублікація: запам'ятовуємо останню подію
    QString   m_lastAction;
    QString   m_lastDevice;
    QDateTime m_lastTime;
};

#endif // USBMONITOR_H
