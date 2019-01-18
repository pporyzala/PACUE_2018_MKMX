#ifndef DEBUGTOOLS_H
#define DEBUGTOOLS_H

#include <QtCore/QtGlobal>
#include <QObject>
#include <QDateTime>
#include <QMap>
#include <QString>

#include <QDebug>

#include <stdint.h>

QString u8ToString(uint8_t u8Data);
QString u16ToString(uint16_t u16Data);
QString u32ToString(uint32_t u32Data);
QString u64ToString(uint64_t u64Data);

QString qbaToString(const QByteArray &data);
QString pu8ToString(uint8_t *data, uint16_t len);
QString pu16ToString(uint16_t *data, uint16_t len);
QString pi16ToString(int16_t *data, uint16_t len);
QString pfToString(float *data, uint16_t len);

#endif // DEBUGTOOLS_H

