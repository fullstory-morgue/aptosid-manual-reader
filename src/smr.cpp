/*
sidux Manual Reader
Copyright (C) 2009 Fabian Wuertz  
License GPL2
*/

#include "smr.h"
#include "QDir"
#include "QTextStream"
#include "QDockWidget"
#include "QLineEdit"
#include "QHeaderView"
#include "QTabWidget"
#include "QMessageBox"
#include "QWebFrame"
#include "QDesktopServices"
#include <QSettings>

Smr::Smr(QStringList args, QMainWindow* parent, Qt::WFlags flags): QMainWindow (parent, flags) {

	manpath = "/usr/share/sidux-manual/";



	// init gui
 	setupUi(this);
	chaptersTreeWidget = new QTreeWidget;
	chaptersTreeWidget->setColumnCount(3);
	backMenu = new QMenu();
	backToolButton->setMenu(backMenu);
	forwardMenu = new QMenu();
	forwardToolButton->setMenu(forwardMenu);
	webView->page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);

	// get languages
	QSettings settings("sidux-manual-reader");
	lang = settings.value("language").toString ();
	if(lang.isEmpty())
		lang = QLocale::system().name().left(2);
	QStringList languages;
	languages << "da" << "de" << "en" << "el" << "es" << "fr" << "hr" << "it" << "nl" << "pl" << "ja" <<  "pt" << "ro" << "ru";
	if( lang == "pt")
		lang = "pt-br";
	else if( lang == "" or languages.indexOf(lang) == -1)
		lang = "en";
	loadManual();

	// argument
	if(args.count() > 1 )
		showChapter(args[1]);
	start = true;
}



//------------------------------------------------------------------------------
//-- search --------------------------------------------------------------------
//------------------------------------------------------------------------------

void Smr::showSearch() {
	stackedWidget->setCurrentIndex(1);
}

void Smr::searchManual() {

	if(searchLineEdit->text().isEmpty() )
		return;
  
	QStringList searchList = searchLineEdit->text().split(" ");
	QTreeWidget *tmpTreeWidget1 = new QTreeWidget;
	tmpTreeWidget1->setColumnCount(3);
	QTreeWidgetItemIterator it(chaptersTreeWidget);
	while (*it) {
		QTreeWidgetItem *item = new QTreeWidgetItem(tmpTreeWidget1, 0);
		item->setText( 0, (*it)->text(0) );
		item->setText( 1, (*it)->text(1) );
		item->setText( 2, (*it)->text(2) );
		++it;
	}
	QTreeWidget *tmpTreeWidget2 = new QTreeWidget;
	tmpTreeWidget2->setColumnCount(3);

	foreach(QString search, searchList) {
		QList<QTreeWidgetItem *> result = tmpTreeWidget1->findItems(search, Qt::MatchContains, 2 );
		tmpTreeWidget2->clear();
		for(int i = 0; i < result.count(); i++) {
			QTreeWidgetItem *item = new QTreeWidgetItem(tmpTreeWidget2, 0);
			item->setText( 0, result[i]->text(0) );
 			item->setText( 1, result[i]->text(1) );
 			item->setText( 2, result[i]->text(2) );
		}
		tmpTreeWidget1->clear();
		QTreeWidgetItemIterator it(tmpTreeWidget2);
		while (*it) {
			QTreeWidgetItem *item = new QTreeWidgetItem(tmpTreeWidget1, 0);
			item->setText( 0, (*it)->text(0) );
			item->setText( 1, (*it)->text(1) );
			item->setText( 2, (*it)->text(2) );
			++it;
		}
	}

	searchTreeWidget->clear();
	QTreeWidgetItemIterator it2(tmpTreeWidget2);
	while (*it2) {
		QTreeWidgetItem *item = new QTreeWidgetItem(searchTreeWidget, 0);
		item->setText( 0, (*it2)->text(0) );
		item->setText( 1, (*it2)->text(1) );
		item->setText( 2, (*it2)->text(2) );
		++it2;
	}
	stackedWidget->setCurrentIndex(1);
}


void Smr::clearSearch() {
	if(start) {
		searchLineEdit->clear();
		start = false;
	}
}

//------------------------------------------------------------------------------
//-- index ---------------------------------------------------------------------
//------------------------------------------------------------------------------


void Smr::showIndex() {
	stackedWidget->setCurrentIndex(0);
}

//------------------------------------------------------------------------------
//-- content -------------------------------------------------------------------
//------------------------------------------------------------------------------

