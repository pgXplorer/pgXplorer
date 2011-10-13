#ifndef PGXCONSOLE_H
#define PGXCONSOLE_H

#include <QPlainTextEdit>
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
    void promptPaintEvent(QPaintEvent *event);

protected:
    void resizeEvent(QResizeEvent *event);
    virtual void keyPressEvent(QKeyEvent *e);
    virtual void wheelEvent(QWheelEvent *e);

private slots:
    //void updatePromptWidth(int newBlockCount);
    void updatePrompt(const QRect &, int);
    void curChanged();
    void showView(QKeyEvent *);
    void histUpCmd();
    void histDnCmd();
    void paste_cmd();

private:
    QWidget *prompt;
    Highlighter *highlighter;
    QStringList history;
    qint32 hit;
    QCompleter *completer;
    Database *db;
    void createWidgets();

Q_SIGNALS:
    void cmd(QKeyEvent *);
    void histUp();
    void histDn();
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

#endif // PGXCONSOLE_H