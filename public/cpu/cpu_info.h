#ifndef CPUINFO_H
#define CPUINFO_H

#include <QDebug>
#include <QVector>

#include "public/utils/file_util.h"

#define PROC_CPUINFO "/proc/cpuinfo"
#define LSCPU_COMMAND "LANG=nl_NL.UTF-8 lscpu"
#define PROC_LOADAVG "/proc/loadavg"
#define PROC_STAT    "/proc/stat"


class CpuInfo
{
public:
    int getCpuPhysicalCoreCount() const;
    int getCpuCoreCount() const;
    QList<int> getCpuPercents() const;
    QList<double> getLoadAvgs() const;
    double getAvgClock() const;
    QList<double> getClocks() const;
    //补充
    QString getCpuVendor() const;
    QString getCpuFamily() const;
    QString getCpuModel() const;
    QString getCpuModelName() const;
    quint64  getCpuMHZ() const; //单位MHz
    quint64  getCpuCache() const; //单位K
private:
    int getCpuPercent(const QList<double> &cpuTimes, const int &processor = 0) const;
};

#endif // CPUINFO_H
