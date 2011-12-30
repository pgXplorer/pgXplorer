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

#ifndef LICENSEDIALOG_H
#define LICENSEDIALOG_H

#include <QtGui>
#include "database.h"
#include "mainWin.h"

class LicenseDialog : public QDialog
{
    Q_OBJECT

private:
    QLabel *title_label;
    QLabel *user_label;
    QLabel *site_wide_label;
    QLabel *key_label;

    QLineEdit *user;
    QCheckBox *site_wide;
    QLineEdit *key;

private slots:
    void okslot();
    void cancel();

protected:
    void closeEvent(QCloseEvent *);

public:
    LicenseDialog(MainWin *mainwin);
    bool validLicense(QString, bool, QByteArray);
};

#endif // LICENSEDIALOG_H
