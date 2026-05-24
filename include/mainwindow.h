#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>
#include "activitychart.h"
#include "monitors/appmonitor.h"
#include "monitors/inputmonitor.h"
#include "monitors/usbmonitor.h"
#include "monitors/clipboardmonitor.h"
#include "monitors/powermonitor.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void refreshLogs();
    void updateHeatmap();

private:
    void startMonitoring();

    bool nativeEvent(const QByteArray &eventType, void *message, qintptr *result) override;

    Ui::MainWindow *ui;
    ActivityChart  *m_activityChart;

    // Вкладка живлення — створюється програмно, не через .ui
    QTableWidget   *m_powerTable = nullptr;

    AppMonitor     *m_appMon;
    InputMonitor   *m_inputMon;
    UsbMonitor     *m_usbMon;
    ClipboardMonitor *m_clipMon;
    PowerMonitor   *m_powerMon;
};

#endif // MAINWINDOW_H
