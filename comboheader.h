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

class ComboBox : public QComboBox
{
    Q_OBJECT
    
private:
    int index;
    
public:
    explicit ComboBox(QWidget *parent=0, int index=0)
    {
        setParent(parent);
        this->index = index;
    }
    
    ~ComboBox(){}
    
    void mousePressEvent(QMouseEvent *e)
    {
        emit columnToSelect(index);
        if(e->x() > width()-20)
            QComboBox::mousePressEvent(e);
        else
            e->accept();
    }
    
    void paintEvent(QPaintEvent *e)
    {
        QStylePainter painter(this);
        painter.setPen(palette().color(QPalette::Text));
        
        QStyleOptionComboBox opt;
        initStyleOption(&opt);
        painter.drawComplexControl(QStyle::CC_ComboBox, opt);
        
        QRect rect = opt.rect;
        
        int h = rect.height();
        
        QFontMetrics fm(painter.font());
        
        if(!itemIcon(0).isNull()) {
            QSize icon_size = iconSize();
            QRect icon_rect(QPoint((h-icon_size.height())/2, (h-icon_size.height())/2), icon_size);
            painter.drawImage(icon_rect, QImage(":/icons/key.svg"), QImage(":/icons/key.svg").rect());
            painter.drawItemText(rect.adjusted(0.75*h, 0, 0, 0), Qt::AlignCenter, palette(), false, fm.elidedText(currentText(), Qt::ElideMiddle, width()-0.75*h));
        }
        else {
            if(count() > 1)
                painter.drawItemText(rect.adjusted(0, 0, -0.5*h, 0), Qt::AlignCenter, palette(), false, fm.elidedText(currentText(), Qt::ElideMiddle, width()-0.5*h));
            else
                painter.drawItemText(rect, Qt::AlignCenter, palette(), false, fm.elidedText(currentText(), Qt::ElideMiddle, width()));
        }
    }
    
signals:
    void columnToSelect(int);
};

class ComboHeader : public QHeaderView
{
    Q_OBJECT

private:
    TableView* tv;
    QList<QComboBox*> boxes;
    const QIcon key_icon = QIcon(":/icons/key.svg");

public:
    explicit ComboHeader(TableView *parent = 0);
    void showEvent(QShowEvent *event);

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
