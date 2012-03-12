#include <QByteArray>
#include <QDir>
#include <QLibraryInfo>
#include <QtCore/QProcess>
#include <QMessageBox>
#include "help.h"

Help::Help(QString app_dir)
    : process(0)
{
    this->app_dir = app_dir;
}

Help::~Help()
{
    if (process && process->state() == QProcess::Running) {
        process->terminate();
        //process->waitForFinished(3000);
    }
    delete process;
}

void Help::showDocumentation(const QString &page)
{
    if (!startHelp())
        return;

    QByteArray ba("expandToc -1;");
    ba.append("SetSource ");
    ba.append("qthelp://com.pgxplorer/doc/");

    process->write(ba + page.toLocal8Bit() + '\n');
}

bool Help::startHelp()
{
    if (!process)
        process = new QProcess();

    if (process->state() != QProcess::Running) {
        QString app = app_dir + QDir::separator(); //QLibraryInfo::location(QLibraryInfo::BinariesPath) + QDir::separator();
#if !defined(Q_OS_MAC)
        app += QLatin1String("assistant");
#else
        app += QLatin1String("Assistant.app/Contents/MacOS/Assistant");
#endif

        QStringList args;
        args << QLatin1String("-collectionFile")
            << app_dir + QLatin1String("/pgXplorer.qhc")
            << QLatin1String("-enableRemoteControl");

        process->start(app, args);

        if (!process->waitForStarted()) {
            QMessageBox::critical(0, QLatin1String("pgXplorer"),
                QObject::tr("Unable to launch help file (%1)").arg(app));
            return false;
        }
    }
    return true;
}
