/*
  LICENSE AND COPYRIGHT INFORMATION - Please read carefully.

  Copyright (c) 2010-2013, davyjones <dj@pgxplorer.com>

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

#ifndef STATUSVIEW_H
#define STATUSVIEW_H

#include<QtGui>
#include<QtSql>
#include "database.h"
#include "simplequerythread.h"

class StatusView : public QMainWindow
{
    Q_OBJECT
    Q_PROPERTY(TimerInterval timer_interval READ timerInterval WRITE setTimerInterval)
    Q_ENUMS(TimerInterval)

public:
    StatusView(Database *);

    enum TimerInterval {NoInterval, TimerInterval1, TimerInterval2};
    void setTimerInterval(TimerInterval timer_interval);
    TimerInterval timerInterval() const;

    ToolBar* getToolbar()
    {
        return toolbar;
    }
    void closeEvent(QCloseEvent*);
    void createActions();

    int getTimerInterval()
    {
        return timer_interval;
    }

protected:
    void timerEvent(QTimerEvent *event);
    void focusOutEvent(QFocusEvent *event)
    {
        toggleActions();
    }

private:
    QBasicTimer timer;
    TimerInterval ti;
    Database *database;
    ToolBar *toolbar;
    QSqlQueryModel *status_query_model;
    SimpleQueryThread *status_query_thread;
    QTableView *status_view;
    QString status_message;
    int timer_interval;

    QAction *refresh_action;
    QAction *stop_action;
    QAction *terminate_action;
    QAction *copy_action;
    QAction *timer_action;

    QAction *no_refresh;
    QAction *refresh_freq_1;
    QAction *refresh_freq_2;

    QToolButton *timer_button;

    QMenu *timer_menu;

signals:
    void requestStatus();

private slots:
    void setNoInterval();
    void setTimerInterval1();
    void setTimerInterval2();

public slots:
    void updateStatus();
    void toggleActions();
    void stopQuery();
    void terminateQuery();
    void copyQuery();
    void dontRefresh();
    void refreshFrequency1();
    void refreshFrequency2();
    void languageChanged(QEvent *);
    void bringOnTop();
};

#endif // STATUSVIEW_H
