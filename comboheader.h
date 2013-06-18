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
    TableView* t;
    QStringList cols;
    QStringList funcs;
    QList<QStringList> funcs_list;
    QList<QComboBox*> boxes;

public:
    explicit ComboHeader(TableView *parent = 0,
                         QStringList columns = QStringList(),
                         QStringList functions = QStringList(),
                         QList<QStringList> functions_list = QList<QStringList>());
    void showEvent(QShowEvent *event);

signals:
    
private slots:
    void handleSectionResized(int);
    void handleSectionMoved(int,int,int);

public slots:
    void fixComboPositions();
    void refreshCombos(QList<QStringList>);
};

#endif // COMBOHEADER_H
