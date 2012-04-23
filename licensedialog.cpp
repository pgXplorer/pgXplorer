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
    cancel_button = new QPushButton(this);
    cancel_button->setObjectName(QString::fromUtf8("pBCancel"));
    cancel_button->setGeometry(QRect(140, 250, 75, 23));
    cancel_button->setFont(font);
    cancel_button->setAutoDefault(false);
    trial_button = new QPushButton(this);
    trial_button->setGeometry(QRect(100, 150, 150, 100));
    trial_button->setFont(font);
    trial_button->setAutoDefault(false);
    trial_button->setIcon(QIcon(":/icons/buy.png"));
    trial_button->setGraphicsEffect(new QGraphicsDropShadowEffect);
    trial_button->setIconSize(QSize(48,48));
    trial_button->hide();

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
    QFont title_font;
    title_font.setFamily(QString::fromUtf8("Verdana"));
    title_font.setPointSize(14);
    title_label->setFont(title_font);

    QWidget::setTabOrder(user, key);
    QWidget::setTabOrder(key, site_wide);
    QWidget::setTabOrder(site_wide, ok_button);
    QWidget::setTabOrder(ok_button, cancel_button);

    QObject::connect(cancel_button, SIGNAL(clicked()), this, SLOT(cancel()));
    QObject::connect(ok_button, SIGNAL(clicked()), this, SLOT(okslot()));
    QObject::connect(trial_button, SIGNAL(clicked()), this, SLOT(launchBuyLink()));

    setWindowTitle(QApplication::translate("License", "License", 0, QApplication::UnicodeUTF8));
    ok_button->setText(QApplication::translate("License", "OK", 0, QApplication::UnicodeUTF8));
    cancel_button->setText(QApplication::translate("License", "Cancel", 0, QApplication::UnicodeUTF8));
    trial_button->setFont(title_font);
    trial_button->setText(QApplication::translate("License", "Buy\nkey", 0, QApplication::UnicodeUTF8));

    title_label->setText(QApplication::translate("License", "pgXplorer License Key", 0, QApplication::UnicodeUTF8));
    user_label->setText(QApplication::translate("License", "Registration name:", 0, QApplication::UnicodeUTF8));
    site_wide_label->setText(QApplication::translate("License", "    This is a site-wide license key.", 0, QApplication::UnicodeUTF8));
    key_label->setText(QApplication::translate("License", "Registration key:", 0, QApplication::UnicodeUTF8));

    QLabel *display_text = new QLabel(this);
    display_text->setTextFormat(Qt::RichText);
    //display_text->setFrameStyle(QFrame::Panel | QFrame::Raised);
    QString tata(tr("first line\nsecond line"));
    display_text->setText(tata);
    display_text->setAlignment(Qt::AlignTop| Qt::AlignLeft);
    display_text->setAutoFillBackground(true);
    display_text->setStyleSheet("QTextEdit {background: }");
    QPalette palette;
    palette.setColor(QPalette::Base, QColor(Qt::lightGray).lighter(125));
    display_text->setGeometry(260,20,220,250);
    QString dt(tr("<b>pgXplorer</b> is an open source PostgreSQL administrative client.\nYou can purchase an individual or site-wide license key at pgxplorer.com. Buying a license key includes support as well as priority ticket requests.\nYou can also freely download the source and build pgXplorer yourself.The source code is available at: github.com/davyjones/pgxplorer \nRefer to the README document at the source code location for more details on build instructions."));

    display_text->setText(dt);

    QSettings settings("pgXplorer","pgXplorer");
    QString user = settings.value("license/user", QString()).toString();
    bool site_wide = settings.value("license/site_wide", false).toBool();
    QByteArray key = settings.value("license/key", QString()).toByteArray();

    if(validLicense(user, site_wide, key)) {
        this->user->setText(user);
        this->site_wide->setChecked(site_wide);
        this->key->setText(key);
        if(!this->site_wide) {
            trial_button->setText(QApplication::translate("License", "Buy site-wide key", 0, QApplication::UnicodeUTF8));
            trial_button->show();
        }
    }
    else {
        trial_button->show();
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

void LicenseDialog::launchBuyLink()
{
    QDesktopServices::openUrl(QUrl("https://www.pgxplorer.com/buy"));
    return;
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
