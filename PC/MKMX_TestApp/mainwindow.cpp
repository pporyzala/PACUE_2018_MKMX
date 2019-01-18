#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "dataviewer.h"
#include "version.h"

#include <QtSerialPort/QSerialPortInfo>
#include <QDesktopWidget>
#include <QMessageBox>
#include <QFileDialog>
#include <QStandardPaths>
#include <QSpinBox>
#include <QTableWidget>
#include <QHeaderView>
#include <QItemDelegate>
#include <QLineEdit>
#include <QRegExpValidator>
#include <QDebug>
#include <QTime>

class HexSpinBox : public QSpinBox {
public:
    HexSpinBox(bool only16Bits, QWidget *parent = nullptr) : QSpinBox(parent), m_only16Bits(only16Bits)
    {
        setPrefix("0x");
        setDisplayIntegerBase(16);
        if (only16Bits)
            setRange(0, 0xFFFF);
        else
            setRange(INT_MIN, INT_MAX);
    }
    unsigned int hexValue() const
    {
        return u(value());
    }
    void setHexValue(unsigned int value)
    {
        setValue(i(value));
    }
protected:
    QString textFromValue(int value) const
    {
        return QString::number(u(value), 16).toUpper();
    }
    int valueFromText(const QString &text) const
    {
        return i(text.toUInt(nullptr, 16));
    }
    QValidator::State validate(QString &input, int &pos) const
    {
        QString copy(input);
        if (copy.startsWith("0x"))
            copy.remove(0, 2);
        pos -= copy.size() - copy.trimmed().size();
        copy = copy.trimmed();
        if (copy.isEmpty())
            return QValidator::Intermediate;
        input = QString("0x") + copy.toUpper();
        bool okay;
        unsigned int val = copy.toUInt(&okay, 16);
        if (!okay || (m_only16Bits && val > 0xFFFF))
            return QValidator::Invalid;
        return QValidator::Acceptable;
    }

private:
    bool m_only16Bits;
    inline unsigned int u(int i) const
    {
        return *reinterpret_cast<unsigned int *>(&i);
    }
    inline int i(unsigned int u) const
    {
        return *reinterpret_cast<int *>(&u);
    }

};

class Delegate : public QItemDelegate
{
public:
    QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem & option,
                      const QModelIndex & index) const
    {
        Q_UNUSED(option);
        Q_UNUSED(index);

        QLineEdit *lineEdit = new QLineEdit(parent);
        // Set validator
        QRegExpValidator *validator = new QRegExpValidator(QRegExp("^[A-Fa-f0-9]+$"), lineEdit);
        lineEdit->setMaxLength(2);
        lineEdit->setValidator(validator);
        return lineEdit;
    }
};

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    globalSettings(new QSettings(QSettings::NativeFormat, QSettings::UserScope,
                       QLatin1String(VER_COMPANYNAME_STR),
                       QLatin1String(VER_PRODUCTNAME_STR), this)),
    refreshStatusTimer(new QTimer(this)),
    errroMsgBox(nullptr),
    lastUsedIncommingDataPortName(""),
    lasyUsedTestFilename("")
{
    ui->setupUi(this);

    setFixedSize(geometry().width(), geometry().height());
    setWindowTitle(trUtf8("Mikomax Test App, v %1").arg(VERSION_STR));

    connect(ui->clearLogBtn, SIGNAL(clicked(bool)), this, SLOT(clearLogSlot()));
    connect(ui->saveToFileBtn, SIGNAL(clicked(bool)), this, SLOT(saveLogToFile()));

    connect(ui->incommingDataRefreshBtn, SIGNAL(clicked(bool)), this, SLOT(refreshBtnSlot()));
    connect(ui->incommingDataConnectBtn, SIGNAL(clicked(bool)), this, SLOT(incommingDataInterfaceConnectBtnSlot()));

    connect(ui->alwaysOnTop, SIGNAL(clicked(bool)), this, SLOT(alwaysOnTopToggledSlot()));

    connect(ui->closeAppBtn, SIGNAL(clicked(bool)), this, SLOT(close()));

    refreshStatusSlot();
    connect(refreshStatusTimer, SIGNAL(timeout()), this, SLOT(refreshStatusSlot()));
    refreshStatusTimer->start(1000);

    connect(&engine, SIGNAL(incommingDataInterfaceBecomesOnline(QString)), this, SLOT(incommingDataInterfaceOpenedSlot(QString)));
    connect(&engine, SIGNAL(incommingDataInterfaceTriggersError(QString)), this, SLOT(incommingDataInterfaceErrorSlot(QString)));
    connect(&engine, SIGNAL(incommingDataInterfaceBecomesOffline()), this, SLOT(incommingDataInterfaceClosedSlot()));

    connect(&engine, SIGNAL(newDebugVariableText(QString)), this, SLOT(newDebugVariableTextSlot(QString)));

    ui->dataReadoutGB->setEnabled(false);
    ui->dataWriteGB->setEnabled(false);

    hsbAddr = new HexSpinBox(true, this);
    hsbAddr->setFixedWidth(80);
    hsbCmd = new HexSpinBox(true, this);
    hsbCmd->setFixedWidth(80);
    hsbPayloadLen = new HexSpinBox(true, this);
    hsbPayloadLen->setFixedWidth(80);
    connect(hsbPayloadLen, SIGNAL(valueChanged(int)), this, SLOT(payloadLenChanged(int)));
//    hsbPayloadLen->setEnabled(false);

    twPayload = new QTableWidget(0, 1);
    twPayload->setFixedWidth(80);
    twPayload->setItemDelegate(new Delegate());
    twPayload->setRowCount(0);
    twPayload->setColumnCount(1);
    twPayload->horizontalHeader()->hide();

    ui->frameDataLayout->addRow(new QLabel("Adres:"), hsbAddr);
    ui->frameDataLayout->addRow(new QLabel("Komenda:"), hsbCmd);
    ui->frameDataLayout->addRow(new QLabel("Dłudość pola danych:"), hsbPayloadLen);
    ui->frameDataLayout->addRow(new QLabel("Dane:"), twPayload);
//static_cast<QFormLayout *>(ui->dataWriteGB->layout())->r

    connect(ui->sendBtn, SIGNAL(clicked()), this, SLOT(sendBtnSlot()));

    readSettings();

    refreshPortsList(lastUsedIncommingDataPortName);
}

