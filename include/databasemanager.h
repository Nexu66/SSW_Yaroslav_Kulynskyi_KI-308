#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDateTime>

class DatabaseManager : public QObject {
    Q_OBJECT
public:
    static DatabaseManager* instance();
    bool initDatabase();

    // Logging methods
    void logEvent(const QString& type, const QString& description);
    void logKey(const QString& key);
    void logUsb(const QString& action, const QString& deviceName);
    void logClipboard(const QString& text);

private:
    explicit DatabaseManager(QObject *parent = nullptr);
    static DatabaseManager* m_instance;
    QSqlDatabase m_db;
};

#endif // DATABASEMANAGER_H
