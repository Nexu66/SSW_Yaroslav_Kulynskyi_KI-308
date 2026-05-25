#ifndef APPMONITOR_H
#define APPMONITOR_H

#include <QObject>
#include <QTimer>
#include <QString>
#include <QDateTime>
#include <QMap>
#include <windows.h>

struct ProcessInfo {
    QString name;
    QDateTime startTime;
};

class AppMonitor : public QObject {
    Q_OBJECT
public:
    explicit AppMonitor(QObject *parent = nullptr);
    void start();
    void stop();

private slots:
    void checkProcesses(bool silentMode = false);

private:
    QTimer *m_timer;
    QMap<DWORD, ProcessInfo> m_runningProcesses;
};

#endif // APPMONITOR_H
