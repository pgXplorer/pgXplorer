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

#include "queryview.h"
#include "graphwindow.h"
#include "reportwindow.h"

ulong QueryView::queryViewObjectId = 0;

QueryView::QueryView(Database *database, QString command)
{
    setAttribute(Qt::WA_DeleteOnClose);
    this->database = database;
    this->command = command;

    //Identify this object with thisTableViewId for constructing database connection
    //specific to this object and this object alone.
    thisQueryViewId = queryViewObjectId++;

    //Thread busy indicator to avoid crashing of app when
    //QueryView receives a close signal before data retrieval.
    thread_busy = false;

    //Start the timer.
    t.start();

    createActions();

    toolbar = new ToolBar;
    toolbar->setIconSize(QSize(36,36));
    toolbar->setObjectName("queryview");
    toolbar->setMovable(false);
    toolbar->addAction(stop_action);
    //toolbar->addAction(terminate_action);
    toolbar->addAction(copy_action);
    toolbar->addAction(copy_with_headers_action);
    toolbar->addSeparator();
    toolbar->addAction(scatterplot_action);
    toolbar->addAction(lineplot_action);
    toolbar->addAction(barplot_action);
    toolbar->addAction(areaplot_action);
    toolbar->addSeparator();
    toolbar->addAction(report_action);
    addToolBar(toolbar);

    errors_model = new QStandardItemModel(0,1);
    query_model = new QueryModel;
    query_view = new QTableView(this);
    query_view->setWordWrap(false);
    query_view->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    query_view->resizeColumnsToContents();
    setCentralWidget(query_view);
    statusBar()->showMessage(QApplication::translate("QueryView", "Fetching data...", 0));
    query_view->setAlternatingRowColors(true);
    query_view->verticalHeader()->setDefaultAlignment(Qt::AlignRight | Qt::AlignVCenter);

    //Create key-sequences for fullscreen and restore.
    shortcut_fullscreen = new QShortcut(QKeySequence(Qt::Key_F11), this);
    connect(shortcut_fullscreen, &QShortcut::activated, this, &QueryView::fullscreen);
    shortcut_restore = new QShortcut(QKeySequence(Qt::Key_Escape), this);
    connect(shortcut_restore, &QShortcut::activated, this, &QueryView::restore);

    //Tie a busy signal to a slot that changes the cursor to wait cursor.
    connect(this, &QueryView::busySignal, this, &QueryView::busySlot);

    //Create and start a query thread.
    simple_query_thread = new SimpleQueryThread(0, query_model, command);
    simple_query_thread->setConnectionParameters(database->getHost(),
                                                  database->getPort(),
                                                  database->getName(),
                                                  database->getUser(),
                                                  database->getPassword());
    simple_query_thread->setExecOnStart(true);
    connect(this, &QueryView::cancelQuery, simple_query_thread, &SimpleQueryThread::stopQuery);
    connect(simple_query_thread, &SimpleQueryThread::workerIsDone, this, &QueryView::updRowCntSlot);
    simple_query_thread->start(QThread::TimeCriticalPriority);
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
    else {
        settings.setValue("queryview_maximized", false);
    }
    settings.setValue("queryview_pos", pos());
    settings.setValue("queryview_size", size());
    settings.setValue("icon_size", toolbar->iconSize());

    emit cancelQuery();

    simple_query_thread->quit();
    simple_query_thread->wait();

    QSqlDatabase::removeDatabase("queryview " + QString::number(thisQueryViewId));
    QMainWindow::closeEvent(event);
}

QueryView::~QueryView()
{
    delete simple_query_thread;
    delete query_view;
    delete errors_model;
    delete query_model;
    delete shortcut_fullscreen;
    delete shortcut_restore;
}

