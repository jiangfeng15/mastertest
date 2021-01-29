#include "disk_info.h"
#include <QDebug>

QList<Disk*> DiskInfo::getDisks() const
{
    return disks;
}

void DiskInfo::updateDiskInfo()
{
    qDeleteAll(disks);
    disks.clear();

    QList<QStorageInfo> storageInfoList = QStorageInfo::mountedVolumes();

    for(const QStorageInfo &info: storageInfoList) {
        if (info.isValid()) {
            Disk *disk = new Disk();
            disk->name = info.displayName();
            disk->device = info.device();
            disk->size = info.bytesTotal();
            disk->used = info.bytesTotal() - info.bytesFree();
            disk->free = info.bytesFree();
            disk->fileSystemType = info.fileSystemType();

            disks << disk;
        }
    }
}

QList<QString> DiskInfo::devices()
{
    QSet<QString> set;
    for(const QStorageInfo &info: QStorageInfo::mountedVolumes()) {
        if (info.isValid()) set.insert(info.device());
    }

    return set.toList();
}

DiskInfo::~DiskInfo()
{
    qDeleteAll(disks);
    qDeleteAll(m_stDiskHardInfo);
}

QList<QString> DiskInfo::fileSystemTypes()
{
    QSet<QString> set;
    for(const QStorageInfo &info: QStorageInfo::mountedVolumes()) {
        if (info.isValid()) set.insert(info.fileSystemType());
    }

    return set.toList();
}

QList<quint64> DiskInfo::getDiskIO() const
{
    static QStringList diskNames = getDiskNames();

    QList<quint64> diskReadWrite;
    quint64 totalRead = 0;
    quint64 totalWrite = 0;

    for (const QString diskName : diskNames) {
      QStringList diskStat = FileUtil::readStringFromFile(QString("/sys/block/%1/stat").arg(diskName))
              .trimmed()
              .split(QRegExp("\\s+"));

      if (diskStat.count() > 7) {
          totalRead = totalRead + (diskStat.at(2).toLongLong() * 512);
          totalWrite = totalWrite + (diskStat.at(6).toLongLong() * 512);
      }
    }
    diskReadWrite.append(totalRead);
    diskReadWrite.append(totalWrite);

    return diskReadWrite;
}

QStringList DiskInfo::getDiskNames() const
{
    QDir blocks("/sys/block");
    QStringList disks;
    for (const QFileInfo entryInfo : blocks.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot)) {
        if (QFile::exists(QString("%1/device").arg(entryInfo.absoluteFilePath()))) {
            disks.append(entryInfo.baseName());
        }
    }
    return disks;
}
QString DiskInfo::getFileTypeByDiskName(QString qsDiskName) const
{
    QString qsFileType,qsFsName;
    static quint8 count = 0;

    if (! count) {
        const QStringList lines = CommandUtil::exec("bash",{"-c", PROC_DF_T}).split("\n");
        if (! lines.isEmpty())
        {
            for(QString echoLine : lines)
            {
                qsFsName = echoLine.split(QRegExp("\\s+")).at(0);
                if(qsFsName.indexOf(qsDiskName)!=-1) //找到
                {
                    qsFileType = echoLine.split(QRegExp("\\s+")).at(1);
                    break;
                }
            }
        }
    }
    return qsFileType;
}
void DiskInfo::getDiskInfo()
{
    QString qsDiskName;
    static quint8 count = 0;

    if (! count) {
        //1 获取diskname
        static QStringList diskNames = getDiskNames();
        //2 根据diskname匹配lsblk信息
        for(QString eachDiskName:diskNames)
        {
            const QStringList lines = CommandUtil::exec("bash",{"-c", PROC_LSBLK}).split("\n");
            if (! lines.isEmpty())
            {
                for(QString echoLine : lines)
                {
                    echoLine = echoLine.remove("\"");
                    qsDiskName = echoLine.split(QRegExp("\\s+")).at(0);
                    qsDiskName = qsDiskName.split("=").last();
                    if(qsDiskName.indexOf(eachDiskName)!=-1 && qsDiskName.indexOf("sd") == 0) //找到,只处理硬盘，不处理光驱
                    {

                        DiskHardInfo * stDiskInfo = new DiskHardInfo;
                        QStringList resList = echoLine.split(QRegExp("\\s+"));
                        stDiskInfo->qsName = qsDiskName;
                        stDiskInfo->qsFileSystemType = getFileTypeByDiskName(qsDiskName);
                        stDiskInfo->qsCaption = resList.at(3).split("=").last();
                        stDiskInfo->qsSerialNo = resList.at(4).split("=").last();
                        stDiskInfo->qlSize = resList.at(5).split("=").last().split("G").first().toDouble()*1024*1024;
                        stDiskInfo->qsManufauturer = resList.at(6).split("=").last();
                        stDiskInfo->qsHotPlug = resList.at(7).split("=").last();

                        m_stDiskHardInfo<<stDiskInfo;
                    }
                }
            }

        }

    }
}
