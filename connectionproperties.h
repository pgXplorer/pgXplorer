/*
  LICENSE AND COPYRIGHT INFORMATION - Please read carefully.

  Copyright (c) 2011-2012, davyjones <davyjones@github>

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
    QLabel *title;
    QLineEdit *server;
    QLineEdit *port;
    QLineEdit *db_name;
    QLineEdit *username;
    QLineEdit *password;
    QDialogButtonBox *button_box;

private slots:
    void okslot()
    {
        emit oksignal(server->text(),port->text().toInt(),db_name->text(),
                      username->text(),password->text());
        close();
    }

public:
    ConnectionProperties(Database *database, MainWin *mainwin);
    ~ConnectionProperties()
    {
        delete title;
        delete server;
        delete port;
        delete db_name;
        delete username;
        delete password;
        delete button_box;
    }

signals:
    void oksignal(QString, qint32, QString, QString, QString);
};

#endif // CONNECTIONPROPERTIES_H
