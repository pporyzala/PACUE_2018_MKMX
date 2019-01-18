#include "debugtools.h"

QString u8ToString(uint8_t u8Data) {
    return ((u8Data > 15) ? QString::number(u8Data, 16).toUpper() : QString::number(u8Data, 16).toUpper().prepend("0"));
}

QString u16ToString(uint16_t u16Data) {
    uint8_t u8_h = (u16Data >> 8) & 0x00FF;
    uint8_t u8_l = (u16Data >> 0) & 0x00FF;

    return (u8ToString(u8_h) + u8ToString(u8_l));
}

QString u32ToString(uint32_t u32Data) {
    uint8_t u8_hh = (u32Data >> 24) & 0x00FF;
    uint8_t u8_hl = (u32Data >> 16) & 0x00FF;
    uint8_t u8_lh = (u32Data >> 8) & 0x00FF;
    uint8_t u8_ll = (u32Data >> 0) & 0x00FF;

    return (u8ToString(u8_hh) + u8ToString(u8_hl) + u8ToString(u8_lh) + u8ToString(u8_ll));
}

QString u64ToString(uint64_t u64Data) {
    uint8_t u32_h = (u64Data >> 32) & 0xFFFFFFFF;
    uint8_t u32_l = (u64Data >>  0) & 0xFFFFFFFF;

    return (u32ToString(u32_h) + u32ToString(u32_l));
}

QString qbaToString(const QByteArray &data) {
    QString strFrm; strFrm.clear();

//    strFrm.append(" ");
    for (int i = 0; i < data.length(); i++) {
        strFrm.append(u8ToString((uint8_t)data.at(i)));

        if (i < data.length() - 1)
            strFrm.append(" ");
    }

    return strFrm;
}

QString pu8ToString(uint8_t *data, uint16_t len) {
    QString strFrm; strFrm.clear();

    for (int i = 0; i < len; i++) {
        strFrm.append(u8ToString(*(data + i)));

        if (i < len - 1)
            strFrm.append(" ");
    }

    return strFrm;
}

QString pfToString(float *data, uint16_t len) {
    QString strFrm; strFrm.clear();

    for (int i = 0; i < len; i++) {
        strFrm.append(QString::number(data[i], 'f'));

        if (i < len - 1)
            strFrm.append(", ");
    }

    return strFrm;
}

QString pu16ToString(uint16_t *data, uint16_t len) {
    QString strFrm; strFrm.clear();

    for (int i = 0; i < len; i++) {
        strFrm.append(u16ToString(*(data + i)));

        if (i < len - 1)
            strFrm.append(" ");
    }

    return strFrm;
}

QString pi16ToString(int16_t *data, uint16_t len) {
    QString strFrm; strFrm.clear();

    for (int i = 0; i < len; i++) {
        strFrm.append(QString::number(*(data + i)));

        if (i < len - 1)
            strFrm.append(" ");
    }

    return strFrm;
}