void QueryView::createActions()
{
    stop_action = new QAction(QIcon(":/icons/stop.svg"), tr("Stop"), this);
    stop_action->setStatusTip(tr("Cancel selected query"));
    connect(stop_action, &QAction::triggered, this , &QueryView::stopQuery);

    terminate_action = new QAction(QIcon(":/icons/terminate.svg"), tr("Terminate"), this);
    terminate_action->setStatusTip(tr("Terminate selected query"));
    connect(terminate_action, &QAction::triggered, this , &QueryView::terminateQuery);

    copy_action = new QAction(QIcon(":/icons/copy_without_headers.svg"), tr("Copy"), this);
    copy_action->setShortcuts(QKeySequence::Copy);
    copy_action->setStatusTip(tr("Copy selected"));
    copy_action->setEnabled(false);
    connect(copy_action, &QAction::triggered, this , &QueryView::copyToClipboard);

    copy_with_headers_action = new QAction(QIcon(":/icons/copy_with_headers.svg"), tr("Copy with headers"), this);
    copy_with_headers_action->setShortcut(QKeySequence("Ctrl+Shift+C"));
    copy_with_headers_action->setStatusTip(tr("Copy selected with headers"));
    copy_with_headers_action->setEnabled(false);
    connect(copy_with_headers_action, &QAction::triggered, this , &QueryView::copyToClipboardWithHeaders);

    scatterplot_action = new QAction(QIcon(":/icons/scatter.svg"), tr("Scatter plot"), this);
    scatterplot_action->setEnabled(false);
    scatterplot_action->setStatusTip(tr("Plot the selected columns as a scatter plot"));
    connect(scatterplot_action, &QAction::triggered, this , &QueryView::scatterPlot);

    lineplot_action = new QAction(QIcon(":/icons/line.svg"), tr("Line plot"), this);
    lineplot_action->setEnabled(false);
    lineplot_action->setStatusTip(tr("Plot the selected columns as a line plot"));
    connect(lineplot_action, &QAction::triggered, this , &QueryView::linePlot);

    barplot_action = new QAction(QIcon(":/icons/bar.svg"), tr("Bar plot"), this);
    barplot_action->setEnabled(false);
    barplot_action->setStatusTip(tr("Plot the selected columns as a bar plot"));
    connect(barplot_action, &QAction::triggered, this , &QueryView::barPlot);

    areaplot_action = new QAction(QIcon(":/icons/area.svg"), tr("Area plot"), this);
    areaplot_action->setEnabled(false);
    areaplot_action->setStatusTip(tr("Plot the selected columns as an area plot"));
    connect(areaplot_action, &QAction::triggered, this , &QueryView::areaPlot);

    report_action = new QAction(QIcon(":/icons/report.svg"), tr("Report"), this);
    report_action->setEnabled(false);
    report_action->setStatusTip(tr("Create a report based on this data"));
    connect(report_action, &QAction::triggered, this , &QueryView::createReport);
}

void QueryView::copyToClipboard()
{
    QItemSelectionModel *selection_model = query_view->selectionModel();
    QModelIndexList indices = selection_model->selectedIndexes();
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
        QVariant data = query_view->model()->data(prev);
        selectedText.append(data.toString());
        if(current.row() != prev.row())
            selectedText.append(QLatin1Char('\n'));
        else
            selectedText.append(QLatin1Char('\t'));
        prev = current;
    }
    selectedText.append(query_view->model()->data(last).toString());
    selectedText.append(QLatin1Char('\n'));
    qApp->clipboard()->setText(selectedText);
}

