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

#include "connectionproperties.h"

ConnectionProperties::ConnectionProperties(Database *database, MainWin *mainwin)
{
    setParent(mainwin);
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_DeleteOnClose);
    setModal(true);
    setWindowModality(Qt::ApplicationModal);
    setWindowTitle(tr("Connection"));

    QFont title_font("Helvetica [Cronyx]", 14, QFont::Bold);
    QFont font("Helvetica [Cronyx]");

    title = new QLabel(this);
    title->setAlignment(Qt::AlignHCenter);
    title->setText(tr("Connection"));
    title->setFont(title_font);

    button_box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    button_box->setCenterButtons(true);
    connect(button_box, &QDialogButtonBox::accepted, this, &ConnectionProperties::okslot);
    connect(button_box, &QDialogButtonBox::rejected, this, &ConnectionProperties::close);

    server = new QLineEdit(this);
    server->setFont(font);

    db_name = new QLineEdit(this);
    db_name->setFont(font);
    db_name->setFocus();

    port = new QLineEdit(this);
    port->setFont(font);
    port->setInputMethodHints(Qt::ImhDigitsOnly);

    username = new QLineEdit(this);
    username->setFont(font);

    password = new QLineEdit(this);
    password->setEchoMode(QLineEdit::Password);
    password->setFont(font);

    QFormLayout *form_layout = new QFormLayout;
    form_layout->setSpacing(20);
    form_layout->addRow(title);
    form_layout->addRow(tr("Server"), server);
    form_layout->labelForField(server)->setFont(font);
    form_layout->addRow(tr("Database"), db_name);
    form_layout->labelForField(db_name)->setFont(font);
    form_layout->addRow(tr("Port"), port);
    form_layout->labelForField(port)->setFont(font);
    form_layout->addRow(tr("Username"), username);
    form_layout->labelForField(username)->setFont(font);
    form_layout->addRow(tr("Password"), password);
    form_layout->labelForField(password)->setFont(font);
    form_layout->addRow(button_box);
    setLayout(form_layout);

    connect(this, &ConnectionProperties::oksignal,
                     mainwin, &MainWin::newDatabaseWithParams);

    server->setText(database->getHost());
    db_name->setText(database->getName());
    port->setText(database->getPort());
    username->setText(database->getUser());
    password->setText(database->getPassword());

#ifdef Q_WS_X11
    move(mainwin->x()+mainwin->width()/2-sizeHint().width()/2, mainwin->y()+mainwin->height()/2-sizeHint().height()/2);
#endif
}

