#include "appmonitor.h"
#include "databasemanager.h"
#include <windows.h>
#include <psapi.h>
#include <QDebug>
#include <tlhelp32.h>

AppMonitor::AppMonitor(QObject *parent) : QObject(parent) {
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, [this]() { checkProcesses(false); });
}

void AppMonitor::start() {
    // Перший запуск: просто заповнюємо список поточних процесів без логування
    checkProcesses(true);
    m_timer->start(1000); // Check every second
}

void AppMonitor::stop() {
    m_timer->stop();
}

void AppMonitor::checkProcesses(bool silentMode) {
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        qDebug() << "Failed to create process snapshot";
        return;
    }

    QMap<DWORD, ProcessInfo> currentScanPids;
    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);

    if (!Process32First(hSnapshot, &pe32)) {
        CloseHandle(hSnapshot);
        return;
    }

    do {
        DWORD pid = pe32.th32ProcessID;
        QString processName = QString::fromWCharArray(pe32.szExeFile);
        currentScanPids[pid] = {processName, QDateTime::currentDateTime()};
    } while (Process32Next(hSnapshot, &pe32));

    CloseHandle(hSnapshot);

    // 1. Check for new processes
    for (auto it = currentScanPids.begin(); it != currentScanPids.end(); ++it) {
        if (!m_runningProcesses.contains(it.key())) {
            m_runningProcesses[it.key()] = it.value();
            if (!silentMode) {
                DatabaseManager::instance()->logEvent("AppOpen", it.value().name + " started");
            }
        }
    }

    // 2. Check for terminated processes
    auto it = m_runningProcesses.begin();
    while (it != m_runningProcesses.end()) {
        if (!currentScanPids.contains(it.key())) {
            if (!silentMode) {
                qint64 durationSecs = it.value().startTime.secsTo(QDateTime::currentDateTime());
                int hours = durationSecs / 3600;
                int minutes = (durationSecs % 3600) / 60;
                int seconds = durationSecs % 60;
                QString durationStr = QString(" | Тривалість життя: %1год %2хв %3сек").arg(hours).arg(minutes).arg(seconds);

                DatabaseManager::instance()->logEvent("AppClose", it.value().name + " terminated" + durationStr);
            }
            it = m_runningProcesses.erase(it);
        } else {
            ++it;
        }
    }
}
