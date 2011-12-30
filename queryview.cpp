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

#include "queryview.h"

ulong QueryView::queryViewObjectId = 0;
QueryView::QueryView(Database *database, QString command)
{
    this->database = database;

    //Identify this object with thisTableViewId for constructing database connection
    //specific to this object and this object alone.
    thisQueryViewId = queryViewObjectId++;

    //Thread busy indicator to avoid crashing of app when
    //QueryView receives a close signal before data retrieval.
    thread_busy = false;

    //Start the timer.
    t.start();

    errors_model = new QStandardItemModel(0,1);
    query_model = new QueryModel;
    qview = new QTableView(this);
    qview->resizeColumnsToContents();
    setCentralWidget(qview);
    statusBar()->showMessage(QApplication::translate("QueryView", "Fetching data...", 0, QApplication::UnicodeUTF8));
    //qview->setModel(model);
    //setWindowTitle(name);
    qview->setStyleSheet("QTableView {font-weight: 400;}");
    qview->setAlternatingRowColors(true);
    //setGeometry(100,100,640,480);

    //Create Ctrl+Shift+C key combo to copy selected table contents with headers.
    QShortcut *shortcut_ctrl_c = new QShortcut(QKeySequence::Copy, this);
    connect(shortcut_ctrl_c, SIGNAL(activated()), this, SLOT(copyc()));

    //Create Ctrl+Shift+C key combo to copy selected table contents with headers.
    QShortcut *shortcut_ctrl_shft_c = new QShortcut(QKeySequence("Ctrl+Shift+C"), this);
    connect(shortcut_ctrl_shft_c, SIGNAL(activated()), this, SLOT(copych()));

    //Create key-sequences for fullscreen and restore.
    shortcut_fullscreen = new QShortcut(QKeySequence(Qt::Key_F11), this);
    connect(shortcut_fullscreen, SIGNAL(activated()), this, SLOT(fullscreen()));
    shortcut_restore = new QShortcut(QKeySequence(Qt::Key_Escape), this);
    connect(shortcut_restore, SIGNAL(activated()), this, SLOT(restore()));

    //Tie thread finish to an update slot that refreshes meta-information.
    connect(this, SIGNAL(updRowCntSignal(QString)), this, SLOT(updRowCntSlot(QString)));

    //Tie a busy signal to a slot that changes the cursor to wait cursor.
    connect(this, SIGNAL(busySignal()), this, SLOT(busySlot()));

    QFuture<void> future = QtConcurrent::run(this, &QueryView::fetchData, command);
}

void QueryView::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    QMenu menu;

    QPalette palette;
    palette.setColor(menu.backgroundRole(), QColor(205,205,205));
    menu.setPalette(palette);

    menu.addAction("Remove");
    QAction *a = menu.exec(event->screenPos());
    if(a && QString::compare(a->text(),"Remove")==0) {

    }
    if(a && QString::compare(a->text(),"Expand")==0) {

    }
    if(a && QString::compare(a->text(),"Collapse")==0) {

    }
}

void QueryView::closeEvent(QCloseEvent *event)
{
    //Clean-up only when there is no active thread.
    //However, this will cause a memory leak when the
    //QueryView is closed when the thread is busy.
    //Proper solution is to create a Thread class
    //and cancel that before we clean-up. We cannot do
    //this now because we are using QFuture (per Qt docs).
    emit queryViewClosing(this);

    QSettings settings("pgXplorer", "pgXplorer");
    if(isMaximized()) {
        settings.setValue("queryview_maximized", true);
        showNormal();
    }
    else
        settings.setValue("queryview_maximized", false);
    settings.setValue("queryview_pos", pos());
    settings.setValue("queryview_size", size());

    if(!thread_busy)
    {
        delete qview;
        delete errors_model;
        delete query_model;
        delete shortcut_fullscreen;
        delete shortcut_restore;
        QSqlDatabase::removeDatabase("queryview" + QString::number(thisQueryViewId));
        close();
    }
}

void QueryView::copyc()
{
    QItemSelectionModel *s = qview->selectionModel();
    QModelIndexList indices = s->selectedIndexes();
    if(indices.isEmpty()) {
        return;
    }
    qSort(indices);
    QModelIndex prev = indices.first();
    QModelIndex last = indices.last();
    indices.removeFirst();
    QModelIndex current;
    QString selectedText;

    foreach(current, indices) {
        QVariant data = qview->model()->data(prev);
        selectedText.append(data.toString());
        if(current.row() != prev.row())
            selectedText.append(QLatin1Char('\n'));
        else
            selectedText.append(QLatin1Char('\t'));
        prev = current;
    }
    selectedText.append(qview->model()->data(last).toString());
    selectedText.append(QLatin1Char('\n'));
    qApp->clipboard()->setText(selectedText);
}