void MainWindow::payloadLenChanged(int rowCnt) {
    twPayload->setRowCount(rowCnt);

    for (int i = 0; i < rowCnt; i++) {
        QTableWidgetItem* item = twPayload->item(i, 0);
        if (item != nullptr) {
            if (item->text().isEmpty())
                item->setText("00");
        }
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::clearLogSlot(void) {
    ui->debugWindow->clear();
}

void MainWindow::saveLogToFile(void) {
    QString fileName;

    if (!QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).empty()) {
        fileName = QFileDialog::getSaveFileName(this, tr("Zapisz plik tesktowy..."),
                                                QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).at(0),
                                                tr("Pliki tekstowe (*.txt)"));
    } else {
        fileName = QFileDialog::getSaveFileName(this, tr("Zapisz plik tesktowy..."),
                                                "",
                                                tr("Pliki tekstowe (*.txt)"));
    }

    if (!fileName.isEmpty()) {
        qDebug() << "fileName: " << fileName;
        QFile file(fileName);

        if (file.open(QIODevice::ReadWrite)) {
            QTextStream stream(&file);
            stream << ui->debugWindow->toPlainText();
            file.flush();
            file.close();
        }
        else {
            QMessageBox::critical(this, tr("Błąd!!!"), tr("Wystąpił błąd podczas próby zapisu pliku..."));
            return;
        }
    }
}

void MainWindow::newDebugVariableDataSlot(QStringList strLst) {
    for (int i = 0; i < strLst.length(); i++)
        ui->debugWindow->appendPlainText(strLst.at(i));

    ui->debugWindow->appendPlainText("");
}

void MainWindow::newDebugVariableTextSlot(QString str) {
    if (ui->timestampMessages->isChecked()) {
        ui->debugWindow->appendPlainText(QTime::currentTime().toString("[ HH:mm:ss:zzz ] -> ") + str);
    } else {
        ui->debugWindow->appendPlainText(str);
    }
}

void MainWindow::refreshStatusSlot(void) {
}

void MainWindow::refreshBtnSlot(void) {
    refreshPortsList();
}

void MainWindow::alwaysOnTopToggledSlot(void) {
    if (ui->alwaysOnTop->isChecked()) {
        setWindowFlags((windowFlags() & ~Qt::WindowStaysOnBottomHint) | Qt::WindowStaysOnTopHint);
    } else {
        setWindowFlags((windowFlags() & ~Qt::WindowStaysOnTopHint) | Qt::WindowStaysOnBottomHint);
    }

    show();
    raise();
}

void MainWindow::sendBtnSlot(void) {
    qDebug() << "send btn slot" << hsbPayloadLen->value();

    QByteArray baData;
    for (int i = 0; i < hsbPayloadLen->value(); i++) {
        bool bOk;
        uint8_t u8Val;
        QTableWidgetItem* item = twPayload->item(i, 0);
        if (item != nullptr) {
            qDebug() << "nullptr";
            u8Val = item->text().toInt(&bOk, 16);
            if (!bOk)
                u8Val = 0;
        } else {
            u8Val = 0;
        }
        baData.append(u8Val);
    }

    engine.txData(hsbAddr->value(), hsbCmd->value(), baData);
}

void MainWindow::resetCalibrationBtnSlot(void) {

}