void QueryView::copyToClipboardWithHeaders()
{
    QAbstractItemModel *model = query_view->model();
    QItemSelectionModel *selection_model = query_view->selectionModel();
    QModelIndexList indices = selection_model->selectedIndexes();
    if(indices.isEmpty())
        return;
    qSort(indices);
    QString headerText;
    QModelIndex current;
    int prevRow = indices.at(0).row();
    foreach(current, indices) {
        if(current.row() == prevRow) {
            QVariant data = model->headerData(current.column(), Qt::Horizontal);
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
        QVariant data = model->data(prev);
        selectedText.append(data.toString());
        if(current.row() != prev.row())
            selectedText.append(QLatin1Char('\n'));
        else
            selectedText.append(QLatin1Char('\t'));
        prev = current;
    }
    selectedText.append(model->data(last).toString());
    selectedText.append(QLatin1Char('\n'));
    qApp->clipboard()->setText(headerText + selectedText);
}

void QueryView::busySlot()
{
    thread_busy = true;
    setCursor(Qt::WaitCursor);
    t.start();
    query_view->horizontalHeader()->setStretchLastSection(false);
    disableActions();
}

void QueryView::notBusy()
{
    enableActions();
    setCursor(Qt::ArrowCursor);
    thread_busy = false;
}

void QueryView::enableActions()
{
    stop_action->setEnabled(false);
    terminate_action->setEnabled(false);
    copy_action->setEnabled(true);
    copy_with_headers_action->setEnabled(true);
}

void QueryView::disableActions()
{
    stop_action->setEnabled(true);
    terminate_action->setEnabled(true);
    copy_action->setEnabled(false);
    copy_with_headers_action->setEnabled(false);
}

void QueryView::hideChartAndReport()
{
    scatterplot_action->setVisible(false);
    lineplot_action->setVisible(false);
    barplot_action->setVisible(false);
    areaplot_action->setVisible(false);
    report_action->setVisible(false);
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

    QSqlDatabase::removeDatabase("queryview " + QString::number(thisQueryViewId));
    QSqlDatabase database_connection;
    database_connection = QSqlDatabase::addDatabase("QPSQL", "queryview " + QString::number(thisQueryViewId));
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

    QSqlQueryModel rollback_query_model;
    rollback_query_model.setQuery("; ROLLBACK; ", database_connection);
}

void QueryView::bringOnTop()
{
    activateWindow();
    raise();
}

void QueryView::updRowCntSlot(QString status, QString error)
{
    time_elapsed_string = QApplication::translate("QueryView", "Time elapsed:", 0);
    rows_string = QApplication::translate("QueryView", "Rows:", 0);
    colums_string = QApplication::translate("QueryView", "Columns:", 0);
    seconds_string = QApplication::translate("QueryView", "s", 0);

    time_elapsed = (double)t.elapsed()/1000;

    if(!error.isEmpty()) {
        errors_model->clear();
        QStringList messages = error.split("\n");
        messages.removeLast();
        foreach(const QString m, messages)
            errors_model->appendRow(new QStandardItem(m));

        QFont courier("Courier", 10, QFont::Bold);
        query_view->setFont(courier);
        query_view->setModel(errors_model);
        query_view->resizeColumnToContents(0);
        query_view->horizontalHeader()->setStretchLastSection(true);

        QStringList header;
        header << tr("Error messages");
        errors_model->setHorizontalHeaderLabels(header);
        statusBar()->showMessage(time_elapsed_string + QString::number(time_elapsed) +
                                 " " + seconds_string + " \t " + rows_string + QString::number(errors_model->rowCount()) +
                                 " \t " + colums_string + "1");
    }
    else if(!query_model->query().isSelect()) {
        QStandardItemModel *model = new QStandardItemModel(0,1);
        QString message_string = QString::number(query_model->query().numRowsAffected());
        model->appendRow(new QStandardItem(message_string));

        QFont courier("Courier", 10, QFont::Bold);
        query_view->setFont(courier);
        query_view->setModel(model);
        QStringList header;
        header << tr("Affected rows");
        model->setHorizontalHeaderLabels(header);
        query_view->resizeColumnToContents(0);
        query_view->setEditTriggers(QAbstractItemView::NoEditTriggers);
        statusBar()->showMessage(time_elapsed_string + QString::number(time_elapsed) +
                                 " " + seconds_string + " \t " + rows_string + "1" +
                                 " \t " + colums_string + "1");
    }
    else {
        query_view->setModel(query_model);
        connect(query_view->selectionModel(), &QItemSelectionModel::selectionChanged, this, &QueryView::togglePlots);

        query_view->verticalScrollBar()->setValue(0);
        if(query_model->rowCount() == 0)
            statusBar()->showMessage(time_elapsed_string + QString::number(time_elapsed) +
                                     " " + seconds_string + " \t " + rows_string + "0" +
                                     " \t " + colums_string + QString::number(query_model->columnCount()));
        else
            statusBar()->showMessage(time_elapsed_string + QString::number(time_elapsed) +
                                     " " + seconds_string + " \t " + rows_string + QString::number(query_model->rowCount()) +
                                     " \t " + colums_string + QString::number(query_model->columnCount()));
        report_action->setEnabled(true);
    }
    notBusy();
}

void QueryView::togglePlots()
{
    QItemSelectionModel *selection_model = query_view->selectionModel();
    QModelIndexList indices = selection_model->selectedIndexes();

    if(indices.isEmpty())
        return;
    qSort(indices);

    int row_count = 0;
    int column_count = 0;

    int first_row = indices.first().row();
    int first_column = indices.first().column();
    int last_row = indices.last().row();
    int row = indices.first().row();
    int column = indices.first().column();
    foreach(QModelIndex index, indices) {
        if(index.row() == first_row)
            column_count++;
        if(index.column() == first_column)
            row_count++;
        column = index.column();
        row = index.row();
    }
    if((column_count == 2) && (row_count - (last_row-first_row+1) == 0)) {
        scatterplot_action->setEnabled(true);
        lineplot_action->setEnabled(true);
        barplot_action->setEnabled(true);
        areaplot_action->setEnabled(true);
    }
    else {
        scatterplot_action->setEnabled(false);
        lineplot_action->setEnabled(false);
        barplot_action->setEnabled(false);
        areaplot_action->setEnabled(false);
    }
}

void QueryView::languageChanged(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        time_elapsed_string = QApplication::translate("QueryView", "Time elapsed:", 0);
        rows_string = QApplication::translate("QueryView", "Rows:", 0);
        colums_string = QApplication::translate("QueryView", "Columns:", 0);
        seconds_string = QApplication::translate("QueryView", "s", 0);

        if(query_view->model()->rowCount() == 0)
            statusBar()->showMessage(time_elapsed_string + QString::number(time_elapsed) +
                                     " " + seconds_string + " \t " + rows_string + "0" +
                                     " \t " + colums_string + QString::number(query_model->columnCount()));
        else
            statusBar()->showMessage(time_elapsed_string + QString::number(time_elapsed) +
                                     " " + seconds_string + " \t " + rows_string + QString::number(query_model->rowCount()) +
                                     " \t " + colums_string + QString::number(query_model->columnCount()));

        copy_action->setText(tr("Copy"));
        copy_action->setStatusTip(tr("Copy selected"));
        copy_with_headers_action->setText(tr("Copy with headers"));
        copy_with_headers_action->setStatusTip(tr("Copy selected with headers"));

        scatterplot_action->setText(tr("Scatter plot"));
        scatterplot_action->setStatusTip(tr("Plot the selected columns as a scatter plot"));
        lineplot_action->setText(tr("Line plot"));
        lineplot_action->setStatusTip(tr("Plot the selected columns as a line plot"));
        barplot_action->setText(tr("Bar plot"));
        barplot_action->setStatusTip(tr("Plot the selected columns as a bar plot"));
        areaplot_action->setText(tr("Area plot"));
        areaplot_action->setStatusTip(tr("Plot the selected columns as an area plot"));
    }
}

