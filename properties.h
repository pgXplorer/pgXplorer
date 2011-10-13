#ifndef PROPERTIES_H
#define PROPERTIES_H

#include <QtGui>
#include "database.h"
#include "mainWin.h"

class PropDialog : public QDialog
{
    Q_OBJECT

private:
    QLineEdit* lESrv;
    QLineEdit* lEPort;
    QLineEdit* lEDb;
    QLineEdit* lEUser;
    QLineEdit* lEPass;

private slots:
    void okslot()
    {
        close();
        emit oksignal(lESrv->text(),lEPort->text().toInt(),lEDb->text(),
                      lEUser->text(),lEPass->text());
    }

public:
    PropDialog(Database *db);
    ~PropDialog();
    void setSrv(QLineEdit* lESrv)
    {
        this->lESrv = lESrv;
    }
    void setPort(QLineEdit* lEPort)
    {
        this->lEPort = lEPort;
    }
    void setDb(QLineEdit* lEDb)
    {
        this->lEDb = lEDb;
    }
    void setUser(QLineEdit* lEUser)
    {
        this->lEUser = lEUser;
    }
    void setPass(QLineEdit* lEPass)
    {
        this->lEPass = lEPass;
    }

    QLineEdit* getSrv()
    {
        return this->lESrv;
    }
    QLineEdit* getPort()
    {
        return this->lEPort;
    }
    QLineEdit* getDb()
    {
        return this->lEDb;
    }
    QLineEdit* getUser()
    {
        return this->lEUser;
    }
    QLineEdit* getPass()
    {
        return this->lEPass;
    }
Q_SIGNALS:
    void oksignal(QString, qint32, QString, QString, QString);
};

#endif // PROPERTIES_H