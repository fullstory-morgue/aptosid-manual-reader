/*
sidux Manual Reader
Copyright (C) 2009 Fabian Wuertz  
License GPL2
*/

#include <QApplication>
#include "smr.h"

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
	QStringList args = app.arguments();
	Smr *window = new Smr(args);
	window->show();
	return app.exec();
}
