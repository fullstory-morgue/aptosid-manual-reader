/*
aptosid Manual Reader
Copyright (C) 2009 Fabian Wuertz  
License GPL2
*/

#ifndef SMR_HEADER
#define SMR_HEADER

// #include "ui_smr.h"
#include <QtGui/QMainWindow>
#include <QBitArray>
#include <QTreeWidget>
#include <QTextBrowser>
#include <QLineEdit>
#include <QToolBox>
#include <QLineEdit>
#include <QGridLayout>
#include <QComboBox>
#include <QProcess>
#include <QPushButton>

#include "ui_smr.h"

class Smr : public QMainWindow, Ui::Smr {

	Q_OBJECT

	public:
		Smr(QStringList, QMainWindow* parent = 0, Qt::WFlags flags = 0);


	private slots:
		void loadManual();
		void showChapter0(QTreeWidgetItem*, int);
		void showChapter(QString);
		void showUrl(QUrl);
		void searchManual();
		void showSearch();
		void clearSearch();
		void showIndex();
		void back();
		void back1(QAction*);
		void forward();
		void forward1(QAction*);
		void next();
		void exec();
		void previous();
		void quickstart();
		void aboutaptosid();
		void aboutReader();
		void toDa();
		void toDe();
		void toEn();
		void toEl();
		void toEs();
		void toFr();
		void toHr();
		void toIt(); 
		void toNl();
		void toPl();
		void toJa();
		void toPt();
		void toRo();
		void toRu();

	private:
		QTreeWidget *chaptersTreeWidget;
		int currentChapter;
		QString manpath;
		QProcess proc;
		QStringList backHistory, forwardHistory, h2List, h2IdList;
		QString lang;
		QMenu *backMenu, *forwardMenu;
		bool noHistory, start;
		void getEntries(QString, QString);
		void getMenu(QString, QString);

};
#endif
