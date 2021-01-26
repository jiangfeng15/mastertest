﻿#include "cpu_info.h"

#include "public/utils/command_util.h"

int CpuInfo::getCpuPhysicalCoreCount() const
{
    static int count = 0;

    if (! count) {
        QStringList cpuinfo = FileUtil::readListFromFile(PROC_CPUINFO);

        if (! cpuinfo.isEmpty()) {
	    QSet<QPair<int, int> > physicalCoreSet;
	    int physical = 0;
	    int core = 0;
	    for (int i = 0; i < cpuinfo.size(); ++i) {
	        const QString& line = cpuinfo[i];
		if (line.startsWith("physical id")) {
		    QStringList fields = line.split(": ");
		    if (fields.size() > 1)
		        physical = fields[1].toInt();
		}
		if (line.startsWith("core id")) {
		    QStringList fields = line.split(": ");
		    if (fields.size() > 1)
		        core = fields[1].toInt();
		    // We assume core id appears after physical id.
		    physicalCoreSet.insert(qMakePair(physical, core));
		}
	    }
	    count = physicalCoreSet.size();
	}
    }

    return count;
}

int CpuInfo::getCpuCoreCount() const
{
    static quint8 count = 0;

    if (! count) {
        QStringList cpuinfo = FileUtil::readListFromFile(PROC_CPUINFO);

        if (! cpuinfo.isEmpty())
            count = cpuinfo.filter(QRegExp("^processor")).count();
    }

    return count;
}

QList<double> CpuInfo::getLoadAvgs() const
{
    QList<double> avgs = {0, 0, 0};

    QStringList strListAvgs = FileUtil::readStringFromFile(PROC_LOADAVG).split(QRegExp("\\s+"));

    if (strListAvgs.count() > 2) {
        avgs.clear();
        avgs << strListAvgs.takeFirst().toDouble();
        avgs << strListAvgs.takeFirst().toDouble();
        avgs << strListAvgs.takeFirst().toDouble();
    }

    return avgs;
}

double CpuInfo::getAvgClock() const
{
    const QStringList lines = CommandUtil::exec("bash",{"-c", LSCPU_COMMAND}).split('\n');
    const QString clockMHz = lines.filter(QRegExp("^CPU MHz")).first().split(":").last();
    return clockMHz.toDouble();
}

QList<double> CpuInfo::getClocks() const
{
    QStringList lines = FileUtil::readListFromFile(PROC_CPUINFO)
            .filter(QRegExp("^cpu MHz"));

    QList<double> clocks;
    for(auto line: lines){
        clocks.push_back(line.split(":").last().toDouble());
    }
    return clocks;
}

QList<int> CpuInfo::getCpuPercents() const
{
    QList<double> cpuTimes;

    QList<int> cpuPercents;

    QStringList times = FileUtil::readListFromFile(PROC_STAT);

    if (! times.isEmpty())
    {
     /*  user nice system idle iowait  irq  softirq steal guest guest_nice
        cpu  4705 356  584    3699   23    23     0       0     0      0
         .
        cpuN 4705 356  584    3699   23    23     0       0     0      0

          The meanings of the columns are as follows, from left to right:
             - user: normal processes executing in user mode
             - nice: niced processes executing in user mode
             - system: processes executing in kernel mode
             - idle: twiddling thumbs
             - iowait: waiting for I/O to complete
             - irq: servicing interrupts
             - softirq: servicing softirqs
             - steal: involuntary wait
             - guest: running a normal guest
             - guest_nice: running a niced guest
        */

        QRegExp sep("\\s+");
        int count = CpuInfo::getCpuCoreCount() + 1;
        for (int i = 0; i < count; ++i)
        {
            QStringList n_times = times.at(i).split(sep);
            n_times.removeFirst();
            for (const QString &t : n_times)
                cpuTimes << t.toDouble();

            cpuPercents << getCpuPercent(cpuTimes, i);

            cpuTimes.clear();
        }
    }

    return cpuPercents;
}

