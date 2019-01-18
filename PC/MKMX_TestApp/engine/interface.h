#ifndef INTERFACE_H
#define INTERFACE_H

#include <QThread>
#include <QMutex>
#include <QSerialPort>

#include "utils/ringbuffer.h"

#define TX_BUFFER_LENGTH    1024

class cInterface: public QThread
{
    Q_OBJECT

    void run(void);

public:
    cInterface(QObject *paretn);

    bool isOnline(void);
    void start(const QString &qsPortName, int iWaitTimeout);
    void stop(void);

    QString serialPortName(void) { return m_serialPortName; }

    bool txData(uint8_t u8Addr, uint8_t u8Cmd, QByteArray baData);

signals:
    void timeout(const QString &s);

    void newData(QByteArray baData);

    void connected(void);
    void error(const QString &s);
    void disconnected(void);

private:
    QMutex m_mutex;
    bool m_closeRequest;
    bool m_online;

    sRingBuffer_t m_sDataTxBuffer;
    uint8_t u8DataTxBuffer[TX_BUFFER_LENGTH];

    QString m_interfaceID;
    QString m_serialPortName;
    int m_waitTimeout;

    QString errorToString(QSerialPort::SerialPortError errCode);

    uint8_t u8FrameCnt;
};

#endif // INTERFACE_H
