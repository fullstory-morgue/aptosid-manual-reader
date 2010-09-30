/*
aptosid Manual Reader
Copyright (C) 2009 Fabian Wuertz
Coryright (C) 2010 Nikolas Poniros
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
#include <QDebug>

Smr::Smr(QStringList args, QMainWindow* parent, Qt::WFlags flags): QMainWindow (parent, flags) {

	manpath = "/usr/share/aptosid-manual/";

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
	QSettings settings("aptosid-manual-reader");
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
	//content.prepend("<head><link href=\"/usr/share/aptosid-manual-reader/css/content.css\" rel=\"stylesheet\" type=\"text/css\" /></head>");
	//content.prepend("<head><link href=\"/home/edhunter/fullstory-svn/aptosid-manual-reader/src/content.css\" rel=\"stylesheet\" type=\"text/css\" /></head>");
	content.prepend("<head><link href=\"file:///usr/share/aptosid-manual/lib/css-js/content.css\" rel=\"stylesheet\" type=\"text/css\" /></head>");
	// create alters
	content.replace( QRegExp("<p class=\"highlight-2\">([^<]*)</p>"), "<table class=alert><tr><td class=noBorder><img src=\"/usr/share/aptosid-manual-reader/icons/software-update-urgent.svg\"></td><td class=noBorder>\\1</td></tr></table>");
	content.replace( QRegExp("<div class=\"highlight-2\">([^<]*)</div>"), "<table class=alert><tr><td class=noBorder><img src=\"/usr/share/aptosid-manual-reader/icons/software-update-urgent.svg\"></td><td class=noBorder>\\1</td></tr></table>");

	
	if( searchLineEdit->text() != "" && searchLineEdit->text() != "Type your keyword search here") {
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
	qDebug() << "Id is" << id;
	qDebug() << "Current Chapter is" << currentChapter ;
	// Make sure that the currentChapter actually exists
	if( currentChapter < 0 ) {
		QMessageBox::critical(this, tr("Error"), tr("The current chapter is invalid. Will return to Index. Please inform the developers about this. Thank you!") );
		showIndex();
		return;
	} 
	
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
	if( id == "netcardconfig") {
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
	id.replace("file:///usr/share/aptosid-manual/"+lang+"/", "");

	if( id.contains("http") ) {
		QDesktopServices::openUrl(id);
		return;
	}

	if( !id.contains("#") ) {
		if( chaptersTreeWidget->findItems(id, Qt::MatchContains, 2 ).count() < 1 ) 
			id = "missing";
		else{
			if( chaptersTreeWidget->findItems(id, Qt::MatchExactly, 2).count() > 0 ){
				id = chaptersTreeWidget->findItems(id, Qt::MatchContains, 2).first()->text(1); 
			}else{
				QMessageBox::critical(this, tr("Error"), tr("An error occured while trying to load a new chapter. Please inform the developers about this. Thank you!") );
				qDebug() << "Error loading chapter" ;
				qDebug() << "Id is" << id;
				showIndex();
				return;
			}
		}
	}
	id = id.split("#")[1];
	backHistory.prepend( h2IdList[currentChapter] );
	forwardHistory.clear();
	qDebug() << "Calling showChapter(id) with" << id;
	showChapter( id );
	qDebug() << "Call succeeded";
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
	if( execPushButton->text().contains("Ceni") ) {
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

void Smr::aboutaptosid() {
	if(!noHistory )
		backHistory.prepend( h2IdList[currentChapter] );
	forwardHistory.clear();
	showChapter( "cred-team" );
}

void Smr::aboutReader() {
	if(!noHistory )
		backHistory.prepend( h2IdList[currentChapter] );
	webView->setHtml("<h2>aptosid Manual Reader</h2><b>"+tr("Programmers")+":</b> Fabian Wuertz xadras@aptosid.com <br> Nikolas Poniros edhunter@aptosid.com<br><b>"+tr("License")+":</b> GPL2<br>");
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

	if( !QFile::exists( "/usr/share/doc/aptosid-manual-"+lang+"/copyright" ) ) {
		QString s = tr("This language is not installed! If you want to use it install it with") + " apt-get install aptosid-manual-" + lang;
		QMessageBox::warning(this, tr("Warning"), s);
		return;
	}

	QSettings settings("aptosid-manual-reader");
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

	getMenu(manpath2, pages[0]);

	foreach(QString page, pages )
		getEntries(page, manpath2);

	noHistory = true;
	showIndex();
}

void Smr::getMenu(QString manpath2, QString page)
{
	// get h1 
	QFile file1( manpath2+page );
	file1.open( QIODevice::ReadOnly | QIODevice::Text );
	QTextStream stream( &file1 );
	QString tmp;
	QStringList titlePath, titleName;

	
	// get menu
	// ------------------------------------------------------------------------------
	QString line = stream.readLine();
	while ( !stream.atEnd() ) {
	
		if( line.contains("showHide") ) {
			QString h1   = line.split(">")[2].replace("</a", "").replace("&amp;", "&").replace("&#230;", QString::fromUtf8("æ") ).replace("&#8482;", QString::fromUtf8("™"));
			QString h1Id = line.split("\"")[5];
			QTreeWidgetItem *item = new QTreeWidgetItem(indexTreeWidget);
			item->setText( 0,  h1 );
			item->setText( 1,  h1Id ); 
			stream.readLine();
			line = 	stream.readLine();
			while( ! line.contains("showHide"))
			{
				if ( line.contains("<li><a href=")){
					if( line.contains(".htm\"") )
						line.replace( ".htm", ".htm#misssing" );
					QString h2   = line.split(">")[2].replace("</a", "").replace("&amp;", "&").replace("&#198;", QString::fromUtf8("Æ")).replace("&#229;", QString::fromUtf8("å")).replace("&#230;", QString::fromUtf8("æ")).replace("&#248;", QString::fromUtf8("ø")).replace("&#8482;", QString::fromUtf8("™"));
					QString h2Id = line.split("\"")[1].split("#")[1];
					if ( h2Id == "partition" and h2.contains("GParted") )
						h2Id = "partitionGp";
					if( !h2Id.contains("table-contents") ) {
						QTreeWidgetItem *subItem = new QTreeWidgetItem(item);
						//--- used to read the second level of the menu
						if ( h2.contains("&#8658"))
						{
							h2.replace("&#8658;", "");
							subItem->setText( 0,  h2 );
							subItem->setText( 1,  h2Id );
							h2List.append(h2);
							h2IdList.append(h2Id);
							line = stream.readLine();
							while( ! line.contains("</ul>"))
							{
								if ( line.contains("<li><a href=") )
								{
								if( line.contains(".htm\"") )
									line.replace(".htm", ".htm#missing" );
								QString h3 = line.split(">")[2].replace("</a", "").replace("&amp;", "&").replace("&#198;", QString::fromUtf8("Æ")).replace("&#229;", QString::fromUtf8("å")).replace("    &#230;", QString::fromUtf8("æ")).replace("&#248;", QString::fromUtf8("ø"));
								QString h3Id = line.split("\"")[1].split("#")[1];
								if ( h3Id == "partition" and h3.contains("GParted") )
									h2Id = "partitionGp";
								if( !h3Id.contains("table-contents") ) {
								QTreeWidgetItem *subSubItem = new QTreeWidgetItem(subItem);
								subSubItem->setText( 0, h3 );
								subSubItem->setText( 1, h3Id );
								h2List.append(h3);
								h2IdList.append(h3Id);
								}
								}
								line = stream.readLine();
							}
						//---
						} else {
							subItem->setText( 0, h2 );
							subItem->setText( 1, h2Id );
							h2List.append(h2);
							h2IdList.append(h2Id);
						}
					}
				}
				
				line = stream.readLine();
				if ( line.contains("<!-- menu ends here -->"))
					break;
			}
	
		}

		if ( ! line.contains("showHide"))
		{
			line = stream.readLine();
		}
	}
}

void Smr::getEntries(QString page, QString manpath2)
{
	QFile file( manpath2 + page );
	file.open( QIODevice::ReadOnly | QIODevice::Text );
	QTextStream stream(&file);
	QString id, title, content, line, contentNext;
	QStringList idList, titleList;
	content = "";
	contentNext = "";
	line = stream.readLine();
	//Ignore menu
	while (!line.contains("<!-- menu ends here -->") )
		line = stream.readLine();
	bool needTitle = false;
	bool nextLoop = false;
	/* 
		The idea is: read the whole page line by line and search for <div class="divider" lines and from there get the id. If the divider id is not already in h2IdList add it (if it is already in there it means that the menu has this id in it) if its already in break saving the id and then the title and content for the next loop. For ids that are not in the menu of the manual dont split the divider into chapters, read it in the chapter before but for all ids/titles create entry in the treewidget so we dont loose content/ids.
	*/
	while( !stream.atEnd() )
	{
		id = "";
		while( !h2IdList.contains(id) && !stream.atEnd() )
		{

			if(line.contains("<div class=\"divider\"") )
			{
				id = line.split("\"")[3];
				if ( h2IdList.contains(id) )
					nextLoop = true;
				else
				{
					h2IdList.append(id);
				//	qDebug() << "ID is" << id;
					idList.append(id);
					needTitle = true;
					// reset ID so I go in the loop again
					id = "";
				}
			}

			if(nextLoop)
				contentNext = contentNext + line;
			else
				//Have this here to first check if the current line is an id line
				content = content + "\n" + line;
			
			while( !line.contains(QRegExp("<h[0-9].*>.*</h[0-9]>") ) && needTitle )
			{
				content = content + line;
			    line = stream.readLine();
			}
			
			if(line.contains(QRegExp("<h[0-9].*>.*</h[0-9]>") ) && needTitle )
			{
				needTitle = false;
				//qDebug() << "line is" << line;
				title = line.split(">")[1];
				title.replace(QRegExp("</h[0-9]"), "");
				title.replace("</a", "").replace("&amp;", "&").replace("&#198;", QString::fromUtf8("Æ")).replace("&#229;", QString::fromUtf8("å")).replace("    &#230;", QString::fromUtf8("æ")).replace("&#248;", QString::fromUtf8("ø")).replace("&#8482;", QString::fromUtf8("™"));

				//Wait till the end of the loop to add the title as it belongs to the next chapter
				if (!nextLoop){
					if ( !h2List.contains(title) )
						h2List.append(title);
					titleList.append(title);
					contentNext = contentNext + line;
				}
				content = content + line;
				//qDebug() << "Title is" << title;
			}
			line = stream.readLine();
		}

		for (int i = 0; i < idList.size(); i++ )
		{
			content.replace("id=\"" + idList[i] + "\">", "");
		}
		content.replace(QRegExp("<div id=\"rev\">.*</div>"), "");
		content.replace("../lib/", "file:///" + manpath + "lib/");

		for (int i = 0; i < idList.size(); i++)
		{
			if( !idList[i].contains("W3C//DTD") && idList[i] != "table-contents" )
			{
				QTreeWidgetItem *item = new QTreeWidgetItem(chaptersTreeWidget);
				item->setText( 0,  titleList[i] );
				item->setText( 1,  idList[i] );
				item->setText( 2,  content );
			}
		}
		
		idList.clear();
		titleList.clear();
		//Do this so we dont loose id/title/content
		idList.append(id);
		titleList.append(title);
		content = contentNext;
		contentNext = "";
		line = stream.readLine();
		nextLoop = false;
	}
}
