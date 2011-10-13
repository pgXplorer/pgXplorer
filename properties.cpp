#include "properties.h"

PropDialog::PropDialog(Database *db)
{
    QPushButton *pBOK;
    QPushButton *pBCancel;
    QWidget *layoutWidget;
    QVBoxLayout *verticalLayout;
    QLabel *lSrv;
    QLabel *lDb;
    QLabel *lPort;
    QLabel *lUser;
    QLabel *lPass;
    QLabel *lTitle;
    setWindowFlags(Qt::FramelessWindowHint);
    resize(256, 288);
    setModal(true);
    pBOK = new QPushButton(this);
    pBOK->setObjectName(QString::fromUtf8("pBOK"));
    pBOK->setAutoDefault(false);
    pBOK->setGeometry(QRect(30, 250, 75, 23));
    QFont font;
    font.setFamily(QString::fromUtf8("Verdana"));
    pBOK->setFont(font);
    pBCancel = new QPushButton(this);
    pBCancel->setObjectName(QString::fromUtf8("pBCancel"));
    pBCancel->setGeometry(QRect(140, 250, 75, 23));
    pBCancel->setFont(font);
    pBCancel->setAutoDefault(false);
    setDb(new QLineEdit(this));
    getDb()->setObjectName(QString::fromUtf8("lEDb"));
    getDb()->setGeometry(QRect(100, 90, 133, 20));
    getDb()->setFont(font);
    getDb()->setAutoFillBackground(true);
    getDb()->setInputMethodHints(Qt::ImhNone);
    setSrv(new QLineEdit(this));
    getSrv()->setObjectName(QString::fromUtf8("lESrv"));
    getSrv()->setGeometry(QRect(100, 47, 133, 20));
    getSrv()->setFont(font);
    setPort(new QLineEdit(this));
    getPort()->setObjectName(QString::fromUtf8("lEPort"));
    getPort()->setGeometry(QRect(100, 130, 133, 20));
    getPort()->setFont(font);
    getPort()->setInputMethodHints(Qt::ImhFormattedNumbersOnly);
    getPort()->setInputMask("00000");
    setUser(new QLineEdit(this));
    getUser()->setObjectName(QString::fromUtf8("lEUser"));
    getUser()->setGeometry(QRect(100, 171, 133, 20));
    getUser()->setFont(font);
    setPass(new QLineEdit(this));
    getPass()->setObjectName(QString::fromUtf8("lEPass"));
    getPass()->setGeometry(QRect(100, 213, 133, 20));
    getPass()->setEchoMode(QLineEdit::Password);
    getPass()->setFont(font);
    layoutWidget = new QWidget(this);
    layoutWidget->setObjectName(QString::fromUtf8("layoutWidget"));
    layoutWidget->setGeometry(QRect(20, 40, 71, 201));
    verticalLayout = new QVBoxLayout(layoutWidget);
    verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
    verticalLayout->setContentsMargins(0, 0, 0, 0);
    lSrv = new QLabel(layoutWidget);
    lSrv->setObjectName(QString::fromUtf8("lSrv"));
    lSrv->setFont(font);
    verticalLayout->addWidget(lSrv);
    lDb = new QLabel(layoutWidget);
    lDb->setObjectName(QString::fromUtf8("lDb"));
    lDb->setFont(font);
    verticalLayout->addWidget(lDb);
    lPort = new QLabel(layoutWidget);
    lPort->setObjectName(QString::fromUtf8("lPort"));
    lPort->setFont(font);
    verticalLayout->addWidget(lPort);
    lUser = new QLabel(layoutWidget);
    lUser->setObjectName(QString::fromUtf8("lUser"));
    lUser->setFont(font);
    verticalLayout->addWidget(lUser);
    lPass = new QLabel(layoutWidget);
    lPass->setObjectName(QString::fromUtf8("lPass"));
    lPass->setFont(font);
    verticalLayout->addWidget(lPass);
    lTitle = new QLabel(this);
    lTitle->setObjectName(QString::fromUtf8("lTitle"));
    lTitle->setGeometry(QRect(20, 10, 211, 21));
    lTitle->setAlignment(Qt::AlignHCenter);
    QFont font1;
    font1.setFamily(QString::fromUtf8("Verdana"));
    font1.setPointSize(14);
    lTitle->setFont(font1);
    QWidget::setTabOrder(getSrv(), getDb());
    QWidget::setTabOrder(getDb(), getPort());
    QWidget::setTabOrder(getPort(), getUser());
    QWidget::setTabOrder(getUser(), getPass());
    QWidget::setTabOrder(getPass(), pBOK);
    QWidget::setTabOrder(pBOK, pBCancel);
    QObject::connect(pBCancel, SIGNAL(clicked()), this, SLOT(close()));
    QObject::connect(pBOK, SIGNAL(clicked()), this, SLOT(okslot()));
    QObject::connect(this, SIGNAL(oksignal(QString,qint32,QString,QString,QString)),
                     db, SLOT(setConnProps(QString,qint32,QString,QString,QString)));
    QObject::connect(getSrv(), SIGNAL(returnPressed()), getDb(), SLOT(setFocus()));
    QObject::connect(getDb(), SIGNAL(returnPressed()), getPort(), SLOT(setFocus()));
    QObject::connect(getPort(), SIGNAL(returnPressed()), getUser(), SLOT(setFocus()));
    QObject::connect(getUser(), SIGNAL(returnPressed()), getPass(), SLOT(setFocus()));
    QObject::connect(getPass(), SIGNAL(returnPressed()), pBOK, SLOT(setFocus()));
    setWindowTitle(QApplication::translate("Connection", "Connection", 0, QApplication::UnicodeUTF8));
    pBOK->setText(QApplication::translate("Connection", "OK", 0, QApplication::UnicodeUTF8));
    pBCancel->setText(QApplication::translate("Connection", "Cancel", 0, QApplication::UnicodeUTF8));
    if(!db->getStatus()) {
        getSrv()->setText(db->getHost());
        getDb()->setText(db->getName());
        getPort()->setText(db->getPort());
        getUser()->setText(db->getUser());
        getPass()->setText(db->getPassword());
        lSrv->setText(QApplication::translate("Connection", "Server", 0, QApplication::UnicodeUTF8));
        lDb->setText(QApplication::translate("Connection", "Database", 0, QApplication::UnicodeUTF8));
        lPort->setText(QApplication::translate("Connection", "Port", 0, QApplication::UnicodeUTF8));
        lUser->setText(QApplication::translate("Connection", "Username", 0, QApplication::UnicodeUTF8));
        lPass->setText(QApplication::translate("Connection", "Password", 0, QApplication::UnicodeUTF8));
        lTitle->setText(QApplication::translate("Connection", "Connection", 0, QApplication::UnicodeUTF8));
    }
    else {
        QSqlDatabase dbact;
        if(QSqlDatabase::connectionNames().length()>0) {
            dbact = QSqlDatabase::database(QSqlDatabase::connectionNames ().at(0));
        }
        getSrv()->setText(dbact.hostName());
        getDb()->setText(db->getName());
        getPort()->setText(QString::number(dbact.port()));
        getUser()->setText(dbact.userName());
        getPass()->setText(dbact.password());
        lSrv->setText(QApplication::translate("Connection", "Server", 0, QApplication::UnicodeUTF8));
        lDb->setText(QApplication::translate("Connection", "Database", 0, QApplication::UnicodeUTF8));
        lPort->setText(QApplication::translate("Connection", "Port", 0, QApplication::UnicodeUTF8));
        lUser->setText(QApplication::translate("Connection", "Username", 0, QApplication::UnicodeUTF8));
        lPass->setText(QApplication::translate("Connection", "Password", 0, QApplication::UnicodeUTF8));
        lTitle->setText(QApplication::translate("Connection", "Connection", 0, QApplication::UnicodeUTF8));
    }
}

PropDialog::~PropDialog(){}