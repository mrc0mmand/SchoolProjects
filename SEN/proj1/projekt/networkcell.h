#ifndef NETWORKCELL_H
#define NETWORKCELL_H

#include <QObject>
#include <QStringList>

class NetworkCell : public QObject
{
    Q_OBJECT
    Q_PROPERTY(qint32 mcc READ getMcc CONSTANT)
    Q_PROPERTY(qint32 mnc READ getMnc CONSTANT)
    Q_PROPERTY(qint32 lac READ getLac CONSTANT)
    Q_PROPERTY(qint32 cid READ getCid CONSTANT)
    Q_PROPERTY(float lon READ getLon CONSTANT)
    Q_PROPERTY(float lat READ getLat CONSTANT)
    Q_PROPERTY(int ss READ getSs CONSTANT)

public:
    qint32 mcc; // Mobile Countr Code
    qint32 mnc; // Mobile Network Code
    qint32 lac; // Local Area Code
    qint32 cid; // Cell ID
    float lon; // Longitude
    float lat; // Latitude
    int ss;  // Signal strength

    static const int size = 7;

    explicit NetworkCell(QObject *parent = nullptr) : QObject(parent) {}

    qint32 getMcc() { return mcc; }
    qint32 getMnc() { return mnc; }
    qint32 getLac() { return lac; }
    qint32 getCid() { return cid; }
    float getLon() { return lon; }
    float getLat() { return lat; }
    int getSs() { return ss; }

    QStringList toStringList() {
        QStringList res;
        res << QString::number(mcc) << QString::number(mnc)
            << QString::number(lac) << QString::number(cid)
            << QString::number(lon) << QString::number(lat)
            << QString::number(ss);
        return res;
    }
};

#endif // NETWORKCELL_H
