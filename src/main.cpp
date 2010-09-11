/*
aptosid Manual Reader
Copyright (C) 2009 Fabian Wuertz  
License GPL2
*/

#include <QApplication>
#include <QTranslator>
#include "smr.h"

int main(int argc, char *argv[])
{
        QString appName = "aptosid-manual-reader";
	QApplication app(argc, argv);
	QStringList args = app.arguments();
        QString locale = QLocale::system().name();
        QTranslator translator;
        translator.load(appName+"_"+locale, "/usr/share/"+appName+"/translations" );
        app.installTranslator(&translator);
	Smr *window = new Smr(args);
	window->show();
	return app.exec();
}
