#include "monitors/powermonitor.h"
#include "databasemanager.h"
#include <windows.h>
#include <QDebug>

PowerMonitor::PowerMonitor(QObject *parent) : QObject(parent) {
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &PowerMonitor::checkBattery);
}

void PowerMonitor::start() {
    // Перший знімок стану батареї без логування
    SYSTEM_POWER_STATUS sps;
    if (GetSystemPowerStatus(&sps)) {
        m_lastBatteryPercent = sps.BatteryLifePercent;
        m_wasCharging = (sps.ACLineStatus == 1);
    }
    m_timer->start(30000); // перевірка кожні 30 секунд
}

void PowerMonitor::stop() {
    m_timer->stop();
}

// ─── Обробка Windows-повідомлень ────────────────────────────────────────────

void PowerMonitor::processWindowsMessage(UINT msg, WPARAM wParam) {

    if (msg == WM_POWERBROADCAST) {
        switch (wParam) {

        case PBT_APMSUSPEND:
            // Система переходить у сон
            DatabaseManager::instance()->logEvent("Power", "Система перейшла в режим сну");
            qDebug() << "Power: sleep";
            break;

        case PBT_APMRESUMEAUTOMATIC:
            // Пробудження (автоматичне або ручне — приходить завжди)
            DatabaseManager::instance()->logEvent("Power", "Система прокинулась із режиму сну");
            qDebug() << "Power: wake";
            break;

        case PBT_APMPOWERSTATUSCHANGE: {
            // Зміна статусу живлення (підключили/від'єднали зарядку)
            SYSTEM_POWER_STATUS sps;
            if (!GetSystemPowerStatus(&sps)) break;

            bool isCharging = (sps.ACLineStatus == 1);
            if (isCharging != m_wasCharging) {
                if (isCharging) {
                    DatabaseManager::instance()->logEvent("Power", "Зарядку підключено");
                } else {
                    DatabaseManager::instance()->logEvent("Power", "Зарядку від'єднано");
                }
                m_wasCharging = isCharging;
                qDebug() << "Power: charger" << (isCharging ? "connected" : "disconnected");
            }
            break;
        }

        default:
            break;
        }
    }

    // WM_WTSSESSION_CHANGE — блокування / розблокування екрана
    else if (msg == WM_WTSSESSION_CHANGE) {
        if (wParam == WTS_SESSION_LOCK) {
            DatabaseManager::instance()->logEvent("Power", "Екран заблоковано");
            qDebug() << "Power: screen locked";
        } else if (wParam == WTS_SESSION_UNLOCK) {
            DatabaseManager::instance()->logEvent("Power", "Екран розблоковано");
            qDebug() << "Power: screen unlocked";
        }
    }
}

// ─── Опитування батареї ─────────────────────────────────────────────────────

void PowerMonitor::checkBattery() {
    SYSTEM_POWER_STATUS sps;
    if (!GetSystemPowerStatus(&sps)) return;

    int current = static_cast<int>(sps.BatteryLifePercent);
    if (current == 255) return; // немає батареї (ПК на зарядці без батареї)

    // Попередження при низькому заряді (тільки якщо не заряджається)
    if (sps.ACLineStatus == 0 && current <= 20 && current != m_lastBatteryPercent) {
        DatabaseManager::instance()->logEvent(
            "Power",
            QString("Низький заряд батареї: %1%%").arg(current)
        );
        qDebug() << "Power: low battery" << current << "%";
    }

    m_lastBatteryPercent = current;
}
