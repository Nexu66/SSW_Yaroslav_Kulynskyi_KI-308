#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "databasemanager.h"
#include "activitychart.h"
#include <QHeaderView>
#include <QSqlQuery>
#include <QTimer>
#include <QVBoxLayout>
#include <QDebug>
#include <windows.h>
#include <dbt.h>
#include <wtsapi32.h>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // ── Налаштування існуючих таблиць ────────────────────────────────────────
    ui->logTable->setHorizontalHeaderLabels({"Час", "Тип", "Опис"});
    ui->logTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    ui->logTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->logTable->setSelectionBehavior(QAbstractItemView::SelectRows);

    ui->keylogTable->setHorizontalHeaderLabels({"Час", "Клавіша"});
    ui->keylogTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    ui->keylogTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    ui->clipboardTable->setHorizontalHeaderLabels({"Час", "Вміст"});
    ui->clipboardTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    ui->clipboardTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    ui->usbTable->setHorizontalHeaderLabels({"Час", "Подія"});
    ui->usbTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    ui->usbTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    m_powerTable = new QTableWidget(0, 3, this);
    m_powerTable->setHorizontalHeaderLabels({"Час", "Тип", "Подія"});
    m_powerTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    m_powerTable->horizontalHeader()->resizeSection(0, 160);
    m_powerTable->horizontalHeader()->resizeSection(1, 100);
    m_powerTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_powerTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_powerTable->setAlternatingRowColors(true);

    QWidget     *powerTab    = new QWidget(this);
    QVBoxLayout *powerLayout = new QVBoxLayout(powerTab);
    powerLayout->setContentsMargins(4, 4, 4, 4);
    powerLayout->addWidget(m_powerTable);

    ui->tabWidget->addTab(powerTab, "Живлення");

    // ── Графік активності ────────────────────────────────────────────────────
    m_activityChart = new ActivityChart(this);
    QVBoxLayout *chartLayout = new QVBoxLayout(ui->chartWidget);
    chartLayout->setContentsMargins(0, 0, 0, 0);
    chartLayout->addWidget(m_activityChart);

    // ── Запуск моніторингу та таймер оновлення ───────────────────────────────
    startMonitoring();

    connect(ui->refreshBtn, &QPushButton::clicked, this, &MainWindow::refreshLogs);

    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::refreshLogs);
    timer->start(5000);

    refreshLogs();
    updateHeatmap();
}

MainWindow::~MainWindow() {
    WTSUnRegisterSessionNotification((HWND)this->winId());
    delete ui;
    m_appMon->stop();
    m_inputMon->stop();
    m_usbMon->stop();
    m_clipMon->stop();
    m_powerMon->stop();
}

void MainWindow::startMonitoring() {
    m_appMon   = new AppMonitor(this);
    m_inputMon = new InputMonitor(this);
    m_usbMon   = new UsbMonitor(this);
    m_clipMon  = new ClipboardMonitor(this);
    m_powerMon = new PowerMonitor(this);

    m_appMon->start();
    m_inputMon->start();
    m_usbMon->start();
    m_clipMon->start();
    m_powerMon->start();

    // Реєстрація USB-сповіщень
    DEV_BROADCAST_DEVICEINTERFACE notificationFilter = {};
    notificationFilter.dbcc_size       = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
    notificationFilter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;

    HDEVNOTIFY hUsb = RegisterDeviceNotification(
        (HWND)this->winId(),
        &notificationFilter,
        DEVICE_NOTIFY_WINDOW_HANDLE | DEVICE_NOTIFY_ALL_INTERFACE_CLASSES
    );
    if (!hUsb)
        qDebug() << "RegisterDeviceNotification FAILED:" << GetLastError();
    else
        qDebug() << "RegisterDeviceNotification OK";

    // Реєстрація сесійних сповіщень (блокування / розблокування екрана)
    if (!WTSRegisterSessionNotification((HWND)this->winId(), NOTIFY_FOR_THIS_SESSION))
        qDebug() << "WTSRegisterSessionNotification FAILED:" << GetLastError();
    else
        qDebug() << "WTSRegisterSessionNotification OK";
}

// ── Перехоплення Windows-повідомлень ─────────────────────────────────────────

