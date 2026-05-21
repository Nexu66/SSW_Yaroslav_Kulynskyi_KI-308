#ifndef USBMONITOR_H
#define USBMONITOR_H

#include <QObject>
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
};

#endif // USBMONITOR_H
