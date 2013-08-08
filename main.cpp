/*
  LICENSE AND COPYRIGHT INFORMATION - Please read carefully.

  Copyright (c) 2010-2013, davyjones <dj@pgxplorer.com>

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

#include "pgxplorerapplication.h"
#include <QtWidgets/QtWidgets>
#include <QTimer>
#include "mainwin.h"
#include "licensedialog.h"

int main(int argc, char **argv)
{
    Q_INIT_RESOURCE(pgXplorer);
    PgxplorerApplication pgXplorer(argc, argv);

    QString arg1;
    if(argc > 1)
        arg1.append(QString::fromLocal8Bit(argv[1]));

    QCoreApplication::setOrganizationName("pgXplorer");
    QCoreApplication::setOrganizationDomain("pgXplorer.com");
    QCoreApplication::setApplicationName("pgXplorer");

    QSettings settings("pgXplorer", "pgXplorer");
    settings.remove("license");
    /*
    Disabling license nag dialog for now.
    The source code will always be free. Just to keep track
    of potential paying customers, enable a simple License
    key class.

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
    */

    //qApp->setStyleSheet("QDialog {background-color: beige; font: bold 14px;} QPushButton {background-color: lightgray; \
    //                      border-style: outset; \
    //                      border-width: 2px; \
    //                      border-radius: 10px; \
    //                      border-color: beige; \
    //                      font: bold 14px; \
    //                      min-width: 6em; \
    //                      padding: 6px;} \
    //                      QPushButton:pressed {background-color: rgb(255, 250, 200); \
    //                      border-style: inset;}");

    qApp->setStyleSheet("QTableView {selection-background-color: \
                            qlineargradient(x1: 0, y1: 0, x2: 1, y2: 0.25, \
                            stop: 0 #5F5F7F, stop: 1 #7F7F9F); \
                            selection-color: #F0F0F0; \
                            color: #0F0F0F; \
                        }");

    return pgXplorer.exec();
}
