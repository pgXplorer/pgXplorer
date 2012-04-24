#ifndef TABLEPROPERTIES_H
#define TABLEPROPERTIES_H

#include <QtGui>

class TableProperties : public QDialog
{
    Q_OBJECT

private:
    QLabel *title;
    QCheckBox *with_oid;
    QCheckBox *inherit_like_cb;
    QComboBox *inherit_like;
    QLineEdit *tablespace;
    QSpinBox *fill_factor;
    QDialogButtonBox *button_box;

public:
    TableProperties(QMainWindow *, QString);
    ~TableProperties(){}

private slots:
    void okslot()
    {
        emit oksignal("test");
        close();
    }

signals:
    void oksignal(QString);
};

#endif // TABLEPROPERTIES_H
