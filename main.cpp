#include <QApplication>
#include <QDesktopWidget>
#include <QTimer>
#include "mainWin.h"

int main(int argc, char** argv)
{
    Q_INIT_RESOURCE(portedcanvas);
    QApplication app(argc, argv);
    QTranslator translator;
    translator.load("pgXplorer_ja");
    app.installTranslator(&translator);
    QGraphicsScene scn;
    scn.setSceneRect(0, 0, 1024, 768);
    MainWin mainwin(scn);
    mainwin.setWindowTitle("pgXplorer");
    if ( QApplication::desktop()->width() > mainwin.width() + 10
        && QApplication::desktop()->height() > mainwin.height() +30 )
        mainwin.show();
    else
        mainwin.showMaximized();
    QObject::connect( qApp, SIGNAL(lastWindowClosed()), qApp, SLOT(quit()) );
    return app.exec();
}
