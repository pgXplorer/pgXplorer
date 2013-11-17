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

#include "statusview.h"

StatusView::StatusView(Database *database)
{
    menuBar()->setVisible(false);

    setWindowTitle(tr("Status - ").append(database->getName()));
    setObjectName("Status");
    setContextMenuPolicy(Qt::NoContextMenu);

    this->database = database;

    createActions();

    QSettings settings("pgXplorer", "pgXplorer");
    ti = static_cast<StatusView::TimerInterval> (settings.value("interval", StatusView::NoInterval).toInt());
    if(timerInterval() == StatusView::TimerInterval1)
        setTimerInterval1();
    else if(timerInterval() == StatusView::TimerInterval2)
        setTimerInterval2();
    else
        setNoInterval();

    toolbar = new ToolBar;
    toolbar->setIconSize(QSize(36,36));
    toolbar->setObjectName("statusview");
    toolbar->setMovable(false);
    toolbar->addAction(refresh_action);
    toolbar->addAction(stop_action);
    toolbar->addAction(terminate_action);
    toolbar->addSeparator();
    toolbar->addWidget(timer_button);
    toolbar->addSeparator();
    toolbar->addAction(copy_action);

    addToolBar(toolbar);
    status_query_model = new QSqlQueryModel;
    status_view = new QTableView(this);
    status_view->setSelectionBehavior(QAbstractItemView::SelectRows);
    status_view->setSelectionMode(QAbstractItemView::SingleSelection);
    status_view->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    status_view->setModel(status_query_model);

    QString sql;
    if(database->settings("server_version_num").compare("90200") < 0) {
        sql = ("SELECT procpid AS \"PID\", usename AS \"User\", client_addr AS \"Client\", query_start AS \"Started\", current_query AS \"Query\" FROM pg_stat_activity where current_query not like '%pg_stat_activity%' and datname='");
    }
    else {
        sql = ("SELECT pid AS \"PID\", usename AS \"User\", client_addr AS \"Client\", query_start AS \"Started\", query AS \"Query\" FROM pg_stat_activity where query not like '%pg_stat_activity%' and state<>'idle' and datname='");
    }

    sql.append(database->getName() + "'");
    status_query_thread = new SimpleQueryThread(0, status_query_model, sql);
    status_query_thread->setConnectionParameters(database->getHost(),
                                                  database->getPort(),
                                                  database->getName(),
                                                  database->getUser(),
                                                  database->getPassword());
    status_query_thread->setExecOnStart(true);
    connect(this, &StatusView::requestStatus, status_query_thread, &SimpleQueryThread::executeDefaultQuery);
    connect(status_query_thread, &SimpleQueryThread::workerIsDone, this, &StatusView::updateStatus);
    status_query_thread->start();

    connect(status_view->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), SLOT(toggleActions()));

    setCentralWidget(status_view);
    statusBar()->showMessage("");

    if(timer_interval>0)
        timer.start(timer_interval, this);
}

void StatusView::setNoInterval()
{
    timer_interval = 0;
    no_refresh->setIconVisibleInMenu(true);
    refresh_freq_1->setIconVisibleInMenu(false);
    refresh_freq_2->setIconVisibleInMenu(false);
    setTimerInterval(StatusView::NoInterval);
}

void StatusView::setTimerInterval1()
{
    timer_interval = 10000;
    no_refresh->setIconVisibleInMenu(false);
    refresh_freq_1->setIconVisibleInMenu(true);
    refresh_freq_2->setIconVisibleInMenu(false);
    setTimerInterval(StatusView::TimerInterval1);
}

void StatusView::setTimerInterval2()
{
    timer_interval = 20000;
    no_refresh->setIconVisibleInMenu(false);
    refresh_freq_1->setIconVisibleInMenu(false);
    refresh_freq_2->setIconVisibleInMenu(true);
    setTimerInterval(StatusView::TimerInterval2);
}

void StatusView::updateStatus()
{
    if(!timer.isActive() && timer_interval>0)
        timer.start(timer_interval, this);

    status_view->setModel(status_query_model);
    status_view->resizeColumnsToContents();
    toggleActions();
}

