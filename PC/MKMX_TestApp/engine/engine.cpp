#include "engine.h"

#include <QSettings>
#include <QMessageBox>

#include "utils/crctools.h"

#include "interface.h"

#include "version.h"

#define DEBUG_ENGINE

//#ifdef DEBUG_ENGINE
    #include <QDebug>
    #include "utils/debugtools.h"
//#endif

#define BINARY_DEBUG_DATA_COMMAND	'g'
#define TEXT_DEBUG_DATA_COMMAND		't'

#define RXED_DATA_COMMAND			'd'
#define TXED_DATA_COMMAND			'e'

#define RESET_STORED_STATUS_COMMAND	'x'
#define CALIBRATION_DATA_COMMAND	'k'

#define READ_DATA_SAMPLES_AT_ONCE       1
#define READ_DATA_BASE_TIMEOUT_PERIOD   15

cEngine::cEngine(QObject *parent) :
    QObject(parent),
    eRxState(eStart0x5A),
    dataInterface(nullptr)
{
    incommingDataInterfaceResetInternalState();
}

void cEngine::readSettings(QSettings *settings) {
    Q_UNUSED(settings);
}

void cEngine::writeSettings(QSettings *settings) {
    Q_UNUSED(settings);
}

void cEngine::incommingDataInterfaceConnected(void) {
    qDebug() << "ENGINE: incommingDataInterfaceConnected()";

    incommingDataInterfaceResetInternalState();

    if ((dataInterface != nullptr)) {
        if (dataInterface->isOnline())
            emit incommingDataInterfaceBecomesOnline(dataInterface->serialPortName());
    }
}

void cEngine::incommingDataInterfaceError(const QString &qsError) {
    emit incommingDataInterfaceTriggersError(qsError);
}


void cEngine::incommingDataInterfaceDisconnected(void) {
    qDebug() << "ENGINE: incommingDataInterfaceDisconnected()";

    emit incommingDataInterfaceBecomesOffline();

    closeIncommingDataInterface();
}

bool cEngine::checkFrameCRC(sRxFrame_t *frame) {

    uint8_t crc = 0, i;
    for (i = 0; i < frame->u8Len + 3; i++)
        crc = _crc8_ccitt_update(crc, frame->u8RawData[i]);

#ifdef DEBUG_ENGINE
        qDebug() << "ENGINE checkFrameCRC => computed for rxed data: " << u8ToString(crc) << ", embedded within the frame: " << u8ToString(frame->u8CRC);
#endif

    return (crc == frame->u8CRC);
}

bool cEngine::txData(uint8_t u8Addr, uint8_t u8Cmd, QByteArray baData) {
    return dataInterface->txData(u8Addr, u8Cmd, baData);
}

void cEngine::incommingDataInterfaceResetInternalState(void) {
    eRxState = eStart0x5A;
}

void cEngine::parseFrame(sRxFrame_t *frame) {
    if (frame->u8Cmd == TEXT_DEBUG_DATA_COMMAND) {
        char cText[512];
        memcpy(cText, frame->u8Payload, frame->u8Len);
        cText[frame->u8Len] = '\0';

        emit newDebugVariableText(QString(cText));
    } else {
        emit incommingDataInterfaceError(trUtf8("Odebrano ramkę z kodem ID o nieoczekiwanej wartości ?! Wartość kodu ID: %1 (hex: %2)... To nie powinno się zdarzyć !")
                                         .arg(QString::number(frame->u8Cmd),
                                              QString::number(frame->u8Cmd, 16)));

//        qDebug() << "unknown command: " << u8ToString(frame->u8Cmd) << ", length: " << frame->u16Len;
    }
}

