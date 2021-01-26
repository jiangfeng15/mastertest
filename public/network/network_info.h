#ifndef NETWORK_INFO_H
#define NETWORK_INFO_H
#include <QtNetwork/QNetworkInterface>
#include "public/utils/file_util.h"
#include "public/utils/command_util.h"

#include <QVector>
#include <QList>

typedef struct netIpInfo
{
    QString qsNetName;
    QString qsIp;
    QString qsMac;
    QString qsMask;
    QString qsBroadCast;
}ST_NETIPINFO,*PST_NETIPINFO;

class NetworkInfo
{
public:
    NetworkInfo();

    QString getDefaultNetworkInterface() const;
    QList<QNetworkInterface> getAllInterfaces();

    quint64 getRXbytes() const;
    quint64 getTXbytes() const;
    void getNetInfo();

private:
    QString defaultNetworkInterface;

    QString rxPath;
    QString txPath;
public:
    QVector<ST_NETIPINFO> m_qvecNet;
};

#endif // NETWORK_INFO_H