bool MainWindow::nativeEvent(const QByteArray &eventType, void *message, qintptr *result) {
    Q_UNUSED(eventType);
    Q_UNUSED(result);

    MSG *msg = static_cast<MSG *>(message);

    switch (msg->message) {
    case WM_DEVICECHANGE:
        m_usbMon->processDeviceChange(msg->wParam, msg->lParam);
        break;
    case WM_POWERBROADCAST:
        m_powerMon->processWindowsMessage(WM_POWERBROADCAST, msg->wParam);
        break;
    case WM_WTSSESSION_CHANGE:
        m_powerMon->processWindowsMessage(WM_WTSSESSION_CHANGE, msg->wParam);
        break;
    default:
        break;
    }

    return false;
}

// ── Оновлення таблиць ─────────────────────────────────────────────────────────

void MainWindow::refreshLogs() {
    // Загальний лог
    ui->logTable->setRowCount(0);
    QSqlQuery q("SELECT timestamp, type, description FROM events ORDER BY timestamp DESC LIMIT 100");
    for (int row = 0; q.next(); ++row) {
        ui->logTable->insertRow(row);
        ui->logTable->setItem(row, 0, new QTableWidgetItem(q.value(0).toString()));
        ui->logTable->setItem(row, 1, new QTableWidgetItem(q.value(1).toString()));
        ui->logTable->setItem(row, 2, new QTableWidgetItem(q.value(2).toString()));
    }

    // Keylogger
    ui->keylogTable->setRowCount(0);
    QSqlQuery qk("SELECT timestamp, key FROM keylogs ORDER BY timestamp DESC LIMIT 100");
    for (int row = 0; qk.next(); ++row) {
        ui->keylogTable->insertRow(row);
        ui->keylogTable->setItem(row, 0, new QTableWidgetItem(qk.value(0).toString()));
        ui->keylogTable->setItem(row, 1, new QTableWidgetItem(qk.value(1).toString()));
    }

    // Буфер обміну
    ui->clipboardTable->setRowCount(0);
    QSqlQuery qc("SELECT timestamp, content FROM clipboard ORDER BY timestamp DESC LIMIT 100");
    for (int row = 0; qc.next(); ++row) {
        ui->clipboardTable->insertRow(row);
        ui->clipboardTable->setItem(row, 0, new QTableWidgetItem(qc.value(0).toString()));
        ui->clipboardTable->setItem(row, 1, new QTableWidgetItem(qc.value(1).toString()));
    }

    // USB
    ui->usbTable->setRowCount(0);
    QSqlQuery qu("SELECT timestamp, description FROM events WHERE type='USB' ORDER BY timestamp DESC LIMIT 100");
    for (int row = 0; qu.next(); ++row) {
        ui->usbTable->insertRow(row);
        ui->usbTable->setItem(row, 0, new QTableWidgetItem(qu.value(0).toString()));
        ui->usbTable->setItem(row, 1, new QTableWidgetItem(qu.value(1).toString()));
    }

    // Живлення
    m_powerTable->setRowCount(0);
    QSqlQuery qp("SELECT timestamp, type, description FROM events WHERE type='Power' ORDER BY timestamp DESC LIMIT 100");
    for (int row = 0; qp.next(); ++row) {
        m_powerTable->insertRow(row);
        m_powerTable->setItem(row, 0, new QTableWidgetItem(qp.value(0).toString()));
        m_powerTable->setItem(row, 1, new QTableWidgetItem(qp.value(1).toString()));
        m_powerTable->setItem(row, 2, new QTableWidgetItem(qp.value(2).toString()));
    }

    updateHeatmap();
}

void MainWindow::updateHeatmap() {
    QMap<int, double> hourlyData;
    for (int i = 0; i < 24; ++i) hourlyData[i] = 0;

    QSqlQuery q(
        "SELECT strftime('%H', timestamp, 'localtime') as hour, COUNT(*) as count "
        "FROM (SELECT timestamp FROM events UNION ALL "
              "SELECT timestamp FROM keylogs UNION ALL "
              "SELECT timestamp FROM clipboard) "
        "WHERE date(timestamp, 'localtime') = date('now', 'localtime') "
        "GROUP BY hour ORDER BY hour ASC"
    );
    while (q.next())
        hourlyData[q.value(0).toInt()] = q.value(1).toDouble();

    m_activityChart->setData(hourlyData);
}