void cEngine::incommingInterfaceDataRxed(QByteArray baData) {
    static uint8_t u8RxPayloadDataCnt = 0, u8PayloadLen = 0;
    static uint16_t u16RawDataCnt = 0;
    static sRxFrame_t rxFrame;

    while (!baData.isEmpty()) {
        uint8_t u8Data = baData.at(0);
        baData.remove(0, 1);
#ifdef DEBUG_ENGINE
        qDebug() << "eRxState " << eRxState << "u8Data" << u8ToString(u8Data);
#endif
        switch (eRxState) {
        case eStart0x5A:
            if (u8Data == 0x5A) {
                eRxState = eStart0xA5;
                u16RawDataCnt = 0;
                u8RxPayloadDataCnt = 0;
            }
            break;

        case eStart0xA5:
            if (u8Data == 0xA5) {
                rxFrame.u16Start = 0x5AA5;
                eRxState = eDestAddr;
            } else {
                eRxState = eStart0x5A;
            }
            break;

        case eDestAddr:
            rxFrame.u8DestAddr = u8Data;
            rxFrame.u8RawData[u16RawDataCnt++] = u8Data;
            eRxState = eCommand;
            break;

        case eCommand:
            rxFrame.u8Cmd = u8Data;
            rxFrame.u8RawData[u16RawDataCnt++] = u8Data;
            eRxState = ePayloadLen;
            break;

        case ePayloadLen:
            rxFrame.u8Len = u8Data;
            rxFrame.u8RawData[u16RawDataCnt++] = u8Data;

            if (rxFrame.u8Len <= MAX_PAYLOAD_LENGTH) {
                if (rxFrame.u8Len != 0) {
                    u8PayloadLen = rxFrame.u8Len;

                    eRxState = ePayload;
                } else {
                    eRxState = eCRC;
                }
            } else {
                qDebug() << "MAX PAYLOAD ERROR!!!";
                //ERROR!!!
                eRxState = eStart0x5A;
            }
            break;

        case ePayload:
            rxFrame.u8Payload[u8RxPayloadDataCnt++] = u8Data;
            rxFrame.u8RawData[u16RawDataCnt++] = u8Data;

            if (--u8PayloadLen == 0) {
                rxFrame.u8CRC = 0;
                eRxState = eCRC;
            }
            break;

        case eCRC:
            rxFrame.u8CRC = u8Data;

#ifdef DEBUG_ENGINE
            qDebug() << "FULL FRAME";
            qDebug() << "  cmd:     " << u8ToString(rxFrame.u8Cmd);
            qDebug() << "  adr:     " << u8ToString(rxFrame.u8DestAddr);
            qDebug() << "  len:     " << u8ToString(rxFrame.u8Len) << "(" << rxFrame.u8Len << "bajtów )";
            qDebug() << "  payload: " << pu8ToString(rxFrame.u8Payload, rxFrame.u8Len);
            qDebug() << "  crc:     " << u8ToString(rxFrame.u8CRC);
#endif
            if (checkFrameCRC(&rxFrame)) {
               parseFrame(&rxFrame);
            } else {
                //ERROR!
                qDebug() << "CRC ERROR!!!";
            }

            eRxState = eStart0x5A;
            break;
        }
    }
}

bool cEngine::isIncommingDataInterfaceConnected(void) {
    if (dataInterface != nullptr)
        return dataInterface->isOnline();
    else
        return false;
}

void cEngine::openIncommingDataInterface(const QString &qsPortName, int iWaitTimeout) {
    dataInterface = new cInterface(this);

    connect(dataInterface, SIGNAL(connected()), this, SLOT(incommingDataInterfaceConnected()));
    connect(dataInterface, SIGNAL(error(QString)), this, SLOT(incommingDataInterfaceError(QString)));
    connect(dataInterface, SIGNAL(disconnected()), this, SLOT(incommingDataInterfaceDisconnected()));
    connect(dataInterface, SIGNAL(newData(QByteArray)), this, SLOT(incommingInterfaceDataRxed(QByteArray)), Qt::DirectConnection);

    dataInterface->start(qsPortName, iWaitTimeout);
}

void cEngine::closeIncommingDataInterface(void) {
    bool bEmitOfflineSignal = false;

    if ((dataInterface != nullptr))
        bEmitOfflineSignal = true;

    if (dataInterface != nullptr) {
        // disconnect everything connected to an object's signals
        disconnect(dataInterface, nullptr, nullptr, nullptr);

        dataInterface->stop();

        if (dataInterface != nullptr) {
            delete dataInterface;
            dataInterface = nullptr;
        }
    }

    if (bEmitOfflineSignal)
        emit incommingDataInterfaceBecomesOffline();
}
