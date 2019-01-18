#ifndef ENGINE_H
#define ENGINE_H

#include <QObject>
#include <QTimer>
#include <QSettings>

#include <stdint.h>

class cInterface;

#define MAX_PAYLOAD_LENGTH  128

class cEngine : public QObject
{
    Q_OBJECT
private:
    typedef struct {
        uint16_t u16Start;
        uint8_t u8DestAddr;
        uint8_t u8Cmd;
        uint8_t u8Len;
        uint8_t u8Payload[MAX_PAYLOAD_LENGTH];
        uint8_t u8CRC;

        uint8_t u8RawData[MAX_PAYLOAD_LENGTH + 3];
    } sRxFrame_t;

    typedef enum {
        eStart0x5A = 0,
        eStart0xA5,
        eDestAddr,
        eCommand,
        ePayloadLen,
        ePayload,
        eCRC
    } eRxState_t;

public:
    cEngine(QObject *parent = nullptr);

    bool txData(uint8_t u8Addr, uint8_t u8Cmd, QByteArray baData);

    void readSettings(QSettings *settings);
    void writeSettings(QSettings *settings);

    void openIncommingDataInterface(const QString &qsPortName, int iWaitTimeout);
    void closeIncommingDataInterface(void);

    bool isIncommingDataInterfaceConnected(void);

private:
    bool checkFrameCRC(sRxFrame_t *frame);
    void parseFrame(sRxFrame_t *frame);

signals:
    void incommingDataInterfaceBecomesOnline(QString pn);
    void incommingDataInterfaceTriggersError(QString error);
    void incommingDataInterfaceBecomesOffline(void);

    void newDebugVariableText(QString str);

private slots:
    void incommingDataInterfaceConnected(void);
    void incommingDataInterfaceError(const QString &qsError);
    void incommingDataInterfaceDisconnected(void);
    void incommingInterfaceDataRxed(const QByteArray baData);

private:
    eRxState_t eRxState;

    cInterface* dataInterface;

    void incommingDataInterfaceResetInternalState(void);
};

#endif // ENGINE_H
