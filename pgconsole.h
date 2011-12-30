#ifndef PGCONSOLE_H
#define PGCONSOLE_H

#include <QtGui>

class PgConsole : public QTextEdit
{
    Q_OBJECT

public:
    PgConsole();

protected:
    virtual void keyPressEvent(QKeyEvent * e);

private slots:
    void showView(QString);

Q_SIGNALS:
    void sqlcmd(QString);
};

#endif // PGCONSOLE_H
