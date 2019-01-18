#include "interface.h"

#include "utils/crctools.h"

#include <QtSerialPort/QtSerialPort>

#define DEBUG_INTERFACE_THREAD

#ifdef DEBUG_INTERFACE_THREAD
    #include <QDebug>
    #include "utils/debugtools.h"
#endif

cInterface::cInterface(QObject *parent) :
    QThread(parent),
    m_closeRequest(false),
    m_online(false)
{
    m_interfaceID = "strThreadID";

    u8FrameCnt = 0;

    InitializeRingBuffer(&m_sDataTxBuffer, u8DataTxBuffer, TX_BUFFER_LENGTH, 0);
}

void cInterface::start(const QString &qsPortName, int iWaitTimeout) {
    qDebug() << "open serial: " << qsPortName << iWaitTimeout;
    m_serialPortName = qsPortName;
    m_waitTimeout = iWaitTimeout;

    m_online = false;
    m_closeRequest = false;

    QThread::start();
}

void cInterface::stop(void) {
    qDebug() << "close serial";

    m_mutex.lock();
    m_closeRequest = true;
    m_mutex.unlock();

    wait();
}

bool cInterface::isOnline(void) {
    QMutexLocker locker(&m_mutex);
    return m_online;
}

bool cInterface::txData(uint8_t u8Addr, uint8_t u8Cmd, QByteArray baData) {
    m_mutex.lock();

    if (m_online) {
#ifdef DEBUG_INTERFACE_THREAD
        QByteArray payload = QByteArray(baData);
#endif
        uint8_t u8LenInBytes = baData.length();

        //prepend length protocol field:
        baData.prepend((char)u8LenInBytes);

        //prepend frame count field:
        baData.prepend((char)u8Cmd);

        //prepend command protocol field:
        baData.prepend((char)u8Addr);

        //prepend FRAME HEADER:
        baData.prepend((char)0xA5);
        baData.prepend((char)0x5A);

        uint8_t u8CrcResult = computeCRC((uint8_t *)(baData.data() + 2), baData.length() - 2);

        baData.append((char)u8CrcResult);

#ifdef DEBUG_INTERFACE_THREAD
        qDebug() << "IFACE TX_DATA (" << m_interfaceID << "):";
        qDebug() << "  data len:    " << baData.length();
        qDebug() << "  data raw:    " << qbaToString(baData);
        qDebug() << "  payload len: " << u8LenInBytes;
        qDebug() << "  payload raw: " << qbaToString(payload);
        qDebug() << "  free:        " << NoOfFreeBytes(&m_sDataTxBuffer);
        qDebug() << "  crc:         " << u8ToString(u8CrcResult);
#endif

        if (NoOfFreeBytes(&m_sDataTxBuffer) >= baData.length()) {
            for(int i = 0; i < baData.length(); i++) {
                PushByte(&m_sDataTxBuffer, baData.at(i));
            }

            m_mutex.unlock();

            return true;
        } else {
            qDebug() << "there is not enough free space in tx buffer for: " << m_serialPortName;
        }
    } else {
        qDebug() << "interface is offline - can not tx data...";
    }

    m_mutex.unlock();

    return false;
}

QString cInterface::errorToString(QSerialPort::SerialPortError errCode) {
    switch (errCode) {

        case QSerialPort::DeviceNotFoundError:
            return "Attempting to open an non-existing device";
        case QSerialPort::PermissionError:
            return "Attempting to open device which was already opened or current user does not have enough permission";
        case QSerialPort::OpenError:
            return "Attempting to open device which was already opened in this object";
        case QSerialPort::ParityError:
            return "Parity error detected by the hardware while reading data";
        case QSerialPort::FramingError:
            return "Framing error detected by the hardware while reading data";
        case QSerialPort::BreakConditionError:
            return "Break condition detected by the hardware on the input line";
        case QSerialPort::WriteError:
            return "An I/O error occurred while writing the data";
        case QSerialPort::ReadError:
            return "An I/O error occurred while reading the data";
        case QSerialPort::ResourceError:
            return "Resource becomes unavailable (the device was unexpectedly removed from the system?)";
        case QSerialPort::UnsupportedOperationError:
            return "The requested device operation is not supported or prohibited by the running operating system";
        case QSerialPort::UnknownError:
            return "An unidentified error occurred";
        case QSerialPort::TimeoutError:
            return "A timeout error occurred";
        case QSerialPort::NotOpenError:
            return "Operation can only be successfully performed if the device is open";
        default:
        case QSerialPort::NoError:
            return "No error occurred";
    }
}

void cInterface::run(void) {
    bool currentPortNameChanged = false;

    m_mutex.lock();
    QString currentPortName;
    if (currentPortName != m_serialPortName) {
        currentPortName = m_serialPortName;
        currentPortNameChanged = true;
    }

    int currentWaitTimeout = m_waitTimeout;
    m_mutex.unlock();

    QSerialPort serial;

    while (!m_closeRequest) {
        if (currentPortNameChanged) {
            qDebug() << "currentPortNameChanged";

            serial.close();
            serial.setPortName(currentPortName);
            serial.setBaudRate(4800);

            if (!serial.open(QIODevice::ReadWrite)) {
                emit error(errorToString(serial.error())); //tr("Can't open %1, error code %2").arg(m_serialPortName).arg(serial.error()));
                qDebug() << tr("Can't open %1, error code %2").arg(m_serialPortName).arg(serial.error());

                break;
            }

            m_online = true;

            emit connected();
        }

        if (serial.waitForReadyRead(currentWaitTimeout)) {
            // read request
            QByteArray rxedData = serial.readAll();

            if (!rxedData.isEmpty()) {
                emit newData(rxedData);
            }
        } else {
            //emit timeout(tr("Wait read request timeout %1").arg(QTime::currentTime().toString()));
            //qDebug() << tr("Wait read request timeout %1").arg(QTime::currentTime().toString());
        }

        m_mutex.lock();

        uint8_t u8UsedBytes = NoOfUsedBytes(&m_sDataTxBuffer);
        if (u8UsedBytes != 0) {
            uint8_t *u8DataBuffer = new uint8_t [u8UsedBytes];

            uint8_t u8NoOfBytesToSend = PopData(&m_sDataTxBuffer, u8DataBuffer, u8UsedBytes);

            serial.write((const char *)u8DataBuffer, u8NoOfBytesToSend);

            if (!serial.waitForBytesWritten(currentWaitTimeout)) {
                emit timeout(tr("Wait write response timeout %1").arg(QTime::currentTime().toString()));
                qDebug() << tr("Wait write response timeout %1").arg(QTime::currentTime().toString());
            }

            delete [] u8DataBuffer;
        }

        if (currentPortName != m_serialPortName) {
            currentPortName = m_serialPortName;
            currentPortNameChanged = true;
        } else {
            currentPortNameChanged = false;
        }
        currentWaitTimeout = m_waitTimeout;
        m_mutex.unlock();
    }

    emit disconnected();

    m_online = false;
}
