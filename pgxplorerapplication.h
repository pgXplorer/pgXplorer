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

#ifndef PGXPLORERAPPLICATION_H
#define PGXPLORERAPPLICATION_H

#include <QApplication>
#include "mainwin.h"

class PgxplorerApplication : public QApplication
{
    Q_OBJECT

private:
    void loadFile(QString);
    MainWin *default_mainwin;

protected:
    bool event(QEvent *);

public:
    PgxplorerApplication(int & argc, char **argv);
    
signals:
    
public slots:
    
};

#endif // PGXPLORERAPPLICATION_H
