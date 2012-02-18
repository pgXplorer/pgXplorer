/*
  LICENSE AND COPYRIGHT INFORMATION - Please read carefully.

  Copyright (c) 2011, davyjones <davyjones@github.com>

  Permission to use, copy, modify, and/or distribute this software for any
  purpose with or without fee is hereby granted, provided that the above
  copyright notice and this permission notice appear in all copies.

  THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
  WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
  MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
  ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
  WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
  ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
  OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/

#ifndef CONNECTIONPROPERTIES_H
#define CONNECTIONPROPERTIES_H

#include <QtGui>
#include "database.h"
#include "mainwin.h"

class ConnectionProperties : public QDialog
{
    Q_OBJECT

private:
    QLabel *lTitle;
    QLabel *lSrv;
    QLabel *lDb;
    QLabel *lPort;
    QLabel *lUser;
    QLabel *lPass;

    QLineEdit *lESrv;
    QLineEdit *lEPort;
    QLineEdit *lEDb;
    QLineEdit *lEUser;
    QLineEdit *lEPass;

protected:

private slots:
    void okslot()
    {
        emit oksignal(lESrv->text(),lEPort->text().toInt(),lEDb->text(),
                      lEUser->text(),lEPass->text());
        close();
    }

public:
    ConnectionProperties(Database *db, MainWin *mainwin);
    ~ConnectionProperties();
    void setSrv(QLineEdit *lESrv)
    {
        this->lESrv = lESrv;
    }
    void setPort(QLineEdit *lEPort)
    {
        this->lEPort = lEPort;
    }
    void setDb(QLineEdit *lEDb)
    {
        this->lEDb = lEDb;
    }
    void setUser(QLineEdit *lEUser)
    {
        this->lEUser = lEUser;
    }
    void setPass(QLineEdit *lEPass)
    {
        this->lEPass = lEPass;
    }

    QLineEdit *getSrv()
    {
        return this->lESrv;
    }
    QLineEdit *getPort()
    {
        return this->lEPort;
    }
    QLineEdit *getDb()
    {
        return this->lEDb;
    }
    QLineEdit *getUser()
    {
        return this->lEUser;
    }
    QLineEdit *getPass()
    {
        return this->lEPass;
    }
Q_SIGNALS:
    void oksignal(QString, qint32, QString, QString, QString);
};

#endif // CONNECTIONPROPERTIES_H