void Smr::showChapter0(QTreeWidgetItem* item, int i) {

	if( indexTreeWidget->indexOfTopLevelItem(item) != -1) {
		if ( item->isExpanded() )
			item->setExpanded(FALSE);
		else
			item->setExpanded(TRUE);
		return;
	}
	forwardHistory.clear();
	showChapter(item->text(1));
	
}


void Smr::showChapter(QString id) {
  
  
	noHistory = false;
  
	if( chaptersTreeWidget->findItems(id, Qt::MatchExactly, 1).count() < 1 ) {
		webView->setHtml(tr("Sorry this page doesn't exist!")+id);
		navWidget->hide();
		stackedWidget->setCurrentIndex(2);
		return;
	}



	QString content = chaptersTreeWidget->findItems(id, Qt::MatchExactly, 1).first()->text(2);
	content.prepend("<head><link href=\"/usr/share/sidux-manual-reader/css/content.css\" rel=\"stylesheet\" type=\"text/css\" /></head>");
	// create alters
	content.replace( QRegExp("<p class=\"highlight-2\">([^<]*)</p>"), "<table class=alert><tr><td class=noBorder><img src=\"/usr/share/sidux-manual-reader/icons/software-update-urgent.svg\"></td><td class=noBorder>\\1</td></tr></table>");
	content.replace( QRegExp("<div class=\"highlight-2\">([^<]*)</div>"), "<table class=alert><tr><td class=noBorder><img src=\"/usr/share/sidux-manual-reader/icons/software-update-urgent.svg\"></td><td class=noBorder>\\1</td></tr></table>");

	
	
	
	
	if( searchLineEdit->text() != "" ) {
		QStringList searchList = searchLineEdit->text().split(" ");
		foreach(QString search, searchList)
			content.replace(" "+search+" ", " <b style=\"background-color:#FFFF66; font-size:x-large\">"+search+"</b> ");
	}



 	webView->setHtml(content);

	
	// backMenu
	backMenu->clear();
	int i = 0;
	foreach(QString e, backHistory ) {
		if( chaptersTreeWidget->findItems(e, Qt::MatchExactly, 1 ).count() > 0 ) {
			QAction* action = new QAction(chaptersTreeWidget->findItems(e, Qt::MatchExactly, 1).first()->text(0), this);
			action->setWhatsThis(QString::number(i));
			backMenu->addAction(action);
			i++;
		}
	}



	// forwardMenu
	forwardMenu->clear();
	i = 0;
	foreach(QString e, forwardHistory ) {
		if( chaptersTreeWidget->findItems(e, Qt::MatchExactly, 1 ).count() > 0 ) {
			QAction* action = new QAction(chaptersTreeWidget->findItems(e, Qt::MatchExactly, 1).first()->text(0), this);
			action->setWhatsThis(QString::number(i));
			forwardMenu->addAction(action);
			i++;
		}
	}



	// show previous/next buttons
	currentChapter = h2IdList.indexOf(id);
	if( currentChapter > 0 ) {
		previousPushButton->setText( h2List[currentChapter-1] );
		previousPushButton->show();
	}
	else
		previousPushButton->hide();
	if(currentChapter+1 < h2List.count() ) {
		nextPushButton->setText( h2List[currentChapter+1] );
		nextPushButton->show();
	}
	else
		nextPushButton->hide();


	// show exec buttons
	if( id == "siduxcc") {
		execPushButton->setText( tr("start")+" "+tr("sidux Control Center") );
		execPushButton->setIcon( QPixmap( "/usr/share/icons/hicolor/16x16/apps/siduxcc.png") );
		execPushButton->show();
	}
	else if( id == "netcardconfig") {
		execPushButton->setText( tr("start")+" "+tr("Ceni") );
		execPushButton->setIcon( QPixmap( "/usr/share/icons/hicolor/16x16/apps/ceni.png") );
		execPushButton->show();
	}
	else if( id == "dial-mod") {
		execPushButton->setText( tr("start")+" "+tr("KPPP") );
		execPushButton->setIcon( QPixmap( "/usr/share/icons/hicolor/16x16/apps/kppp.png") );
		execPushButton->show();
	}
	else  {
		execPushButton->hide();
	}

		

	navWidget->show();
	stackedWidget->setCurrentIndex(2); 

}

