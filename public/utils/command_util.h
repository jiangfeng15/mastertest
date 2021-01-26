#ifndef COMMAND_UTIL_H
#define COMMAND_UTIL_H

#include <QtCore/qglobal.h>
#include <QStringList>

class CommandUtil
{
public:
    static QString sudoExec(const QString &cmd, QStringList args = QStringList(), QByteArray data = QByteArray());
    static QString exec(const QString &cmd, QStringList args = QStringList(), QByteArray data = QByteArray());
    static bool isExecutable(const QString &cmd);
};

#endif // COMMAND_UTIL_H
