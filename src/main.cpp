#include <QApplication>
#include "mainwindow.h"
#include "databasemanager.h"
#include <QMessageBox>

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    if (!DatabaseManager::instance()->initDatabase()) {
        QMessageBox::critical(nullptr, "Помилка", "Не вдалося ініціалізувати базу даних!");
        return -1;
    }

    MainWindow w;
    w.show();
    return a.exec();
}