void QueryView::scatterPlot()
{
    QModelIndexList list = query_view->selectionModel()->selectedIndexes();
    if(list.isEmpty()) {
        statusBar()->showMessage(tr("Cannot plot without anything selected"));
        return;
    }
    qSort(list);
    GraphWindow *graph_win = new GraphWindow(list, GraphWindow::Scatter);
    graph_win->show();
}

void QueryView::linePlot()
{
    QModelIndexList list = query_view->selectionModel()->selectedIndexes();
    if(list.isEmpty()) {
        statusBar()->showMessage(tr("Cannot plot without anything selected"));
        return;
    }
    qSort(list);
    GraphWindow *graph_win = new GraphWindow(list, GraphWindow::Line);
    graph_win->show();
}

void QueryView::barPlot()
{
    QModelIndexList list = query_view->selectionModel()->selectedIndexes();
    if(list.isEmpty()) {
        statusBar()->showMessage(tr("Cannot plot without anything selected"));
        return;
    }
    qSort(list);
    GraphWindow *graph_win = new GraphWindow(list, GraphWindow::Bar);
    graph_win->show();
}

void QueryView::areaPlot()
{
    QModelIndexList list = query_view->selectionModel()->selectedIndexes();
    if(list.isEmpty()) {
        statusBar()->showMessage(tr("Cannot plot without anything selected"));
        return;
    }
    qSort(list);
    GraphWindow *graph_win = new GraphWindow(list, GraphWindow::Area);
    graph_win->show();
}