void QueryView::copych()
{
    QAbstractItemModel *atm = qview->model();
    QItemSelectionModel *s = qview->selectionModel();
    QModelIndexList indices = s->selectedIndexes();
    if(indices.isEmpty())
        return;
    qSort(indices);
    QString headerText;
    QModelIndex current;
    int prevRow = indices.at(0).row();
    foreach(current, indices) {
        if(current.row() == prevRow) {
            QVariant data = atm->headerData(current.column(), Qt::Horizontal);
            headerText.append(data.toString());
            headerText.append(QLatin1Char('\t'));
        }
        else {
            headerText.append(QLatin1Char('\n'));
            break;
        }
        prevRow = current.row();
    }
    if(!headerText.endsWith("\n"))
        headerText.append(QLatin1Char('\n'));
    QString selectedText;
    QModelIndex prev = indices.first();
    QModelIndex last = indices.last();
    indices.removeFirst();
    foreach(current, indices) {
        QVariant data = atm->data(prev);
        selectedText.append(data.toString());
        if(current.row() != prev.row())
            selectedText.append(QLatin1Char('\n'));
        else
            selectedText.append(QLatin1Char('\t'));
        prev = current;
    }
    selectedText.append(atm->data(last).toString());
    selectedText.append(QLatin1Char('\n'));
    qApp->clipboard()->setText(headerText + selectedText);
}

void QueryView::busySlot()
{
    thread_busy = true;
    setCursor(Qt::WaitCursor);
}

void QueryView::fullscreen()
{
    this->showFullScreen();
}

void QueryView::restore()
{
    this->showNormal();
}

void QueryView::fetchData(QString command)
{
    emit busySignal();
    QTime ts;
    ts.start();
    QSqlDatabase::removeDatabase("queryview" + QString::number(thisQueryViewId));
    QSqlDatabase database_connection;
    database_connection = QSqlDatabase::addDatabase("QPSQL", "queryview" + QString::number(thisQueryViewId));
    database_connection.setHostName(database->getHost());
    database_connection.setPort(database->getPort().toInt());
    database_connection.setDatabaseName(database->getName());
    database_connection.setUserName(database->getUser());
    database_connection.setPassword(database->getPassword());
    if (!database_connection.open()) {
        emit updRowCntSignal(tr("Couldn't connect to database.\n"
                                      "Check connection parameters.\n"));
    }
    else {
        query_model->setQuery(command, database_connection);
        if(query_model->lastError().isValid())
            emit updRowCntSignal(query_model->lastError().databaseText());
        else
            emit updRowCntSignal(QString());
    }
}

void QueryView::updRowCntSlot(QString error)
{
    QString time_elapsed = QApplication::translate("QueryView", "Time elapsed:", 0, QApplication::UnicodeUTF8);
    QString rows_string = QApplication::translate("QueryView", "Rows:", 0, QApplication::UnicodeUTF8);
    QString colums_string = QApplication::translate("QueryView", "Columns:", 0, QApplication::UnicodeUTF8);
    QString seconds_string = QApplication::translate("QueryView", "s", 0, QApplication::UnicodeUTF8);

    if(!error.isEmpty()) {
        errors_model->clear();
        QStringList messages = error.split("\n");
        messages.removeLast();
        foreach(const QString m, messages)
            errors_model->appendRow(new QStandardItem(m));

        QFont serifFont("Courier", 10, QFont::Bold);
        qview->setFont(serifFont);
        qview->setModel(errors_model);
        qview->resizeColumnToContents(0);
        QStringList header;
        header << tr("Error messages");
        errors_model->setHorizontalHeaderLabels(header);
        statusBar()->showMessage(time_elapsed + QString::number((double)t.elapsed()/1000) +
                                 " " + seconds_string + " \t " + rows_string + QString::number(errors_model->rowCount()) +
                                 " \t " + colums_string + "1");
    }
    else if(!query_model->query().isSelect()) {
        QStandardItemModel *model = new QStandardItemModel(0,1);
        QString message_string = QString::number(query_model->query().numRowsAffected());
        model->appendRow(new QStandardItem(message_string));

        QFont serifFont("Courier", 10, QFont::Bold);
        qview->setFont(serifFont);
        qview->setModel(model);
        QStringList header;
        header << tr("Affected rows");
        model->setHorizontalHeaderLabels(header);
        qview->resizeColumnToContents(0);
        qview->setEditTriggers(QAbstractItemView::NoEditTriggers);
        statusBar()->showMessage(time_elapsed + QString::number((double)t.elapsed()/1000) +
                                 " " + seconds_string + " \t " + rows_string + "1" +
                                 " \t " + colums_string + "1");
    }
    else {
        qview->setModel(query_model);
        qview->verticalScrollBar()->setValue(0);
        if(query_model->rowCount() == 0)
            statusBar()->showMessage(time_elapsed + QString::number((double)t.elapsed()/1000) +
                                     " " + seconds_string + " \t " + rows_string + "0" +
                                     " \t " + colums_string + QString::number(query_model->columnCount()));
        else
            statusBar()->showMessage(time_elapsed + QString::number((double)t.elapsed()/1000) +
                                     " " + seconds_string + " \t " + rows_string + QString::number(query_model->rowCount()) +
                                     " \t " + colums_string + QString::number(query_model->columnCount()));
    }
    setCursor(Qt::ArrowCursor);
    thread_busy = false;
}
