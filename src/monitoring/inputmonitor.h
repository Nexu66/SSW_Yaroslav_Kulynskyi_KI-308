#ifndef INPUTMONITOR_H
#define INPUTMONITOR_H

#include <QObject>
#include <windows.h>

class InputMonitor : public QObject {
    Q_OBJECT
public:
    explicit InputMonitor(QObject *parent = nullptr);
    void start();
    void stop();

    static LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);

private:
    HHOOK m_hKeyboardHook;
};

#endif // INPUTMONITOR_H
