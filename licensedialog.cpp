/*
  LICENSE AND COPYRIGHT INFORMATION - Please read carefully.

  Copyright (c) 2011, davyjones <davyjones@github>

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

#include "licensedialog.h"

bool LicenseDialog::validLicense(QString user, bool site_wide, QByteArray key)
{
    user.simplified();
    key.simplified();
    key.toLower();
    QCryptographicHash hash(QCryptographicHash::Md5);
    if(site_wide)
        hash.addData((user.append("yes")).toAscii());
    else
        hash.addData(user.toAscii());
    QByteArray hash_result = hash.result();

    //Reduce hash to 4 bytes by choosing only
    //every fourth byte. Results in a 8 character
    //license key.
    for(int i=0; i < hash_result.size(); i++)
        hash_result.remove(i,1);
    for(int i=0; i < hash_result.size(); i++)
        hash_result.remove(i,1);

    if(key == hash_result.toHex())
        return true;
    else
        return false;
}

LicenseDialog::LicenseDialog(MainWin *mainwin)
{
    QPushButton *ok_button;
    QPushButton *trial_button;
    QPushButton *cancel_button;

    setWindowFlags(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_DeleteOnClose);
    resize(512, 288);
    setModal(true);
    if(mainwin->isVisible())
        setParent(mainwin);
    setWindowFlags(Qt::Window);
    setWindowModality(Qt::WindowModal);

    QFont font;
    font.setFamily(QString::fromUtf8("Verdana"));

    ok_button = new QPushButton(this);
    ok_button->setObjectName(QString::fromUtf8("pBOK"));
    ok_button->setAutoDefault(false);
    ok_button->setGeometry(QRect(30, 250, 75, 23));
    ok_button->setFont(font);
    //trial_button = new QPushButton(this);
    //trial_button->setObjectName(QString::fromUtf8("pBCancel"));
    //trial_button->setGeometry(QRect(140, 250, 75, 23));
    //trial_button->setFont(font);
    //trial_button->setAutoDefault(false);
    cancel_button = new QPushButton(this);
    cancel_button->setObjectName(QString::fromUtf8("pBCancel"));
    cancel_button->setGeometry(QRect(140, 250, 75, 23));
    cancel_button->setFont(font);
    cancel_button->setAutoDefault(false);

    user = new QLineEdit(this);
    user->setGeometry(QRect(20, 80, 133, 20));
    user->setFont(font);
    user->setAutoFillBackground(true);
    key = new QLineEdit(this);
    key->setGeometry(QRect(20, 140, 133, 20));
    key->setFont(font);
    key->setAutoFillBackground(true);

    user_label = new QLabel(this);
    user_label->setGeometry(QRect(20, 50, 133, 20));
    //user_label->setObjectName(QString::fromUtf8("user"));
    user_label->setFont(font);
    key_label = new QLabel(this);
    key_label->setGeometry(QRect(20, 110, 133, 20));
    key_label->setObjectName(QString::fromUtf8("key"));
    key_label->setFont(font);
    site_wide_label = new QLabel("&S", this);
    site_wide_label->setGeometry(QRect(20, 180, 210, 20));
    site_wide_label->setObjectName(QString::fromUtf8("site_wide"));
    site_wide_label->setFont(font);
    site_wide = new QCheckBox(this);
    site_wide->setObjectName(QString::fromUtf8("lEPort"));
    site_wide->setGeometry(QRect(20, 181, 20, 20));
    site_wide_label->setBuddy(site_wide);
    site_wide_label->setToolTip("");
    title_label = new QLabel(this);
    title_label->setObjectName(QString::fromUtf8("lTitle"));
    title_label->setGeometry(QRect(20, 10, 211, 21));
    title_label->setAlignment(Qt::AlignHCenter);
    QFont font1;
    font1.setFamily(QString::fromUtf8("Verdana"));
    font1.setPointSize(14);
    title_label->setFont(font1);

    QWidget::setTabOrder(user, key);
    QWidget::setTabOrder(key, site_wide);
    QWidget::setTabOrder(site_wide, ok_button);
    QWidget::setTabOrder(ok_button, cancel_button);

    QObject::connect(cancel_button, SIGNAL(clicked()), this, SLOT(cancel()));
    QObject::connect(ok_button, SIGNAL(clicked()), this, SLOT(okslot()));

    setWindowTitle(QApplication::translate("Connection", "Connection", 0, QApplication::UnicodeUTF8));
    ok_button->setText(QApplication::translate("Connection", "OK", 0, QApplication::UnicodeUTF8));
    cancel_button->setText(QApplication::translate("Connection", "Cancel", 0, QApplication::UnicodeUTF8));

    title_label->setText(QApplication::translate("Connection", "pgXplorer License Key", 0, QApplication::UnicodeUTF8));
    user_label->setText(QApplication::translate("Connection", "Registration name:", 0, QApplication::UnicodeUTF8));
    site_wide_label->setText(QApplication::translate("Connection", "    This is a site-wide license key.", 0, QApplication::UnicodeUTF8));
    key_label->setText(QApplication::translate("Connection", "Registration key:", 0, QApplication::UnicodeUTF8));

    QTextEdit *display_text = new QTextEdit(this);
    display_text->setAutoFillBackground(true);
    display_text->setStyleSheet("QTextEdit {background: }");
    QPalette palette;
    palette.setColor(QPalette::Base, QColor(Qt::lightGray).lighter(125));
    display_text->setPalette(palette);
    display_text->setReadOnly(true);
    display_text->setGeometry(260,20,220,250);
    display_text->insertHtml("<b>pgXplorer</b> is an open source <a href=http://postgresql.org>PostgreSQL</a> administrative client.</br>");
    display_text->append("\nYou can purchase an individual or site-wide license key at pgxplorer.com. Buying a license key includes support \
as well as priority ticket requests.");
    display_text->append("\nYou can also freely download the source and build pgXplorer yourself. \
The source code is available at: github.com/davyjones/pgxplorer");
    display_text->append("Note that you will have to compile the postgresql database drivers \
and obtain other necessary platform specific libraries if you are compiling from source.");
    display_text->append("\nUnfortunately, unless you have a valid license key, we are unable \
to provide support when you build pgXplorer yourself.\n\
Refer to the README document at the source code location for more details on build instructions.");
    display_text->moveCursor(QTextCursor::Start);

     QSettings settings("pgXplorer","pgXplorer");
     QString user = settings.value("license/user", QString()).toString();
     bool site_wide = settings.value("license/site_wide", false).toBool();
     QByteArray key = settings.value("license/key", QString()).toByteArray();

     if(validLicense(user, site_wide, key)) {
         this->user->setText(user);
         this->site_wide->setChecked(site_wide);
         this->key->setText(key);
     }
}

void LicenseDialog::okslot()
{
    if(!validLicense(user->text(), site_wide->isChecked(), key->text().toAscii())) {
        QMessageBox invalid_license(this);
        invalid_license.setIcon(QMessageBox::Critical);
        invalid_license.setText(tr("The license information provided is invalid."));
        invalid_license.setStandardButtons(QMessageBox::Ok);
        invalid_license.setDefaultButton(QMessageBox::Ok);
        invalid_license.exec();
        user->selectAll();
        user->setFocus();
    }
    else {
        QSettings settings("pgXplorer","pgXplorer");
        settings.setValue("license/user", user->text());
        settings.setValue("license/site_wide", site_wide->isChecked());
        settings.setValue("license/key", key->text());
        close();
    }
}

void LicenseDialog::cancel()
{
    close();
}

void LicenseDialog::closeEvent(QCloseEvent *event)
{
    QSettings settings("pgXplorer","pgXplorer");
    uint tries = settings.value("license/tries", 0).toUInt();
    QString user = settings.value("license/user", QString()).toString();
    bool site_wide = settings.value("license/site_wide", false).toBool();
    QByteArray key = settings.value("license/key", QString()).toByteArray();

    QDialog::closeEvent(event);
    if(!validLicense(user, site_wide, key)) {
        if(tries>30)
            exit(199);
    }
}
