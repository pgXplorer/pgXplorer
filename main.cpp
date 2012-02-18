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

#include <QApplication>
#include <QDesktopWidget>
#include <QTimer>
#include "mainwin.h"
#include "licensedialog.h"

int main(int argc, char **argv)
{
    Q_INIT_RESOURCE(pgXplorer);
    QApplication pgXplorer(argc, argv);

    QString arg1;
    if(argc > 1)
        arg1.append(QString::fromLocal8Bit(argv[1]));

    MainWin *mainwin = new MainWin(0, arg1, Qt::Widget);
    QCoreApplication::setOrganizationName("pgXplorer");
    QCoreApplication::setOrganizationDomain("pgXplorer.com");
    QCoreApplication::setApplicationName("pgXplorer");

    QSettings settings("pgXplorer","pgXplorer");

    QString user = settings.value("license/user", QString()).toString();
    bool site_wide = settings.value("license/site_wide", false).toBool();
    QByteArray key = settings.value("license/key", QString()).toByteArray();

    LicenseDialog *license_dialog = new LicenseDialog(mainwin);
    if(!license_dialog->validLicense(user, site_wide, key)) {
        uint tries = settings.value("license/tries", 0).toUInt();
        settings.setValue("license/tries", ++tries);
        license_dialog->exec();
    }
    else {
        settings.setValue("license/tries", 0);
        license_dialog->close();
    }

    mainwin->show();
    return pgXplorer.exec();
}
