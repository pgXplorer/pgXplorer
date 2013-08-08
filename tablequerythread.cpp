#include "tablequerythread.h"

TableWorker::TableWorker(QObject *parent, QString connection_name,
               QSqlQueryModel *table_model,
               QString status,
               QString time,
               QString host,
               QString port,
               QString dbname,
               QString user,
               QString password)
    : QObject (parent)
{
    this->connection_name = connection_name;
    this->status = status;
    worker_started_at = time;

    QSqlDatabase database_connection = QSqlDatabase::addDatabase("QPSQL", connection_name + worker_started_at);
    this->table_model = table_model;

    database_connection.setHostName(host);
    database_connection.setPort(port.toInt());
    database_connection.setDatabaseName(dbname);
    database_connection.setUserName(user);
    database_connection.setPassword(password);

    database_connection.open();
    cancel = false;
}

bool TableWorker::cancelled()
{
   return cancel;
}

void TableWorker::slotExecute(QString chk_more_data_query, QString main_query)
{
    if(table_model)
    {
        QSqlDatabase database_connection = QSqlDatabase::database(connection_name + worker_started_at);

        if(!chk_more_data_query.isEmpty()) {
            QSqlQueryModel temp_query_model;
            temp_query_model.setQuery(chk_more_data_query, database_connection);
            if(temp_query_model.lastError().isValid()) {
                emit queryFinished(status, temp_query_model.lastError().text(), false);
                return;
            }
            if(temp_query_model.rowCount() == 0)
                can_fetch_more = false;
            else
                can_fetch_more = true;
        }
        //qDebug() << "CHK QRY: " << chk_more_data_query;
        //qDebug() << "MAINQRY: " << main_query;
        table_model->setQuery(main_query, database_connection);
        if(table_model->lastError().isValid()) {
            emit queryFinished(status, table_model->lastError().text(), false);
            return;
        }

        if(!cancelled()) {
            emit queryFinished(status, QString(), can_fetch_more);
        }
    }
}

void TableWorker::slotCancel()
{
    // Receive the signal to cancel a query.
    // Don't close the QSqlDatabase connection yet.
    // Take care of closing it in the destructor.
    QSqlDatabase database_connection = QSqlDatabase::database(connection_name + worker_started_at);
    if(database_connection.isOpen()) {
        QVariant v = database_connection.driver()->handle();
        if (qstrcmp(v.typeName(), "PGconn*") == 0) {
            PGconn *handle = *static_cast<PGconn **>(v.data());
            if (handle != 0) {
                PGcancel *cancel = PQgetCancel(handle);
                char errbuf[256];
                if(PQcancel(cancel, errbuf, sizeof(errbuf)) == 1) {
                   //emit queryCancelled();
                }
                PQfreeCancel(cancel);
            }
        }
    }
    cancel = true;
}

TableWorker::~TableWorker()
{
    // If a query has not been cancelled, cancel it before
    // destructing and close the QSqlDatabase
    // connection.
    if(cancel == false) {
        QSqlDatabase database_connection = QSqlDatabase::database(connection_name + worker_started_at);
        if(database_connection.isOpen()) {
            QVariant v = database_connection.driver()->handle();
            if (qstrcmp(v.typeName(), "PGconn*") == 0) {
                PGconn *handle = *static_cast<PGconn **>(v.data());
                if (handle != 0) {
                    if(PQisBusy(handle)) {
                        PGcancel *cancel = PQgetCancel(handle);
                        char errbuf[256];
                        PQcancel(cancel, errbuf, sizeof(errbuf));
                        PQfreeCancel(cancel);
                    }
                }
            }
        }
        database_connection.close();
    }
    QSqlDatabase::removeDatabase(connection_name + worker_started_at);
}

TableQueryThread::TableQueryThread(QObject *parent, QSqlQueryModel *table_model, QString chk_more_data_sql, QString default_sql) :
    QThread(parent)
{
    this->status = status;
    this->table_model = static_cast<QSqlQueryModel *>(table_model);
    this->chk_more_data_sql = chk_more_data_sql;
    this->default_sql = default_sql;
    this->connection_name = default_sql;
}

void TableQueryThread::setConnectionParameters(QString host, QString port, QString dbname, QString user, QString password)
{
    this->host = host;
    this->port = port;
    this->dbname = dbname;
    this->user = user;
    this->password = password;
    exec_on_start = false;
}

void TableQueryThread::setExecOnStart(bool exec_on_start)
{
    this->exec_on_start = exec_on_start;
}

void TableQueryThread::executeDefaultQuery()
{
    emit execute(chk_more_data_sql, default_sql);
}

void TableQueryThread::executeQuery(QString chk_more_data_sql, QString query_sql)
{
    while(worker == 0) {

    }
    emit execute(chk_more_data_sql, query_sql);
}

void TableQueryThread::stopQuery()
{
    worker->slotCancel();
}

void TableQueryThread::run()
{
    worker = new TableWorker(0, connection_name, table_model,
                        status,
                        QTime::currentTime().toString("HHmmsszzz"),
                        host,
                        port,
                        dbname,
                        user,
                        password);
    connect(this, &TableQueryThread::execute, worker, &TableWorker::slotExecute);
    connect(worker, &TableWorker::queryFinished, this, &TableQueryThread::workerIsDone);
    //connect(worker, &Worker::queryCancelled, this, &DatabaseWorkerThread::queryCancelled);
    connect(this, &TableQueryThread::cancelQuery, worker, &TableWorker::slotCancel);
    if(exec_on_start)
        worker->slotExecute(chk_more_data_sql, default_sql);
    emit workerStarted();
    exec();
}

TableQueryThread::~TableQueryThread()
{
    delete worker;
}