void Smr::showUrl(QUrl input) {

	QString id = input.toString();
	id.replace("file:///usr/share/sidux-manual/"+lang+"/", "");

	if( id.contains("http") ) {
		QDesktopServices::openUrl(id);
		return;
	}


	if( !id.contains("#") ) {
		if( chaptersTreeWidget->findItems(id, Qt::MatchContains, 2 ).count() < 1 )
			id = "missing";
		else
			id = chaptersTreeWidget->findItems(id, Qt::MatchExactly, 2).first()->text(1);
	}
	id = id.split("#")[1];
	backHistory.prepend( h2IdList[currentChapter] );
	forwardHistory.clear();
	showChapter( id );
}


//------------------------------------------------------------------------------
//-- button actions ------------------------------------------------------------
//------------------------------------------------------------------------------


void Smr::back() {
	if( backHistory.count() == 0)
		return;
	forwardHistory.prepend( h2IdList[currentChapter] );
	QString id = backHistory.first();
	backHistory.removeFirst();
	showChapter( id );
}

void Smr::back1(QAction* action) {
	int b = action->whatsThis().toInt();
	QString id = backHistory[b];
	for( int i = 0; i < b; i++) {
		forwardHistory.prepend( backHistory.first() );
		backHistory.removeFirst();
	}
	forwardHistory.append( h2IdList[currentChapter] );
	backHistory.removeFirst();
	showChapter( id );
}


void Smr::forward() {
	if( forwardHistory.count() == 0)
		return;
	backHistory.prepend( h2IdList[currentChapter] );
	QString id = forwardHistory.first();
	forwardHistory.removeFirst();
	showChapter( id );
}


void Smr::forward1(QAction* action) {
	int b = action->whatsThis().toInt();
	QString id = forwardHistory[b];
	for( int i = 0; i < b; i++) {
		backHistory.prepend( forwardHistory.first() );
		forwardHistory.removeFirst();
	}
	backHistory.append( h2IdList[currentChapter] );
	forwardHistory.removeFirst();
	showChapter( id );
}


void Smr::next() {
	backHistory.prepend( h2IdList[currentChapter] );
	forwardHistory.clear();
	showChapter( h2IdList[currentChapter+1] );
}


void Smr::previous() {
	backHistory.prepend( h2IdList[currentChapter] );
	forwardHistory.clear();
	showChapter( h2IdList[currentChapter-1] );
}

void Smr::exec() {

	QString program;
	QStringList arguments;
	if( execPushButton->text().contains("sidux Control Center") ) {
		program = "su-to-root";
		arguments << "-X" << "-c" << "/usr/bin/siduxcc-kde3";
	}
	else if( execPushButton->text().contains("Ceni") ) {
		program = "x-terminal-emulator";
		arguments << "-e" << "/usr/bin/Ceni";
	}
	else if( execPushButton->text().contains("KPPP") )
		program = "/usr/bin/kppp";
	else
		return;

	QProcess *myProcess = new QProcess(this);
	myProcess->start(program, arguments);

}


void Smr::quickstart() {
	if(!noHistory )
		backHistory.prepend( h2IdList[currentChapter] );
	forwardHistory.clear();
	showChapter( "welcome-quick" );
}

void Smr::aboutSidux() {
  	if(!noHistory )
		backHistory.prepend( h2IdList[currentChapter] );
	forwardHistory.clear();
	showChapter( "cred-team" );
}

void Smr::aboutReader() {
    	if(!noHistory )
		backHistory.prepend( h2IdList[currentChapter] );
	webView->setHtml("<h2>sidux Manual Reader</h2><b>"+tr("Programmer")+":</b> Fabian Wuertz xadras@sidux.com<br><b>"+tr("License")+":</b> GPL2<br>");
	stackedWidget->setCurrentIndex(2);
	navWidget->hide();
}

//------------------------------------------------------------------------------
//-- load manual ---------------------------------------------------------------
//------------------------------------------------------------------------------


void Smr::toDa() { lang = "da"; loadManual(); }
void Smr::toDe() { lang = "de"; loadManual(); }
void Smr::toEn() { lang = "en"; loadManual(); }
void Smr::toEl() { lang = "el"; loadManual(); }
void Smr::toEs() { lang = "es"; loadManual(); }
void Smr::toFr() { lang = "fr"; loadManual(); }
void Smr::toHr() { lang = "hr"; loadManual(); }
void Smr::toIt() { lang = "it"; loadManual(); }
void Smr::toNl() { lang = "nl"; loadManual(); }
void Smr::toPl() { lang = "pl"; loadManual(); }
void Smr::toJa() { lang = "ja"; loadManual(); }
void Smr::toPt() { lang = "pt-br"; loadManual(); }
void Smr::toRo() { lang = "ro"; loadManual(); }
void Smr::toRu() { lang = "ru"; loadManual(); }

