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

#include "connectionproperties.h"

ConnectionProperties::ConnectionProperties(Database *database, MainWin *mainwin)
{
    setWindowFlags(Qt::FramelessWindowHint);
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
    button_box->setFont(font);
    connect(button_box, SIGNAL(accepted()), this, SLOT(okslot()));
    connect(button_box, SIGNAL(rejected()), this, SLOT(close()));

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
    form_layout->addRow(tr("Port"), password);
    form_layout->labelForField(password)->setFont(font);
    form_layout->addRow(button_box);
    setLayout(form_layout);

    QWidget::setTabOrder(server, db_name);
    QWidget::setTabOrder(db_name, port);
    QWidget::setTabOrder(port, username);
    QWidget::setTabOrder(username, password);

    QObject::connect(this, SIGNAL(oksignal(QString,qint32,QString,QString,QString)),
                     mainwin, SLOT(newDatabase(QString,qint32,QString,QString,QString)));

    if(!database->getDatabaseStatus()) {
        server->setText(database->getHost());
        db_name->setText(database->getName());
        port->setText(database->getPort());
        username->setText(database->getUser());
        password->setText(database->getPassword());
    }
    else {
        QSqlDatabase database_connection = QSqlDatabase::database(QString("base").append(QString::number(database->getId())));
        server->setText(database_connection.hostName());
        db_name->setText(database->getName());
        port->setText(QString::number(database_connection.port()));
        username->setText(database_connection.userName());
        password->setText(database_connection.password());
    }
}