void StatusView::createActions()
{
    refresh_action = new QAction(QIcon(":/icons/refresh.svg"), tr("Refresh"), this);
    refresh_action->setShortcut(QKeySequence::Refresh);
    refresh_action->setStatusTip(tr("Refresh status now"));
    connect(refresh_action, SIGNAL(triggered()), SIGNAL(requestStatus()));

    stop_action = new QAction(QIcon(":/icons/stop.svg"), tr("Stop"), this);
    stop_action->setEnabled(false);
    stop_action->setStatusTip(tr("Cancel selected query"));
    connect(stop_action, SIGNAL(triggered()), SLOT(stopQuery()));

    terminate_action = new QAction(QIcon(":/icons/terminate.svg"), tr("Terminate"), this);
    terminate_action->setEnabled(false);
    terminate_action->setStatusTip(tr("Terminate selected query"));
    connect(terminate_action, SIGNAL(triggered()), SLOT(terminateQuery()));

    copy_action = new QAction(QIcon(":/icons/copy.svg"), tr("Copy"), this);
    copy_action->setEnabled(false);
    copy_action->setStatusTip(tr("Copy selected contents to clipboard"));
    connect(copy_action, SIGNAL(triggered()), SLOT(copyQuery()));

    no_refresh = new QAction(QIcon(":/icons/ok.png"), tr("Don't refresh"), this);
    connect(no_refresh, SIGNAL(triggered()), SLOT(dontRefresh()));
    if(timerInterval() != StatusView::NoInterval)
        no_refresh->setIconVisibleInMenu(false);

    refresh_freq_1 = new QAction(QIcon(":/icons/ok.png"), tr("Refresh every 10s"), this);
    connect(refresh_freq_1, SIGNAL(triggered()), SLOT(refreshFrequency1()));
    if(timerInterval() != StatusView::TimerInterval1)
        refresh_freq_1->setIconVisibleInMenu(false);

    refresh_freq_2 = new QAction(QIcon(":/icons/ok.png"), tr("Refresh every 20s"), this);
    connect(refresh_freq_2, SIGNAL(triggered()), SLOT(refreshFrequency2()));
    if(timerInterval() != StatusView::TimerInterval2)
        refresh_freq_2->setIconVisibleInMenu(false);

    timer_menu = new QMenu;
    timer_menu->addAction(no_refresh);
    timer_menu->addAction(refresh_freq_1);
    timer_menu->addAction(refresh_freq_2);

    timer_action = new QAction(QIcon(":/icons/timer.svg"), tr("Set interval"), this);
    //timer_action->setMenu(timer_menu);
    timer_action->setStatusTip(tr("Set refresh interval (in seconds)"));

    timer_button = new QToolButton;
    timer_button->setAutoRaise(false);
    timer_button->setPopupMode(QToolButton::InstantPopup);
    timer_button->setDefaultAction(timer_action);
    timer_button->setMenu(timer_menu);
}

void StatusView::toggleActions()
{
    stop_action->setEnabled(false);
    terminate_action->setEnabled(false);
    copy_action->setEnabled(false);

    QItemSelectionModel *selection_model = status_view->selectionModel();
    QModelIndexList indices = selection_model->selectedIndexes();

    if(indices.isEmpty())
        return;
    if(indices.first().row() == indices.last().row()) {
        if(status_view->model()->data(indices.last()).toString().compare("<IDLE>") != 0) {
            stop_action->setEnabled(true);
        }
        terminate_action->setEnabled(true);
        copy_action->setEnabled(true);
    }
}

void StatusView::stopQuery()
{
    int ret = QMessageBox::question(this, "pgXplorer",
                                    tr("Do you want to cancel this query?"),
                                    QMessageBox::Yes | QMessageBox::No,
                                    QMessageBox::Yes);
    if(ret == QMessageBox::No)
        return;

    QItemSelectionModel *selection_model = status_view->selectionModel();
    QModelIndexList indices = selection_model->selectedIndexes();

    if(indices.isEmpty())
        return;
    if(indices.first().row() == indices.last().row()) {
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

            QString sql("SELECT pg_cancel_backend(" + status_view->model()->data(indices.first()).toString() + ")");
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
    }
    emit requestStatus();
}

