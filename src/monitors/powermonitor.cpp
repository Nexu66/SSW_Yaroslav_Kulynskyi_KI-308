#include "monitors/powermonitor.h"
#include "databasemanager.h"
#include <windows.h>
#include <QDebug>

PowerMonitor::PowerMonitor(QObject *parent) : QObject(parent), m_lastBatteryPercent(-1) {
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &PowerMonitor::checkPowerState);
}

void PowerMonitor::start() {
    m_timer->start(30000); // Check battery every 30 seconds
}

void PowerMonitor::stop() {
    m_timer->stop();
}

void PowerMonitor::checkPowerState() {
    SYSTEM_POWER_STATUS sps;
    if (GetSystemPowerStatus(&sps)) {
        int currentPercent = sps.BatteryLifePercent;

        if (currentPercent != -1 && currentPercent != m_lastBatteryPercent) {
            if (currentPercent < 20) {
                DatabaseManager::instance()->logEvent("Power", QString("Low battery warning: %1%").arg(currentPercent));
            }
            m_lastBatteryPercent = currentPercent;
        }
    }
}