void MainWindow::incommingDataInterfaceConnectBtnSlot(void) {
    if (!engine.isIncommingDataInterfaceConnected()) {
        engine.openIncommingDataInterface(ui->incommingDataPortBox->currentText(), 10);
    } else {
        engine.closeIncommingDataInterface();
    }
}

void MainWindow::refreshPortsList(const QString &lastIncommingPort) {
    QString oldIncommingDataPort = ui->incommingDataPortBox->currentText();

    ui->incommingDataPortBox->clear();
    QMap<int, QString> portsList;
    portsList.clear();
    QList<QSerialPortInfo> allPorts = QSerialPortInfo::availablePorts();

    foreach (const QSerialPortInfo &portName, allPorts) {
        QString temp = portName.portName();
        portsList.insert(temp.remove(0, 3).toInt(), portName.portName());
    }

    ui->incommingDataPortBox->addItems(portsList.values());

    if (lastIncommingPort.isEmpty()) {
        //set previously selected port:
        if (oldIncommingDataPort != QString("")) {
            for (int i = 0; i < ui->incommingDataPortBox->count(); i++) {
                if (oldIncommingDataPort == ui->incommingDataPortBox->itemText(i)) {
                    ui->incommingDataPortBox->setCurrentIndex(i);
                    break;
                }
            }
        }
    } else {
        //set port used in last session:
        for (int i = 0; i < ui->incommingDataPortBox->count(); i++) {
            if (lastIncommingPort == ui->incommingDataPortBox->itemText(i)) {
                ui->incommingDataPortBox->setCurrentIndex(i);
                break;
            }
        }
    }
}

void MainWindow::incommingDataInterfaceOpenedSlot(const QString &pn) {
    refreshStatusSlot();

    //adjust connect button:
    ui->incommingDataConnectBtn->setIcon(QIcon(":/imgs/connected.png"));
    ui->incommingDataConnectBtn->setText(trUtf8("Zamknij"));

    //disable controls:
    ui->dataReadoutGB->setEnabled(true);
    ui->dataWriteGB->setEnabled(true);

    ui->incommingDataRefreshBtn->setEnabled(false);
    ui->incommingDataPortBox->setEnabled(false);

    //remember this selection for future sessions:
    lastUsedIncommingDataPortName = pn;
}

void MainWindow::incommingDataInterfaceErrorSlot(const QString &error) {
    if (errroMsgBox == nullptr) {
        errroMsgBox = new QMessageBox(QMessageBox::Critical,
                                      trUtf8("Błąd!"),
                                      trUtf8("Podczas komunikacji wystąpił błąd: "
                                             "<br>"
                                             "<br><b>%1</b>"
                                             "<br>"
                                             "<br>Nastąpi automatyczne zakończenie komunikacji z urządzeniem...").arg(error),
                                      QMessageBox::Close, this);

        errroMsgBox->exec();

        delete errroMsgBox;
        errroMsgBox = nullptr;
    }

    engine.closeIncommingDataInterface();
}

void MainWindow::incommingDataInterfaceClosedSlot(void) {
    refreshStatusSlot();

    //adjust connect button:
    ui->incommingDataConnectBtn->setIcon(QIcon(":/imgs/disconnected.png"));
    ui->incommingDataConnectBtn->setText(trUtf8("Otwórz"));

    //enable controls:
    ui->dataReadoutGB->setEnabled(false);
    ui->dataWriteGB->setEnabled(false);

    ui->incommingDataRefreshBtn->setEnabled(true);
    ui->incommingDataPortBox->setEnabled(true);
}

void MainWindow::closeEvent(QCloseEvent *event) {
    Q_UNUSED(event);

    engine.closeIncommingDataInterface();

    disconnect(&engine, nullptr, nullptr, nullptr);

    //write settings:
    writeSettings();

    //now quit application
    qApp->quit();
}

void MainWindow::readSettings() {
    const QVariant position = globalSettings->value("windowPosition");
    if ( position.isValid() ) {
        move(position.toPoint());
    }

    if (globalSettings->value("alwaysOnTop").toBool()) {
        setWindowFlags((windowFlags() & ~Qt::WindowStaysOnBottomHint) | Qt::WindowStaysOnTopHint);
        ui->alwaysOnTop->setChecked(true);
    } else {
        setWindowFlags((windowFlags() & ~Qt::WindowStaysOnTopHint) | Qt::WindowStaysOnBottomHint);
        ui->alwaysOnTop->setChecked(false);
    }

    show();
    raise();

    lastUsedIncommingDataPortName = globalSettings->value("lastUsedIncommingDataPortName", "").toString();

    engine.readSettings(globalSettings);
}

void MainWindow::writeSettings() {
    //save window position on the screen:
    globalSettings->setValue("windowPosition", pos());

    globalSettings->setValue("alwaysOnTop", ui->alwaysOnTop->isChecked());

    globalSettings->setValue("lastUsedIncommingDataPortName", lastUsedIncommingDataPortName);

    engine.writeSettings(globalSettings);
}
