#ifndef TABLEPROPERTIES_H
#define TABLEPROPERTIES_H

#include <QtGui>
#include <QtWidgets>

class TableProperties : public QDialog
{
    Q_OBJECT

private:
    QLabel *title;
    QCheckBox *with_oid;
    QLineEdit *parent_table;
    QComboBox *inherit_like;
    QLineEdit *tablespace;
    QSpinBox *fill_factor;
    QDialogButtonBox *button_box;
    QFormLayout *form_layout;
    QCompleter *completer;

public:
    TableProperties(QMainWindow *, QString, QStringList, bool, QString, QString, int);
    ~TableProperties(){}

private slots:
    void okslot();

public slots:
    void languageChanged(QEvent*);

signals:
    void oksignal(bool, QString, QString, int);
};

#endif // TABLEPROPERTIES_H
