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

#ifndef COMBOHEADER_H
#define COMBOHEADER_H

#include <tableview.h>
#include <QHeaderView>
#include <QComboBox>
#include <QObject>

class ComboHeader : public QHeaderView
{
    Q_OBJECT

private:
    TableView* tv;
    QList<QComboBox*> boxes;
    const QIcon key_icon = QIcon(":/icons/key.png");

public:
    explicit ComboHeader(TableView *parent = 0);
    void showEvent(QShowEvent *event);
    bool event(QEvent *e) {
        if(e->type() == QEvent::HoverEnter) {
            setCursor(Qt::ArrowCursor);
        }
        QHeaderView::event(e);
    }

signals:
    void changeGrouping(QStringList);
    
private slots:
    void handleSectionResized(int);
    void handleSectionMoved(int,int,int);
    void groupingChanged();

public slots:
    void fixComboPositions();
    void refreshCombos();
};

#endif // COMBOHEADER_H
