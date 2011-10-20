#ifndef PGXCONSOLE_H
#define PGXCONSOLE_H

#include <QPlainTextEdit>
#include <QSqlQueryModel>
#include <QSqlQuery>
#include <QSyntaxHighlighter>
#include <QHash>
#include <QTextCharFormat>
#include <QObject>
#include <QTranslator>
#include <QCompleter>
#include "database.h"

class QPaintEvent;
class QResizeEvent;
class QSize;
class QWidget;
class Prompt;
class Highlighter;
class QTextDocument;

//! Pgx SQL Console

class PgxConsole : public QPlainTextEdit
{
    Q_OBJECT

public:
    PgxConsole(QWidget *parent = 0);
    ~PgxConsole() {
    }
    void promptPaintEvent(QPaintEvent *event);
    void setDbPros(QString, int, QString, QString, QString);

protected:
    void resizeEvent(QResizeEvent *event);
    virtual void keyPressEvent(QKeyEvent *e);
    virtual void wheelEvent(QWheelEvent *e);
    //virtual void closeEvent(QCloseEvent *);

private slots:
    //void updatePromptWidth(int newBlockCount);
    void updatePrompt(const QRect &, int);
    void curChanged();
    void showView(QKeyEvent *);
    void histUpCmd();
    void histDnCmd();
    void paste_cmd();
    void getErrMesg(QString, uint);
    void finish();

private:
    QString cmd;
    QWidget *prompt;
    Highlighter *highlighter;
    QStringList history;
    qint32 hit;
    QCompleter *completer;
    QString host;
    int port;
    QString dbname;
    QString user;
    QString password;
    void createWidgets();

Q_SIGNALS:
    void cmdS(QKeyEvent *);
    void histUp();
    void histDn();
    void getDbPros();
};

class Prompt : public QWidget
{
public:
    Prompt(PgxConsole *console) : QWidget(console) {
        pgxConsole = console;
        this->setStyleSheet("QWidget{background-color: white; font: bold 14px;}");
    }
    QSize sizeHint() const {
        return QSize(10, 0);
    }

protected:
    void paintEvent(QPaintEvent *event) {
         pgxConsole->promptPaintEvent(event);
     }

private:
    PgxConsole *pgxConsole;
};

class Highlighter : public QSyntaxHighlighter
{
     Q_OBJECT

 public:
     Highlighter(QTextDocument *parent = 0);

 protected:
     void highlightBlock(const QString &text);

 private:
     struct HighlightingRule
     {
         QRegExp pattern;
         QTextCharFormat format;
     };
     QVector<HighlightingRule> highlightingRules;
     QTextCharFormat keywordFormat;
     QTextCharFormat keywordFormat2;
     QTextCharFormat classFormat;
     QTextCharFormat singleQuotFormat;
     QTextCharFormat doubleQuotFormat;
     QTextCharFormat functionFormat;
};

class SqlMdl : public QSqlQueryModel
{
    Q_OBJECT

private:
    QString dbConnId;

public slots:
    void destry();

public:
    void fetchData(SqlMdl*, QString, QStringList);

Q_SIGNALS:
    void fetchDataSignal(SqlMdl*, int, qint32, qint32);
    void busySignal();
};

#endif // PGXCONSOLE_H
