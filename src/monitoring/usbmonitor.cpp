#include "usbmonitor.h"
#include "databasemanager.h"
#include <windows.h>
#include <dbt.h>
#include <QDebug>

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
    if (!m_running || !lParam) return;
    if (wParam != DBT_DEVICEARRIVAL && wParam != DBT_DEVICEREMOVECOMPLETE) return;

    DEV_BROADCAST_HDR *pHdr = reinterpret_cast<DEV_BROADCAST_HDR *>(lParam);
    if (pHdr->dbch_devicetype != DBT_DEVTYP_DEVICEINTERFACE) return;

    DEV_BROADCAST_DEVICEINTERFACE *pDev =
        reinterpret_cast<DEV_BROADCAST_DEVICEINTERFACE *>(lParam);

    // Витягуємо VID/PID з шляху \\?\USB#VID_xxxx&PID_xxxx#...
    QString rawName = QString::fromWCharArray(pDev->dbcc_name);
    QString deviceName;
    QStringList parts = rawName.split('#');
    if (parts.size() >= 2) {
        QString bus = parts[0].replace("\\\\?\\", "").replace("\\??\\", "");
        deviceName  = bus + " — " + parts[1];
    } else {
        deviceName = rawName;
    }

    QString action = (wParam == DBT_DEVICEARRIVAL) ? "Connected" : "Disconnected";

    QDateTime now = QDateTime::currentDateTime();
    if (action == m_lastAction &&
        deviceName == m_lastDevice &&
        m_lastTime.isValid() &&
        m_lastTime.msecsTo(now) < 2000)
    {
        qDebug() << "USB duplicate suppressed:" << action << deviceName;
        return;
    }

    m_lastAction = action;
    m_lastDevice = deviceName;
    m_lastTime   = now;

    DatabaseManager::instance()->logUsb(action, deviceName);
    qDebug() << "USB" << action << ":" << deviceName;
}
