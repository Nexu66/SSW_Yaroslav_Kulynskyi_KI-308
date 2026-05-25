#include "clipboardmonitor.h"
#include "databasemanager.h"
#include <QTimer>
#include <QClipboard>
#include <QGuiApplication>
#include <QDebug>

ClipboardMonitor::ClipboardMonitor(QObject *parent) : QObject(parent), m_active(false) {}

void ClipboardMonitor::start() {
    m_active = true;
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &ClipboardMonitor::checkClipboard);
    timer->start(500); // Check every 500ms
}

void ClipboardMonitor::stop() {
    m_active = false;
}

void ClipboardMonitor::checkClipboard() {
    if (!m_active) return;

    QClipboard *clipboard = QGuiApplication::clipboard();
    if (!clipboard) return;

    QString currentText = clipboard->text();
    if (!currentText.isEmpty() && currentText != m_lastClipboardText) {
        m_lastClipboardText = currentText;
        DatabaseManager::instance()->logClipboard(currentText);
        qDebug() << "Clipboard changed: " << currentText;
    }
}
