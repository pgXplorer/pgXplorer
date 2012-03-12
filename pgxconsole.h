/*
  LICENSE AND COPYRIGHT INFORMATION - Please read carefully.

  Copyright (c) 2011-2012, davyjones <davyjones@github>

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

#ifndef PGXCONSOLE_H
#define PGXCONSOLE_H

#include <QPlainTextEdit>
#include <QSqlQuery>
#include <QHash>
#include <QTextCharFormat>
#include <QObject>
#include <QTranslator>
#include <QCompleter>
#include "database.h"
#include "highlighter.h"

class QPaintEvent;
class QResizeEvent;
class QSize;
class QWidget;
class Prompt;
class Highlighter;
class QTextDocument;
class PgxConsoleMainWindow;

class PgxConsole : public QPlainTextEdit
{
    Q_OBJECT

public:
    PgxConsole(Database*);
    ~PgxConsole() {
    }
    void highlightSearchedWord();
    void removeSearchHighlighting();
    void promptPaintEvent(QPaintEvent *);
    void setConnectionProperties(QString, int, QString, QString, QString);
    void insertFromMimeData(const QMimeData *);
    void createActions();
    void setResizePos(QSize, QPoint);
    void closeMain();

protected:
    void resizeEvent(QResizeEvent *);
    void keyPressEvent(QKeyEvent *);
    void wheelEvent(QWheelEvent *);

private slots:
    void updatePrompt(const QRect &, int);
    void makePreviousBlocksReadonly();
    void showView(QString);
    void historyUpCommand();
    void historyDownCommand();
    void pasteFromClipboard();
    void pasteAsSingleFromClipboard();
    void toggleFindBar();
    void findText();
    void pgxconsoleClosing();

public slots:
    void languageChanged(QEvent*);

private:
    PgxConsoleMainWindow *pgxconsole_mainwin;
    Database *database;
    QWidget *prompt;
    QLineEdit *find_bar;
    Highlighter *highlighter;
    QTextCursor find_cursor;
    QToolBar *toolbar;
    QStringList history;
    qint32 hit;
    QCompleter *completer;
    QAction *newpgxconsole_action;
    QAction *cut_action;
    QAction *copy_action;
    QAction *paste_action;
    QAction *clear_action;
    QAction *find_action;
    QAction *casesensitivity_action;
    QAction *wholeword_action;
    QAction *backwards_action;
    QToolButton *casesensitivity_button;
    QToolButton *wholeword_button;
    QToolButton *backwards_button;
    void __createWidgets();

Q_SIGNALS:
    void commandSignal(QString);
    void historyUp();
    void historyDown();
    void getDbPros();
    void newPgxconsole();
    void showQueryView(Database *, QString);
    void pgxconsoleClosing(PgxConsole *);
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

class PgxConsoleMainWindow : public QMainWindow
{
    Q_OBJECT

protected:
    void closeEvent(QCloseEvent *);

Q_SIGNALS:
    void pgxconsoleClosing();
};

#endif // PGXCONSOLE_H
