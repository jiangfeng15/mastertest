#ifndef DISKINFO_H
#define DISKINFO_H

#include "public/utils/command_util.h"
#include "public/utils/file_util.h"
#include <QStorageInfo>
#include <QSet>


#define PROC_MOUNTS "/proc/mounts"
#define PROC_LSBLK  "lsblk -d -P -o NAME,KNAME,FSTYPE,MODEL,SERIAL,SIZE,VENDOR,HOTPLUG"
#define PROC_DF_T   "df -T"

class Disk;
class DiskHardInfo;

class DiskInfo
{
public:
    //获取分区信息
    QList<Disk*> getDisks() const;
    void updateDiskInfo();
    QList<quint64> getDiskIO() const;
    QStringList getDiskNames() const;
    QList<QString> fileSystemTypes();
    QList<QString> devices();
    ~DiskInfo();
    //新增加
    QString getFileTypeByDiskName(QString qsDiskName) const;
    //获取磁盘硬件信息，厂商，等
    void getDiskInfo();

public:
    QList<Disk*> disks;

    QList<DiskHardInfo *> m_stDiskHardInfo;
};

struct Disk {
    QString name;
    QString device;
    QString fileSystemType;
    quint64 size;
    quint64 free;
    quint64 used;
};
struct DiskHardInfo
{
    QString qsName;
    QString qsCaption;
    QString qsFileSystemType;
    QString qsSerialNo;
    QString qsManufauturer;
    quint64 qlSize;
    QString qsHotPlug; //热插拔
};

#endif // DISKINFO_H
