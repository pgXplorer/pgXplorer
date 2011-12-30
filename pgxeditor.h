/*
  LICENSE AND COPYRIGHT INFORMATION - Please read carefully.

  Copyright (c) 2011, davyjones <davyjones@github.com>

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

#ifndef PGXEDITOR_H
#define PGXEDITOR_H

#include <QtGui>
#include <QPlainTextEdit>
#include <QObject>
#include "database.h"
#include "highlighter.h"

class QPaintEvent;
class QResizeEvent;
class QSize;
class QWidget;

class LineNumberArea;
class PgxEditorMainWindow;

class PgxEditor : public QPlainTextEdit
{
    Q_OBJECT

public:
    static ulong editor_widow_id;
    PgxEditor(Database *, QString);
    void closeMain();
    void setText(QString editor_text, bool save_state);
    void lineNumberAreaPaintEvent(QPaintEvent *event);
    void breakpointAreaPaintEvent(QPaintEvent *event);
    int lineNumberAreaWidth();
    void createActions();
    void setResizePos(QSize, QPoint);
    void save();

protected:
    void resizeEvent(QResizeEvent *event);

private slots:
    void updateLineNumberAreaWidth(int newBlockCount);
    void updateLineNumberArea(const QRect &, int);
    void highlightCurrentLine();
    void removeHighlighting();
    void saveFunction();
    void executeText();
    void executeFunction();
    void selectionChangedSlot();
    void textChangedSlot();
    void toggleFindBar();
    void findText();
    void replaceText();
    void pgxeditorClosing();

private:
    QString editor_name;
    Database *database;
    QWidget *lineNumberArea;
    QWidget *breakpointArea;
    QLineEdit *find_bar;
    QLineEdit *replace_bar;
    QTextCursor find_cursor;
    Highlighter *highlighter;
    QToolBar *toolbar;
    PgxEditorMainWindow *mainwin;
    QAction *cut_action;
    QAction *copy_action;
    QAction *paste_action;
    QAction *save_action;
    QAction *execute_action;
    QAction *selected_execute_action;
    QAction *find_action;
    QAction *casesensitivity_action;
    QAction *wholeword_action;
    QAction *backwards_action;
    QToolButton *casesensitivity_button;
    QToolButton *wholeword_button;
    QToolButton *backwards_button;

Q_SIGNALS:
    void showQueryView(Database *, QString);
    void pgxeditorClosing(PgxEditor *);
};

class LineNumberArea : public QWidget
{
public:
    LineNumberArea(PgxEditor *editor) : QWidget(editor) {
        pgxeditor = editor;
    }
    QSize sizeHint() const {
        return QSize(pgxeditor->lineNumberAreaWidth(), 0);
    }

protected:
    void paintEvent(QPaintEvent *event) {
        pgxeditor->lineNumberAreaPaintEvent(event);
    }

private:
    PgxEditor *pgxeditor;
};

class BreakPointArea : public QWidget
{
public:
    BreakPointArea(PgxEditor *editor) : QWidget(editor) {
        pgxeditor = editor;
    }
    QSize sizeHint() const {
        return QSize(fontMetrics().width(QLatin1Char('9')), 0);
    }
protected:
    void paintEvent(QPaintEvent *event) {
        pgxeditor->breakpointAreaPaintEvent(event);
    }

private:
    PgxEditor *pgxeditor;
};

class PgxEditorMainWindow : public QMainWindow
{
    Q_OBJECT

protected:
    void closeEvent(QCloseEvent *);

Q_SIGNALS:
    void pgxeditorClosing();
};

#endif // PGXEDITOR_H
