#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSettings>

#include "engine/engine.h"

namespace Ui {
class MainWindow;
}

class HexSpinBox;
class QTableWidget;
class QMessageBox;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *evt);

private:
    void refreshPortsList(const QString &lastIncommingPort = "");

    void writeSettings();
    void readSettings();

private slots:
    void refreshBtnSlot(void);

    void incommingDataInterfaceConnectBtnSlot(void);

    void alwaysOnTopToggledSlot(void);
    void clearLogSlot(void);
    void saveLogToFile(void);

    void incommingDataInterfaceOpenedSlot(const QString &pn);
    void incommingDataInterfaceErrorSlot(const QString &error);
    void incommingDataInterfaceClosedSlot(void);

    void newDebugVariableDataSlot(QStringList strLst);
    void newDebugVariableTextSlot(QString str);

    void payloadLenChanged(int i);

    void refreshStatusSlot(void);

    void sendBtnSlot(void);
    void resetCalibrationBtnSlot(void);

private:
    Ui::MainWindow *ui;
    QSettings *globalSettings;
    QTimer *refreshStatusTimer;

    HexSpinBox* hsbAddr;
    HexSpinBox* hsbCmd;
    HexSpinBox* hsbPayloadLen;
    QTableWidget* twPayload;

    QMessageBox *errroMsgBox;

    QString lastUsedIncommingDataPortName;
    QString lasyUsedTestFilename;

    cEngine engine;
};

#endif // MAINWINDOW_H