void QueryView::createReport()
{
    ReportWindow *report_win = new ReportWindow(database, command);
    QSettings settings("pgXplorer", "pgXplorer");
    QPoint pos = settings.value("reportwindow_pos", QPoint(100, 100)).toPoint();
    QSize size = settings.value("reportwindow_size", QSize(1024, 768)).toSize();
    QSize icon_size = settings.value("icon_size", QSize(36, 36)).toSize();

    report_win->resize(size);
    report_win->move(pos);
    report_win->getToolbar()->setIconSize(icon_size);
    report_win->show();
}

void QueryView::stopQuery()
{
    int ret = QMessageBox::question(this, "pgXplorer",
                                    tr("Do you want to cancel this query?"),
                                    QMessageBox::Yes | QMessageBox::No,
                                    QMessageBox::Yes);
    if(ret == QMessageBox::No)
        return;
    emit cancelQuery();
}

/*void QueryView::stopQuery()
{
    int ret = QMessageBox::question(this, "pgXplorer",
                                    tr("Do you want to cancel this query?"),
                                    QMessageBox::Yes | QMessageBox::No,
                                    QMessageBox::Yes);
    if(ret == QMessageBox::No)
        return;

    {
        QSqlDatabase database_connection;
        database_connection = QSqlDatabase::addDatabase("QPSQL", QLatin1String("cancel"));
        database_connection.setHostName(database->getHost());
        database_connection.setPort(database->getPort().toInt());
        database_connection.setDatabaseName(database->getName());
        database_connection.setUserName(database->getUser());
        database_connection.setPassword(database->getPassword());
        if (!database_connection.open()) {
            qDebug() << tr("Couldn't connect to database.\n"
                         "Check connection parameters.\n");
            return;
        }

        QString sql;
        if(database->settings("server_version_num").compare("90200") < 0) {
            sql = "SELECT pg_cancel_backend(procpid) FROM pg_stat_activity WHERE current_query LIKE '";
        }
        else
        {
            sql = "SELECT pg_cancel_backend(pid) FROM pg_stat_activity WHERE query LIKE '";
        }
        sql.append(command);
        sql.append("%'");
        QSqlQueryModel query_model;
        query_model.setQuery(sql, database_connection);

        if (query_model.lastError().isValid()) {
             qDebug() << query_model.lastError();
        }
        if(query_model.data(query_model.index(0, 0)).toBool())
            statusBar()->showMessage(tr("Query was successfully cancelled."), 5000);
        else
            statusBar()->showMessage(tr("Could not cancel the selected query. Try terminating it."), 5000);
    }
    QSqlDatabase::removeDatabase(QLatin1String("cancel"));
}*/

void QueryView::terminateQuery()
{
    int ret = QMessageBox::question(this, "pgXplorer",
                                    tr("Do you want to terminate this query?"),
                                    QMessageBox::Yes | QMessageBox::No,
                                    QMessageBox::Yes);
    if(ret == QMessageBox::No)
        return;

    {
        QSqlDatabase database_connection;
        database_connection = QSqlDatabase::addDatabase("QPSQL", QLatin1String("terminate"));
        database_connection.setHostName(database->getHost());
        database_connection.setPort(database->getPort().toInt());
        database_connection.setDatabaseName(database->getName());
        database_connection.setUserName(database->getUser());
        database_connection.setPassword(database->getPassword());
        if (!database_connection.open()) {
            qDebug() << tr("Couldn't connect to database.\n"
                           "Check connection parameters.\n");
            return;
        }

        QString sql;
        if(database->settings("server_version_num").compare("90200") < 0) {
            sql = "SELECT pg_terminate_backend(procpid) FROM pg_stat_activity WHERE current_query LIKE '";
        }
        else {
            sql = "SELECT pg_terminate_backend(pid) FROM pg_stat_activity WHERE query LIKE '";
        }

        sql.append(command);
        sql.append("%'");
        QSqlQueryModel query_model;
        query_model.setQuery(sql, database_connection);
        if (query_model.lastError().isValid()) {
             qDebug() << query_model.lastError();
        }
        if(query_model.data(query_model.index(0, 0)).toBool())
            statusBar()->showMessage(tr("Query was successfully terminated."), 5000);
        else
            statusBar()->showMessage(tr("Could not terminate the selected query. Contact database administrator."), 5000);
    }
    QSqlDatabase::removeDatabase(QLatin1String("terminate"));
}
