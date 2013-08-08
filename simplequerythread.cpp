#include "simplequerythread.h"

QueryWorker::QueryWorker(QObject *parent, QString connection_name,
               QSqlQueryModel *query_model,
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
    this->query_model = query_model;

    database_connection.setHostName(host);
    database_connection.setPort(port.toInt());
    database_connection.setDatabaseName(dbname);
    database_connection.setUserName(user);
    database_connection.setPassword(password);

    database_connection.open();
    cancel = false;
}

bool QueryWorker::cancelled()
{
   return cancel;
}

void QueryWorker::slotExecute(QString query)
{
    if(query_model)
    {
        QSqlDatabase database_connection = QSqlDatabase::database(connection_name + worker_started_at);

        query_model->setQuery(query, database_connection);
        if(query_model->lastError().isValid()) {
            emit queryFinished(status, query_model->lastError().text());
            return;
        }

        if(!cancelled()) {
            emit queryFinished(status, QString());
        }
    }
}

void QueryWorker::slotCancel()
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

QueryWorker::~QueryWorker()
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

SimpleQueryThread::SimpleQueryThread(QObject *parent, QSqlQueryModel *query_model, QString default_sql) :
    QThread(parent)
{
    this->status = status;
    this->query_model = static_cast<QSqlQueryModel *>(query_model);
    this->default_sql = default_sql;
    this->connection_name = default_sql;
}

void SimpleQueryThread::setConnectionParameters(QString host, QString port, QString dbname, QString user, QString password)
{
    this->host = host;
    this->port = port;
    this->dbname = dbname;
    this->user = user;
    this->password = password;
    exec_on_start = false;
}

void SimpleQueryThread::setExecOnStart(bool exec_on_start)
{
    this->exec_on_start = exec_on_start;
}

void SimpleQueryThread::executeDefaultQuery()
{
    emit execute(default_sql);
}

void SimpleQueryThread::executeQuery(QString query_sql)
{
    while(worker == 0) {

    }
    emit execute(query_sql);
}

void SimpleQueryThread::stopQuery()
{
    worker->slotCancel();
}

void SimpleQueryThread::run()
{
    worker = new QueryWorker(0, connection_name, query_model,
                        status,
                        QTime::currentTime().toString("HHmmsszzz"),
                        host,
                        port,
                        dbname,
                        user,
                        password);
    connect(this, &SimpleQueryThread::execute, worker, &QueryWorker::slotExecute);
    connect(worker, &QueryWorker::queryFinished, this, &SimpleQueryThread::workerIsDone);
    //connect(worker, &Worker::queryCancelled, this, &DatabaseWorkerThread::queryCancelled);
    connect(this, &SimpleQueryThread::cancelQuery, worker, &QueryWorker::slotCancel);
    if(exec_on_start)
        worker->slotExecute(default_sql);
    exec();
}

SimpleQueryThread::~SimpleQueryThread()
{
    delete worker;
}
