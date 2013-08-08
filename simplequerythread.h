#ifndef SIMPLEQUERYTHREAD_H
#define SIMPLEQUERYTHREAD_H

#include <QSqlDatabase>
#include <QThread>
#include <tablemodel.h>
#include <libpq-fe.h>

class QueryWorker : public QObject
{
    Q_OBJECT

public:
    explicit QueryWorker(QObject *parent = 0,
                    QString connection_name = QString(),
                    QSqlQueryModel *query_model = 0,
                    QString status = QString(),
                    QString time = QString(),
                    QString host = QString(),
                    QString port = QString(),
                    QString dbname = QString(),
                    QString user = QString(),
                    QString password = QString());
    ~QueryWorker();
    bool cancelled();

    bool connected() {
        QSqlDatabase database_connection = QSqlDatabase::database(connection_name + worker_started_at);
        return database_connection.isOpen();
    }

private:
    QString connection_name;
    QString status;
    QString worker_started_at;
    QSqlQueryModel *query_model;
    bool cancel;

signals:
    void queryFinished(QString, QString);
    //void queryCancelled();

public slots:
    void slotExecute(QString);
    void slotCancel();
};

class SimpleQueryThread : public QThread
{
    Q_OBJECT
public:
    explicit SimpleQueryThread(QObject *parent = 0, QSqlQueryModel *table_model = 0, QString sql = QString());
    ~SimpleQueryThread();
    bool cancelled() {
        return worker->cancelled();
    }
    void setConnectionParameters(QString host,
                                 QString port,
                                 QString dbname,
                                 QString user,
                                 QString password);

protected:
    void run();

private:
    bool exec_on_start;
    QString connection_name;
    QString status;
    QString host;
    QString port;
    QString dbname;
    QString user;
    QString password;
    QSqlQueryModel *query_model;
    QString default_sql;
    QueryWorker *worker;

signals:
    void execute(QString);
    void workerIsDone(QString, QString);
    void cancelQuery();
    //void queryCancelled();
    
public slots:
    void setExecOnStart(bool);
    void executeDefaultQuery();
    void executeQuery(QString);
    void stopQuery();
};

#endif // SIMPLEQUERYTHREAD
