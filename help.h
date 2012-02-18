#ifndef HELP_H
#define HELP_H

#include <QtCore/QString>

class QProcess;

class Help
{
public:
    Help(QString);
    ~Help();
    void showDocumentation(const QString &file);

private:
    bool startHelp();
    QString app_dir;
    QProcess *process;
};

#endif // HELP_H