int CpuInfo::getCpuPercent(const QList<double> &cpuTimes, const int &processor) const
{
    const int N = getCpuCoreCount()+1;

    static QVector<double> l_idles(N);
    static QVector<double> l_totals(N);

    int utilisation = 0;

    if (cpuTimes.count() > 0) {

        double idle = cpuTimes.at(3) + cpuTimes.at(4); // get (idle + iowait)
        double total = 0.0;
        for (const double &t : cpuTimes) total += t; // get total time

        double idle_delta  = idle  - l_idles[processor];
        double total_delta = total - l_totals[processor];

        if (total_delta)
            utilisation = 100 * ((total_delta - idle_delta) / total_delta);

        l_idles[processor] = idle;
        l_totals[processor] = total;
    }

    if (utilisation > 100) utilisation = 100;
    else if (utilisation < 0) utilisation = 0;

    return utilisation;
}

QString CpuInfo::getCpuVendor() const
{
    QString qsRes,qsTemp;
    static quint8 count = 0;

    if (! count) {
        const QStringList lines = CommandUtil::exec("bash",{"-c", LSCPU_COMMAND}).split('\n');

        if (! lines.isEmpty())
        {
            qsTemp = lines.filter(QRegExp("Vendor ID")).at(0);
            if(!qsTemp.isEmpty())
            {
                qsRes = qsTemp.split(":").last().trimmed();
            }
        }
    }
    return qsRes;
}
QString CpuInfo::getCpuFamily() const
{
    QString qsRes,qsTemp;
    static quint8 count = 0;

    if (! count) {
        const QStringList lines = CommandUtil::exec("bash",{"-c", LSCPU_COMMAND}).split('\n');

        if (! lines.isEmpty())
        {
            qsTemp = lines.filter(QRegExp("CPU family")).at(0);
            if(!qsTemp.isEmpty())
            {
                qsRes = qsTemp.split(":").last().trimmed();
            }
        }
    }
    return qsRes;
}
QString CpuInfo::getCpuModel() const
{
    QString qsRes,qsTemp;
    static quint8 count = 0;

    if (! count) {
        const QStringList lines = CommandUtil::exec("bash",{"-c", LSCPU_COMMAND}).split('\n');

        if (! lines.isEmpty())
        {
            qsTemp = lines.filter(QRegExp("Model")).at(0);
            if(!qsTemp.isEmpty())
            {
                qsRes = qsTemp.split(":").last().trimmed();
            }
        }
    }
    return qsRes;
}
QString CpuInfo::getCpuModelName() const
{
    QString qsRes,qsTemp;
    static quint8 count = 0;

    if (! count) {
        const QStringList lines = CommandUtil::exec("bash",{"-c", LSCPU_COMMAND}).split('\n');

        if (! lines.isEmpty())
        {
            qsTemp = lines.filter(QRegExp("Model name")).at(0);
            if(!qsTemp.isEmpty())
            {
                qsRes = qsTemp.split(":").last().trimmed();
            }
        }
    }
    return qsRes;
}
quint64 CpuInfo::getCpuMHZ() const
{
    quint64 qlRes=0;
    QString qsMHz, qsTemp;
    static quint8 count = 0;

    if (! count) {
        const QStringList lines = CommandUtil::exec("bash",{"-c", LSCPU_COMMAND}).split('\n');

        if (! lines.isEmpty())
        {
            qsTemp = lines.filter(QRegExp("CPU MHz")).at(0);
            if(!qsTemp.isEmpty())
            {
                qsMHz = qsTemp.split(":").last().trimmed();
                qlRes = qsMHz.toDouble();
            }
        }
    }
    return qlRes;
}
quint64 CpuInfo::getCpuCache() const
{
    quint64 qlRes=0;
    QString qsCache, qsTemp;
    static quint8 count = 0;

    if (! count) {
        const QStringList lines = CommandUtil::exec("bash",{"-c", LSCPU_COMMAND}).split('\n');

        if (! lines.isEmpty())
        {
            qsTemp = lines.filter(QRegExp("L3 cache")).at(0);
            if(!qsTemp.isEmpty())
            {
                qsCache = qsTemp.split(":").last().trimmed();
                if(!qsCache.isEmpty())
                {
                    qsCache = qsCache.split("K").first();
                    qlRes = qsCache.toULongLong();
                }

            }
        }
    }
    return qlRes;
}