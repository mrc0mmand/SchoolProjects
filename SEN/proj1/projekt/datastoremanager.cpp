#include <QFile>
#include <QDebug>
#include <QStringList>

#include "datastoremanager.h"

DataStoreManager::DataStoreManager()
{
    m_datafile = "assets:/cell_towers_CZ.csv";
    loadData();
}

void DataStoreManager::loadData()
{
    QFile in(m_datafile);
    if(!in.open(QIODevice::ReadOnly)) {
        qDebug() << in.errorString();
        return;
    }

    QTextStream instream(&in);
    while(!instream.atEnd()) {
        DataStoreUnit dunit;
        QStringList line = instream.readLine().split(',');
        dunit.mcc = line[1].toInt();
        dunit.mnc = line[2].toInt();
        dunit.lac = line[3].toInt();
        dunit.cid = line[4].toInt();
        dunit.lon = line[6].toFloat();
        dunit.lat = line[7].toFloat();

        m_datastore.append(dunit);
    }
}

QPair<float, float> DataStoreManager::getCoords(int mcc, int mnc, int lac, int cid)
{
    for(const DataStoreUnit &u : m_datastore) {
        if(u.mcc == mcc && u.mnc == mnc && u.lac == lac && u.cid == cid) {
            return QPair<float,float>(u.lon, u.lat);
        }
    }

    return QPair<float, float>(-1.0f, -1.0f);
}
