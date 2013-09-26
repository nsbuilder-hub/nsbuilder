#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <QTextCodec>

#include "mainwindow.h"

int main (int argc, char *argv[])
{
    QApplication app(argc, argv);
	
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));

    QTranslator qtTranslator;
    qtTranslator.load (QString("qt_%1").arg (QLocale::system ().name ()), app.applicationDirPath ());
    app.installTranslator (&qtTranslator);

    QTranslator nsbuilderTranslator;
    nsbuilderTranslator.load (QString("nsbuilder_%1").arg (QLocale::system ().name ()), app.applicationDirPath ());
    app.installTranslator (&nsbuilderTranslator);

    mainWin = new MainWindow();
    mainWin->show();

    return app.exec();
}

