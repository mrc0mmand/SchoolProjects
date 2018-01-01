#ifndef NETWORKINFO_H
#define NETWORKINFO_H

#include <QObject>
#include <QQmlListProperty>
#include <QNetworkConfiguration>
#include <QNetworkConfigurationManager>
#include <QtAndroidExtras/QAndroidJniObject>

#include "datastoremanager.h"
#include "networkcell.h"

class NetworkLocation : public QObject
{
    Q_OBJECT
    Q_PROPERTY(float longitude READ getLon NOTIFY locationChanged)
    Q_PROPERTY(float latitude READ getLat NOTIFY locationChanged)
    Q_PROPERTY(int cellcount READ getCellCount NOTIFY locationChanged)
    Q_PROPERTY(int cellsize READ getCellSize CONSTANT)
    Q_PROPERTY(QQmlListProperty<NetworkCell> cells READ cells NOTIFY locationChanged)

public:
    typedef QList<NetworkCell*> NetworkCells;

    explicit NetworkLocation(QObject *parent = nullptr);
    NetworkCells getCellids();
    Q_INVOKABLE void updateLocation();
    QQmlListProperty<NetworkCell> cells();

    QString getLasterror() { return m_lasterror; }
    float getLon() { return m_lon; }
    float getLat() { return m_lat; }
    int getCellCount() { return m_lastcells.length(); }
    int getCellSize() { return NetworkCell::size; }
    NetworkCells getLastCells() { return m_lastcells; }

private:
    QString m_lasterror;
    DataStoreManager m_datamgr;
    NetworkCells m_lastcells;

    float m_lon;
    float m_lat;

signals:
    void locationChanged();
public slots:
};

#endif // NETWORKINFO_H
