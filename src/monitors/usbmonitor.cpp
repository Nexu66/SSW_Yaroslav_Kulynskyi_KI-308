#include "monitors/usbmonitor.h"
#include "databasemanager.h"
#include <windows.h>
#include <dbt.h>
#include <QDebug>
#include <QWidget>

UsbMonitor::UsbMonitor(QObject *parent) : QObject(parent) {}

void UsbMonitor::start() {
    m_running = true;
    qDebug() << "USB Monitor started";
}

void UsbMonitor::stop() {
    m_running = false;
    qDebug() << "USB Monitor stopped";
}

void UsbMonitor::processDeviceChange(WPARAM wParam, LPARAM lParam) {
    if (wParam == DBT_DEVICEARRIVAL) {
        DatabaseManager::instance()->logUsb("Connected", "USB Device Detected");
        qDebug() << "USB Device Connected";
    } else if (wParam == DBT_DEVICEREMOVECOMPLETE) {
        DatabaseManager::instance()->logUsb("Disconnected", "USB Device Removed");
        qDebug() << "USB Device Disconnected";
    }
}
