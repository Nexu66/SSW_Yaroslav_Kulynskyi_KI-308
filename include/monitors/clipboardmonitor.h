#ifndef CLIPBOARDMONITOR_H
#define CLIPBOARDMONITOR_H

#include <QObject>
#include <windows.h>

class ClipboardMonitor : public QObject {
    Q_OBJECT
public:
    explicit ClipboardMonitor(QObject *parent = nullptr);
    void start();
    void stop();

private:
    void checkClipboard();
    QString m_lastClipboardText;
    bool m_active;
};

#endif // CLIPBOARDMONITOR_H
