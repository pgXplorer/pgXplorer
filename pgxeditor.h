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
    void setCompleter(QCompleter *c);
    QCompleter *completer() const;

    static ulong editor_widow_id;
    PgxEditor(Database *, QString, QString);
    void closeMain();
    void setTitle(QString);
    void setText(QString, bool);
    void lineNumberAreaPaintEvent(QPaintEvent *event);
    void breakpointAreaPaintEvent(QPaintEvent *event);
    int lineNumberAreaWidth();
    void hightlightFirstBlock();
    void createActions();
    void setResizePos(QSize, QPoint);
    void appendCompleterList(QString);
    QStringList completerList();
    QString editorName()
    {
        return editor_name;
    }
    PgxEditorMainWindow* mainWin()
    {
        return pgxeditor_mainwin;
    }

protected:
    void wheelEvent(QWheelEvent *);
    void resizeEvent(QResizeEvent *event);
    //bool eventFilter(QObject *, QEvent*);
    void dragEnterEvent(QDragEnterEvent *);
    void dragMoveEvent(QDragMoveEvent *);
    void dropEvent(QDropEvent *);
    void keyPressEvent(QKeyEvent *e);
    void focusInEvent(QFocusEvent *e);

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
    void makeFirstBlockReadonly();
    void toggleWrap();
    void toggleFindBar();
    void findPrevious();
    void findNext();
    void replacePrevious();
    void replaceNext();
    void pgxeditorClosing();
    void noZoom();
    void insertCompletion(const QString &completion);

public slots:
    void languageChanged(QEvent*);
    void changeIcons(bool);

private:
    QStringList completer_list;
    QString style_sheet;
    quint8 font_size;
    QString editor_name;
    QString function_args;
    Database *database;
    QWidget *lineNumberArea;
    QWidget *breakpointArea;
    QLineEdit *find_bar;
    QLineEdit *replace_bar;
    QTextCursor find_cursor;
    Highlighter *highlighter;
    QToolBar *toolbar;
    PgxEditorMainWindow *pgxeditor_mainwin;
    QAction *newpgxeditor_action;
    QAction *cut_action;
    QAction *copy_action;
    QAction *paste_action;
    QAction *save_action;
    QAction *execute_action;
    QAction *selected_execute_action;
    QAction *wrap_action;
    QAction *find_action;
    QAction *complete_action;
    QAction *casesensitivity_action;
    QAction *wholeword_action;
    //QAction *backwards_action;
    QAction *find_next_action;
    QAction *find_previous_action;
    QAction *replace_next_action;
    QAction *replace_previous_action;
    QToolButton *casesensitivity_button;
    QToolButton *wholeword_button;
    QToolButton *backwards_button;
    QToolButton *find_previous_button;
    QToolButton *find_next_button;
    QToolButton *replace_previous_button;
    QToolButton *replace_next_button;
    QIcon next_icon;
    QString textUnderCursor() const;
    QCompleter *c;

    void matchParentheses();
    bool matchLeftParenthesis(QTextBlock currentBlock, int index, int numRightParentheses);
    bool matchRightParenthesis(QTextBlock currentBlock, int index, int numLeftParentheses, bool firstTime = false);
    void createParenthesisSelection(int pos);
    void warnParenthesisSelection(int pos);

signals:
    void newPgxeditor();
    void newPgxeditor(QString);
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

private:
    PgxEditor *pgxeditor;
    QAbstractItemModel *modelFromFile(const QString& fileName);
    QCompleter *completer;

public slots:
    void bringOnTop();

protected:
    void closeEvent(QCloseEvent *);
    //bool eventFilter(QObject *, QEvent*);

public:
    PgxEditorMainWindow(PgxEditor *editor) {
        pgxeditor = editor;
        QStringList wordList;
        wordList << "select" << "insert" << "update" << "delete"
                 << "truncate" << "union" << "all" << "intersect"
                 << "except"  << "from" << "in" << "into" << "with"
                 << "where" << "join" << "on" << "and" << "group by"
                 << "left" << "right" << "full" << "cross"
                 << "inner" << "outer" << "natural" << "values"
                 << "order by" << "limit" << "fetch" << "having"
                 << "window" << "offset"
                 << "SELECT" << "INSERT" << "UPDATE" << "DELETE"
                 << "TRUNCATE" << "UNION" << "ALL" << "INTERSECT"
                 << "EXCEPT"  << "FROM" << "IN" << "INTO" << "WITH"
                 << "WHERE" << "JOIN" << "ON" << "AND" << "GROUP BY"
                 << "LEFT" << "RIGHT" << "FULL" << "CROSS"
                 << "INNER" << "OUTER" << "NATURLA" << "VALUES"
                 << "ORDER BY" << "LIMIT" << "FETCH" << "HAVING"
                 << "WINDOW" << "OFFSET";
        wordList.append(pgxeditor->completerList());
        completer = new QCompleter(wordList, this);
        completer->setWrapAround(false);
        pgxeditor->setCompleter(completer);
    }

signals:
    void changeIcons(bool);
    void pgxeditorClosing();
};

#endif // PGXEDITOR_H
