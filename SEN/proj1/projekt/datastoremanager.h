#ifndef DATASTOREMANAGER_H
#define DATASTOREMANAGER_H

#include <QObject>

class DataStoreManager
{
public:
    class DataStoreUnit
    {
    public:
        qint32 mcc;
        qint32 mnc;
        qint32 lac;
        qint32 cid;
        float lon;
        float lat;

        QString toString() {
            QString res;
            res.sprintf("mcc: %d, mnc: %d, lac: %d, cid: %d, lon: %f, lat: %f",
                        mcc, mnc, lac, cid, lon, lat);
            return res;
        }
    };

    typedef QList<DataStoreUnit> DataStore;

    DataStoreManager();
    QPair<float, float> getCoords(int mcc, int mnc, int lac, int cid);

private:
    QString m_datafile;
    DataStore m_datastore;

    void loadData();
};

#endif // DATASTOREMANAGER_H
