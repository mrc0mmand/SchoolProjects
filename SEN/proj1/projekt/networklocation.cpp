#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QtAndroid>
#include <QDebug>

#include "networklocation.h"

NetworkLocation::NetworkLocation(QObject *parent) : QObject(parent)
{
    qDebug("NetworkInfo()");
}

NetworkLocation::NetworkCells NetworkLocation::getCellids()
{
    QAndroidJniObject res = QAndroidJniObject::callStaticObjectMethod<jstring>("example/com/networkinfo/NetworkInfo", "getInfo");
    QJsonDocument json = QJsonDocument::fromJson(res.toString().toUtf8());

    if(json.isNull() || !json.isObject()) {
        m_lasterror = "Error in JSON response";
        return NetworkCells();
    }

    QJsonObject cellobj = json.object();
    if(cellobj.contains("error")) {
        m_lasterror = cellobj["error"].toString();
        return NetworkCells();
    } else if(!cellobj.contains("cells")) {
        m_lasterror = "Unexpected error (missing cells array)";
        return NetworkCells();
    }

    NetworkCells networkcells;
    QJsonArray cellarr = cellobj["cells"].toArray();

    for(auto c : cellarr) {
        NetworkCell *cell = new NetworkCell();
        QJsonObject cellobj = c.toObject();

        cell->mcc = cellobj["mcc"].toInt();
        cell->mnc = cellobj["mnc"].toInt();
        cell->lac = cellobj["lac"].toInt();
        cell->cid = cellobj["cid"].toInt();
        cell->ss = cellobj["ss"].toInt();

        networkcells.append(cell);
    }

    return networkcells;
}

void NetworkLocation::updateLocation()
{
    NetworkCells cells = getCellids();
    NetworkCells validcells;

    for(auto cell : cells) {
        QPair<float,float> coords = m_datamgr.getCoords(cell->mcc, cell->mnc, cell->lac, cell->cid);
        if(coords.first == -1 || coords.second == -1) {
            cell->deleteLater();
            continue;
        }

        cell->lon = coords.first;
        cell->lat = coords.second;
        validcells.append(cell);
    }

    // Calculate signal strength sum for weighted signal
    float signalsum = 0;
    for(int i = 0; i < validcells.length(); i++) {
        signalsum += abs(validcells[i]->ss);
    }

    // Calculate weighted signal
    QVector<float> wsignal(validcells.length(), 0);
    for(int i = 0; i < validcells.length(); i++) {
        wsignal[i] = abs(validcells[i]->ss) / signalsum;
    }

    // Calculate location (longitude, latitude)
    QPair<float, float> location(0.0f, 0.0f);
    for(int i = 0; i < validcells.length(); i++) {
        location.first += validcells[i]->lon * wsignal[i];
        location.second += validcells[i]->lat * wsignal[i];
    }

    m_lon = location.first;
    m_lat = location.second;
    m_lastcells = validcells;

    emit locationChanged();
}

QQmlListProperty<NetworkCell> NetworkLocation::cells()
{
    return QQmlListProperty<NetworkCell>(this, m_lastcells);
}
