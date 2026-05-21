#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "databasemanager.h"
#include "activitychart.h"
#include <QHeaderView>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QTimer>
#include <QVBoxLayout>
#include <QPushButton>
#include <windows.h>
#include <dbt.h>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_activityChart = new ActivityChart(this);
    QVBoxLayout *chartLayout = new QVBoxLayout(ui->chartWidget);
    chartLayout->setContentsMargins(0,0,0,0);
    chartLayout->addWidget(m_activityChart);
    ui->chartWidget->setLayout(chartLayout);

    startMonitoring();

    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::refreshLogs);
    timer->start(5000);

    refreshLogs();
    updateHeatmap();
}

MainWindow::~MainWindow() {
    delete ui;
    m_appMon->stop();
    m_inputMon->stop();
    m_usbMon->stop();
    m_clipMon->stop();
    m_powerMon->stop();
}

void MainWindow::startMonitoring() {
    m_appMon = new AppMonitor(this);
    m_inputMon = new InputMonitor(this);
    m_usbMon = new UsbMonitor(this);
    m_clipMon = new ClipboardMonitor(this);
    m_powerMon = new PowerMonitor(this);

    m_appMon->start();
    m_inputMon->start();
    m_usbMon->start();
    m_clipMon->start();
    m_powerMon->start();

    // Register for USB device notifications using the main window handle
    DEV_BROADCAST_DEVICEINTERFACE notificationFilter = {0};
    notificationFilter.dbcc_size = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
    notificationFilter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;

    if (!RegisterDeviceNotification((HWND)this->winId(), &notificationFilter, 0)) {
        qDebug() << "Failed to register USB device notification in MainWindow";
    } else {
        qDebug() << "USB device notification registered successfully in MainWindow";
    }
}

void MainWindow::refreshLogs() {
    ui->logTable->setRowCount(0);
    QSqlQuery query("SELECT timestamp, type, description FROM events ORDER BY timestamp DESC LIMIT 100");

    int row = 0;
    while (query.next()) {
        ui->logTable->insertRow(row);
        ui->logTable->setItem(row, 0, new QTableWidgetItem(query.value(0).toString()));
        ui->logTable->setItem(row, 1, new QTableWidgetItem(query.value(1).toString()));
        ui->logTable->setItem(row, 2, new QTableWidgetItem(query.value(2).toString()));
        row++;
    }
    updateHeatmap();

    // Оновлення таблиці активності введення
    ui->keylogTable->setRowCount(0);
    QSqlQuery keyQuery("SELECT timestamp, key FROM keylogs ORDER BY timestamp DESC LIMIT 100");
    int keyRow = 0;
    while (keyQuery.next()) {
        ui->keylogTable->insertRow(keyRow);
        ui->keylogTable->setItem(keyRow, 0, new QTableWidgetItem(keyQuery.value(0).toString()));
        ui->keylogTable->setItem(keyRow, 1, new QTableWidgetItem(keyQuery.value(1).toString()));
        keyRow++;
    }

    // Оновлення таблиці буфера обміну
    ui->clipboardTable->setRowCount(0);
    QSqlQuery clipQuery("SELECT timestamp, content FROM clipboard ORDER BY timestamp DESC LIMIT 100");
    int clipRow = 0;
    while (clipQuery.next()) {
        ui->clipboardTable->insertRow(clipRow);
        ui->clipboardTable->setItem(clipRow, 0, new QTableWidgetItem(clipQuery.value(0).toString()));
        ui->clipboardTable->setItem(clipRow, 1, new QTableWidgetItem(clipQuery.value(1).toString()));
        clipRow++;
    }

    // Оновлення таблиці USB моніторингу
    ui->usbTable->setRowCount(0);
    QSqlQuery usbQuery("SELECT timestamp, description FROM events WHERE type = 'USB' ORDER BY timestamp DESC LIMIT 100");
    int usbRow = 0;
    while (usbQuery.next()) {
        ui->usbTable->insertRow(usbRow);
        ui->usbTable->setItem(usbRow, 0, new QTableWidgetItem(usbQuery.value(0).toString()));
        ui->usbTable->setItem(usbRow, 1, new QTableWidgetItem(usbQuery.value(1).toString()));
        usbRow++;
    }
}

void MainWindow::updateHeatmap() {
    QMap<int, double> hourlyData;
    for(int i=0; i<24; ++i) hourlyData[i] = 0;

    QSqlQuery query("SELECT strftime('%H', timestamp, 'localtime') as hour, COUNT(*) as count "
                   "FROM (SELECT timestamp FROM events UNION ALL SELECT timestamp FROM keylogs UNION ALL SELECT timestamp FROM clipboard) "
                   "WHERE date(timestamp, 'localtime') = date('now', 'localtime') "
                   "GROUP BY hour ORDER BY hour ASC");

    while (query.next()) {
        int hour = query.value(0).toInt();
        double count = query.value(1).toDouble();
        hourlyData[hour] = count;
    }

    m_activityChart->setData(hourlyData);
}
