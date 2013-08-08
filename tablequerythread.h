#ifndef TABLEQUERYTHREAD_H
#define TABLEQUERYTHREAD_H

#include <QSqlDatabase>
#include <QThread>
#include <tablemodel.h>
#include <libpq-fe.h>

class TableWorker : public QObject
{
    Q_OBJECT

public:
    explicit TableWorker(QObject *parent = 0,
                    QString connection_name = QString(),
                    QSqlQueryModel *table_model = 0,
                    QString status = QString(),
                    QString time = QString(),
                    QString host = QString(),
                    QString port = QString(),
                    QString dbname = QString(),
                    QString user = QString(),
                    QString password = QString());
    ~TableWorker();
    bool cancelled();

    bool connected() {
        QSqlDatabase database_connection = QSqlDatabase::database(connection_name + worker_started_at);
        return database_connection.isOpen();
    }

private:
    QString connection_name;
    QString status;
    QString worker_started_at;
    QSqlQueryModel *table_model;
    bool can_fetch_more;
    bool cancel;

signals:
    void queryFinished(QString, QString, bool);
    //void queryCancelled();

public slots:
    void slotExecute(QString, QString);
    void slotCancel();
};

class TableQueryThread : public QThread
{
    Q_OBJECT
public:
    explicit TableQueryThread(QObject *parent = 0,
                              QSqlQueryModel *table_model = 0,
                              QString chk_more_data_sql = QString(),
                              QString default_query = QString());
    ~TableQueryThread();
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
    QSqlQueryModel *table_model;
    QString chk_more_data_sql;
    QString default_sql;
    TableWorker *worker;

signals:
    void execute(QString, QString);
    void workerStarted();
    void workerIsDone(QString, QString, bool);
    void cancelQuery();

public slots:
    void setExecOnStart(bool);
    void executeDefaultQuery();
    void executeQuery(QString, QString);
    void stopQuery();
    
};

#endif // TABLEQUERYTHREAD_H