void StatusView::terminateQuery()
{
    int ret = QMessageBox::question(this, "pgXplorer",
                                    tr("Do you want to terminate this query?"),
                                    QMessageBox::Yes | QMessageBox::No,
                                    QMessageBox::Yes);
    if(ret == QMessageBox::No)
        return;

    QItemSelectionModel *selection_model = status_view->selectionModel();
    QModelIndexList indices = selection_model->selectedIndexes();

    if(indices.isEmpty())
        return;
    if(indices.first().row() == indices.last().row()) {
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

            QString sql("SELECT pg_terminate_backend(" + status_view->model()->data(indices.first()).toString() + ")");
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
    emit requestStatus();
}

void StatusView::copyQuery()
{

    QItemSelectionModel *selection_model = status_view->selectionModel();
    QModelIndexList indices = selection_model->selectedIndexes();

    if(indices.isEmpty())
        return;

    qSort(indices);
    QModelIndex prev = indices.first();
    QModelIndex last = indices.last();
    indices.removeFirst();
    QModelIndex current;
    QString selectedText;
    foreach(current, indices) {
        QVariant data = status_view->model()->data(prev);
        selectedText.append(data.toString());
        if(current.row() != prev.row())
            selectedText.append(QLatin1Char('\n'));
        else
            selectedText.append(QLatin1Char('\t'));
        prev = current;
    }
    selectedText.append(status_view->model()->data(last).toString());
    selectedText.append(QLatin1Char('\n'));
    qApp->clipboard()->setText(selectedText);
}

void StatusView::dontRefresh()
{
    setNoInterval();
    if(timer.isActive())
        timer.stop();
}

void StatusView::refreshFrequency1()
{
    setTimerInterval1();
    if(timer.isActive())
        timer.stop();
    timer.start(timer_interval, this);
}

void StatusView::refreshFrequency2()
{
    setTimerInterval2();
    if(timer.isActive())
        timer.stop();
    timer.start(timer_interval, this);
}

void StatusView::bringOnTop()
{
    activateWindow();
    raise();
}

void StatusView::timerEvent(QTimerEvent *event)
{
    if(timer.timerId() == event->timerId()) {
        if(isVisible()) {
            emit requestStatus();
            statusBar()->showMessage(tr("Refreshed just now"), 1000);
        }
        else
            timer.stop();
    }
}

void StatusView::closeEvent(QCloseEvent *event)
{
    event->accept();

    QSettings settings("pgXplorer", "pgXplorer");
    if(isMaximized()) {
        settings.setValue("statusview_maximized", true);
        showNormal();
    }
    else
        settings.setValue("statusview_maximized", false);

    settings.setValue("statusview_pos", pos());
    settings.setValue("statusview_size", size());
    settings.setValue("icon_size", toolbar->iconSize());
    settings.setValue("interval", (int) ti);
}

StatusView::TimerInterval StatusView::timerInterval() const
{
    return ti;
}

void StatusView::setTimerInterval(TimerInterval timer_interval)
{
    ti = timer_interval;
}

void StatusView::languageChanged(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        setWindowTitle(tr("Status - ").append(database->getName()));

        refresh_action->setText(tr("Refresh"));
        stop_action->setText(tr("Stop"));
        terminate_action->setText(tr("Terminate"));
        copy_action->setText(tr("Copy"));
        timer_action->setText(tr("Set interval"));
        no_refresh->setText(tr("Don't refresh"));
        refresh_freq_1->setText(tr("Refresh every 10s"));
        refresh_freq_2->setText(tr("Refresh every 20s"));

        refresh_action->setStatusTip(tr("Refresh status now"));
        stop_action->setStatusTip(tr("Cancel selected query"));
        terminate_action->setStatusTip(tr("Terminate selected query"));
        copy_action->setStatusTip(tr("Copy selected contents to clipboard"));
        timer_action->setStatusTip(tr("Set refresh interval (in seconds)"));
        no_refresh->setStatusTip(tr("Don't refresh"));
        refresh_freq_1->setStatusTip(tr("Refresh every 10s"));
        refresh_freq_2->setStatusTip(tr("Refresh every 20s"));
    }
}