void Smr::loadManual() {

	if( !QFile::exists( "/usr/share/doc/sidux-manual-"+lang+"/copyright" ) ) {
		if( QMessageBox::question(this, tr("Question"), tr("The manual with this language is not installed! Do you want to install it?"), QMessageBox::Yes | QMessageBox::No  ) == QMessageBox::Yes ) {
			QString program = "su-to-root";
			QStringList arguments;
			arguments << "-X" << "-c" << "sidux-apt-qt4 +sidux-manual-"+lang;
			QProcess *process = new QProcess(this);
			connect( process, SIGNAL(finished(int)),this, SLOT(loadManual()));
			process->start(program, arguments);
		}
		return;
	}

	QSettings settings("sidux-manual-reader");
	settings.setValue("language", lang);

	indexTreeWidget->clear();
	chaptersTreeWidget->clear();
	backHistory.clear();
	backMenu->clear();
	forwardHistory.clear();
	forwardMenu->clear();
	h2List.clear();
	h2IdList.clear();

	QString manpath2 = manpath+lang+"/";
	QStringList pages = QDir( manpath2 ).entryList( QDir::Files );
	pages.removeOne( "menu-"+lang );


	// get h1 
	QFile file1( manpath2+pages[0] );
	file1.open( QIODevice::ReadOnly | QIODevice::Text );
	QTextStream stream( &file1 );
	QString tmp;
	QStringList titlePath, titleName;

	
	// get menu
	// ------------------------------------------------------------------------------
	while ( !stream.atEnd() ) {
	
		QString line = stream.readLine();
		if( line.contains("showHide") ) {
			QString h1   = line.split(">")[2].replace("</a", "").replace("&amp;", "&").replace("&#230;", QString::fromUtf8("æ") );
			QString h1Id = line.split("\"")[5];
			QTreeWidgetItem *item = new QTreeWidgetItem(indexTreeWidget);
			item->setText( 0,  h1 );
			item->setText( 1,  h1Id ); 
			stream.readLine();
			line = 	stream.readLine();


			while( line.contains("<li><a href="))
			{
				if( line.contains(".htm\"") )
					line.replace( ".htm", ".htm#misssing" );
				QString h2   = line.split(">")[2].replace("</a", "").replace("&amp;", "&").replace("&#198;", QString::fromUtf8("Æ")).replace("&#229;", QString::fromUtf8("å")).replace("&#230;", QString::fromUtf8("æ")).replace("&#248;", QString::fromUtf8("ø"));
				QString h2Id = line.split("\"")[1].split("#")[1];
				if ( h2Id == "partition" and h2.contains("GParted") )
					h2Id = "partitionGp";
				if( !h2Id.contains("table-contents") ) {
					QTreeWidgetItem *subItem = new QTreeWidgetItem(item);
					subItem->setText( 0,  h2 );
					subItem->setText( 1,  h2Id );
					h2List.append(h2);
					h2IdList.append(h2Id);
				}
				line = stream.readLine();
			}

		}
	}


	// get entries
	// ------------------------------------------------------------------------------
	foreach(QString page, pages ) {

		QFile file2( manpath2+page );
		file2.open( QIODevice::ReadOnly | QIODevice::Text );
		QTextStream stream( &file2 );
		QStringList chapters;
		QString pageContent;
		while ( !stream.atEnd() )
			pageContent += stream.readLine()+"\n";

		chapters = pageContent.split("<div class=\"divider\"");
		if( chapters.count() > 1 )
			chapters.removeFirst();

		foreach(QString chapter, chapters) {
			QString id      = chapter.split("\"")[1];
			QString title   = chapter.split(">")[2];
			title.replace("</h2","");
			QString content = chapter;
			content.replace("id=\""+id+"\">","");
			content.replace(QRegExp("<div id=\"rev\">.*</div>"), "");
			content.replace("../lib/", "/usr/share/sidux-manual/lib/");


			if( !id.contains ("W3C//DTD") and id != "table-contents" ) {
				QTreeWidgetItem *item = new QTreeWidgetItem(chaptersTreeWidget);
				title.replace("</h3", "");
				item->setText( 0,  title );
				item->setText( 1,  id );
				item->setText( 2,  content );
			}
		}
	}

	noHistory = true;
	showIndex();
}
