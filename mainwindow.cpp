#include <QtGui>
#include <QPlastiqueStyle>
#include <unistd.h>

#include "mainwindow.h"
#include "nsscheme.h"
#include "instruction.h"
#include "questiondialog.h"
#include "instructionwizard.h"
#include "nsschemewizard.h"
#include "settingsdialog.h"
#include "executionthread.h"
#include "variableeditor.h"
#include "exprtree.h"
#include "navevaluedialog.h"

MainWindow *mainWin;

NSScheme *MainWindow::findSchemeForName (const QString &name)
{
#ifdef DEBUGE
qDebug("[ENTER] MainWindow::findSchemeForName");
#endif
	NSScheme *result = 0;

	QList<QWidget *> windows = workspace->windowList();
	for (int i = 0; (i < windows.size ()) && (! result); ++i) {
		NSScheme *child = qobject_cast<NSScheme *>(windows.at (i));

                if (child && (child->schemeName ().section ('(', 0, 0) == name))
			result = child;
        }
#ifdef DEBUGE
qDebug("[EXIT] MainWindow::findSchemeForName result=%p", result);
#endif
	return result;
}

MainWindow::MainWindow():
	execThread(0)
{
	initiate_builtin_functions ();

	settingsDialog = new SettingsDialog(this);
	QApplication::setStyle (new QPlastiqueStyle);

	workspace = new QWorkspace;
	setCentralWidget (workspace);
	workspace->setScrollBarsEnabled (true);
	connect(workspace, SIGNAL(windowActivated(QWidget *)),
		this, SLOT(updateMenus()));
	connect(workspace, SIGNAL(windowActivated(QWidget *)),
		this, SLOT(rereadVariables()));
	windowMapper = new QSignalMapper(this);
	connect(windowMapper, SIGNAL(mapped(QWidget *)),
		workspace, SLOT(setActiveWindow(QWidget *)));

	createActions ();
	createMenus ();
	createToolBars ();
	createStatusBar ();
	updateMenus ();

	on_thread_finished ();

	readSettings();

	setWindowTitle(tr("NS Builder"));

	instructionWizard = new ::InstructionWizardDialog(this);
	QuestionDialog::initWidgets (this);

	m_variableEditor = new VariableEditor(this);
	m_variableEditor->setVisible (false);

#ifdef Q_WS_X11
	m_lastUsedDir = get_current_dir_name ();
#endif

	p_clipboardInstruction = 0;

	for (int i = 1; i < qApp->argc (); i++) {
		do_open (qApp->argv ()[i]);
	}

        setAcceptDrops (true);

        m_nameValueDialog = new NaveValueDialog(this);
}

QMap<QString, QString>&  MainWindow::getNameValueMap ()
{
    return m_nameValueDialog->nameValueMap();
}

MainWindow::~MainWindow()
{
	delete m_variableEditor;
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    qDebug() << event->mimeData()->formats();
        if (event->mimeData()->hasUrls())
            event->acceptProposedAction();
}

void MainWindow::dropEvent(QDropEvent *event)
{
     if (event->mimeData()->hasUrls()) {
         foreach (QUrl url, event->mimeData()->urls()) {
             qDebug() << url;
             do_open (url.path ());
         }
     }
}

QString MainWindow::lastUsedDir () const
{
	return m_lastUsedDir;
}

void MainWindow::setLastUsedDir (const QString& s)
{
	m_lastUsedDir = s;
}

void MainWindow::closeEvent (QCloseEvent *event)
{
	workspace->closeAllWindows();
	m_variableEditor->hide ();
	if (activeNSScheme()) {
		event->ignore();
	} else {
		writeSettings();
		event->accept();
	}
}

void MainWindow::newFile ()
{
	schemeWizard->reset ();

	if (QuestionDialog::showSchemeWizard () == QDialog::Accepted) {
		NSScheme *child = createNSScheme (schemeWizard);
		child->newFile ();
		child->show ();
	}
}

void MainWindow::do_open (const QString& fileName)
{
	NSScheme *existing = findNSScheme (fileName);

	if (existing) {
		workspace->setActiveWindow (existing);
		return;
	}

	NSScheme *child = createNSScheme ();
	if (child->loadFile (fileName)) {
		statusBar ()->showMessage (tr("File loaded"), 2000);
		child->show ();
	} else {
		child->close ();
                delete child;
	}
}

void MainWindow::open ()
{
	QFileDialog fd(this);
	fd.setAcceptMode (QFileDialog::AcceptOpen);
	fd.setFilters (QStringList() << tr("NS schemes (*.nss)") << tr("Any files (*.*)"));
        fd.setFileMode (QFileDialog::ExistingFiles);
	fd.setDirectory (lastUsedDir ());
	
        if ((fd.exec () == QDialog::Accepted)) {
                foreach (QString fn, fd.selectedFiles ()) {
                        do_open (fn);
                }
	}
	
	setLastUsedDir (fd.directory ().path ());
}

void MainWindow::save ()
{
	bool hasNSScheme = (activeNSScheme () != 0);
	if (hasNSScheme) {
		if (activeNSScheme ()->save ())
			statusBar ()->showMessage (tr("File saved"), 2000);
	}
}

void MainWindow::saveAs ()
{
	bool hasNSScheme = (activeNSScheme() != 0);
	if (hasNSScheme) {
		if (activeNSScheme ()->saveAs ())
			statusBar ()->showMessage (tr("File saved"), 2000);
	}
}

void MainWindow::saveAll ()
{
    QList<QWidget *> windows = workspace->windowList();

    for (int i = 0; i < windows.size(); ++i) {
            NSScheme *child = qobject_cast<NSScheme *>(windows.at(i));
            if (! child)
                continue;

            child->save ();
    }
}

void MainWindow::undo ()
{
    bool hasNSScheme = (activeNSScheme() != 0);
    if (hasNSScheme) {
            activeNSScheme ()->undo ();
            updateMenus();
    }
}

void MainWindow::cut ()
{
	bool hasNSScheme = (activeNSScheme() != 0);
	if (hasNSScheme) {
		activeNSScheme ()->cut ();
                updateMenus();
	}
}

void MainWindow::copy ()
{
	bool hasNSScheme = (activeNSScheme() != 0);
	if (hasNSScheme) {
		activeNSScheme ()->copy ();
                updateMenus();
        }
}

void MainWindow::paste ()
{
	bool hasNSScheme = (activeNSScheme() != 0);
	if (hasNSScheme) {
		activeNSScheme ()->paste ();
                updateMenus();
	}
}

void MainWindow::settings ()
{
	if (settingsDialog->exec () == QDialog::Accepted) {
		foreach (QWidget *window, workspace->windowList ()) {
			NSScheme *mdiChild = qobject_cast<NSScheme*> (window);
			mdiChild->setFont (settingsDialog->schemeFont ());
		}
	}
}

void MainWindow::about ()
{
    QString aboutTemplate = tr("<h1>This is NS scheme builder.</h1><p>Author: Aleksander Wojdyga<br/>Version: %1<br/>URL: <a href=\"https://code.google.com/p/nsbuilder/\">https://code.google.com/p/nsbuilder/</a></p><p>Windows installer: Jakub Kosid&#0322;o</p>");
    QMessageBox::about (this, tr("About NS Builder"), aboutTemplate.arg(SVN_VER));
}

void MainWindow::appendInstruction ()
{
	bool hasNSScheme = (activeNSScheme() != 0);
	if (hasNSScheme) {
		activeNSScheme ()->appendInstruction ();
                updateMenus();
	}
}

void MainWindow::insertInstruction ()
{
	bool hasNSScheme = (activeNSScheme() != 0);
	if (hasNSScheme) {
		activeNSScheme ()->insertInstruction ();
                updateMenus();
	}
}

void MainWindow::editInstruction ()
{
	bool hasNSScheme = (activeNSScheme() != 0);
	if (hasNSScheme) {
		activeNSScheme ()->editInstruction ();
                updateMenus();
	}
}

void MainWindow::removeInstruction ()
{
	bool hasNSScheme = (activeNSScheme() != 0);
	if (hasNSScheme) {
		activeNSScheme ()->removeInstruction ();
                updateMenus();
	}
}

void MainWindow::editScheme ()
{
	NSScheme *s;
	
	if ((s = activeNSScheme ())) {
		schemeWizard->fillDataFrom (s);

		if (QuestionDialog::showSchemeWizard (tr("Edit scheme properties")) == QDialog::Accepted) {
			schemeWizard->setDataTo (s);
		}
	}
}

void MainWindow::editVariables ()
{
	NSScheme *s;
	
	if ((s = activeNSScheme ())) {
		s->assignArraySizes ();
		m_variableEditor->editSchemeVariables (s);
	}
}

void MainWindow::exportScheme()
{
	bool hasNSScheme = (activeNSScheme() != 0);
	if (hasNSScheme) {
		activeNSScheme ()->exportScheme ();
	}
}

void MainWindow::exportSchemeSVG()
{
	bool hasNSScheme = (activeNSScheme() != 0);
	if (hasNSScheme) {
		activeNSScheme ()->exportSchemeSVG ();
	}
}

void MainWindow::putSchemeSVG()
{
	NSScheme *s = activeNSScheme();
	bool hasNSScheme = (s != 0);
	if (hasNSScheme) {
		if (s->save ())
			s->putSchemeSVG ();
	}
}

void MainWindow::putSchemeNSS()
{
	NSScheme *s = activeNSScheme();
	bool hasNSScheme = (s != 0);
	if (hasNSScheme) {
                if (s->save ())
                        if (m_nameValueDialog->exec() == QDialog::Accepted)
                            s->putSchemeNSS ();
	}
}

void MainWindow::emailScheme()
{
	bool hasNSScheme = (activeNSScheme() != 0);
	if (hasNSScheme) {
		activeNSScheme ()->emailScheme ();
	}
}

void MainWindow::updateMenus()
{
	bool hasNSScheme = (activeNSScheme() != 0);
	saveAct->setEnabled(hasNSScheme);
	saveAsAct->setEnabled(hasNSScheme);
        undoAct->setEnabled(hasNSScheme && activeNSScheme()->hasUndoneCommands ());
	pasteAct->setEnabled(hasNSScheme);
	closeAct->setEnabled(hasNSScheme);
	closeAllAct->setEnabled(hasNSScheme);
	tileAct->setEnabled(hasNSScheme);
	cascadeAct->setEnabled(hasNSScheme);
	nextAct->setEnabled(hasNSScheme);
	previousAct->setEnabled(hasNSScheme);
	separatorAct->setVisible(hasNSScheme);
}

void MainWindow::updateWindowMenu()
{
	windowMenu->clear();
	windowMenu->addAction(closeAct);
	windowMenu->addAction(closeAllAct);
	windowMenu->addSeparator();
	windowMenu->addAction(tileAct);
	windowMenu->addAction(cascadeAct);
	windowMenu->addSeparator();
	windowMenu->addAction(nextAct);
	windowMenu->addAction(previousAct);
	windowMenu->addAction(separatorAct);

	QList<QWidget *> windows = workspace->windowList();
	separatorAct->setVisible(!windows.isEmpty());

	for (int i = 0; i < windows.size(); ++i) {
		NSScheme *child = qobject_cast<NSScheme *>(windows.at(i));

		QString text;
		if (i < 9) {
			text = tr("&%1. %2").arg(i + 1).arg(child->userFriendlyCurrentFile());
		} else {
			text = tr("%1. %2").arg(i + 1).arg(child->userFriendlyCurrentFile());
		}
		QAction *action  = windowMenu->addAction (text);
		action->setCheckable (true);
		action->setChecked (child == activeNSScheme ());
		connect(action, SIGNAL(triggered()), windowMapper, SLOT(map()));
		windowMapper->setMapping(action, child);
	}
}

NSScheme *MainWindow::createNSScheme (NSSchemeWizard *wizard)
{
	NSScheme *child = new NSScheme(this);

	if (wizard) {
		wizard->setDataTo (child);
	}

	workspace->addWindow(child);

//	connect(child, SIGNAL(copyAvailable(bool)), cutAct, SLOT(setEnabled(bool)));
//	connect(child, SIGNAL(copyAvailable(bool)), copyAct, SLOT(setEnabled(bool)));

	child->resize (240, 180);

	return child;
}

void MainWindow::createActions()
{
	newAct = new QAction(QIcon(":/images/filenew.png"), tr("&New"), this);
	newAct->setShortcut(tr("Ctrl+N"));
	newAct->setStatusTip(tr("Create a new file"));
	connect(newAct, SIGNAL(triggered()), this, SLOT(newFile()));

	openAct = new QAction(QIcon(":/images/fileopen.png"), tr("&Open..."), this);
	openAct->setShortcut(tr("Ctrl+O"));
	openAct->setStatusTip(tr("Open an existing file"));
	connect(openAct, SIGNAL(triggered()), this, SLOT(open()));

	saveAct = new QAction(QIcon(":/images/filesave.png"), tr("&Save"), this);
	saveAct->setShortcut(tr("Ctrl+S"));
	saveAct->setStatusTip(tr("Save the document to disk"));
	connect(saveAct, SIGNAL(triggered()), this, SLOT(save()));

	saveAsAct = new QAction(QIcon(":/images/filesaveas.png"), tr("Save &As..."), this);
	saveAsAct->setStatusTip(tr("Save the document under a new name"));
	connect(saveAsAct, SIGNAL(triggered()), this, SLOT(saveAs()));

        saveAllAct = new QAction(QIcon(":/images/filesaveall.png"), tr("Save a&ll"), this);
        saveAllAct->setShortcut(tr("Ctrl+Shift+S"));
        saveAllAct->setStatusTip (tr("Save all modified documents"));
        connect(saveAllAct, SIGNAL(triggered()), this, SLOT(saveAll()));

	exitAct = new QAction(tr("E&xit"), this);
	exitAct->setShortcut(tr("Ctrl+Q"));
	exitAct->setStatusTip(tr("Exit the application"));
	connect(exitAct, SIGNAL(triggered()), qApp, SLOT(closeAllWindows()));

	cutAct = new QAction(QIcon(":/images/editcut.png"), tr("Cu&t"), this);
	cutAct->setShortcut(tr("Ctrl+X"));
	cutAct->setStatusTip(tr("Cut the current selection's contents to the "
				"clipboard"));
	connect(cutAct, SIGNAL(triggered()), this, SLOT(cut()));

        undoAct = new QAction(QIcon(":/images/editundo.png"), tr("&Undo"), this);
        undoAct->setShortcut (tr("Ctrl+Z"));
        undoAct->setStatusTip(tr("Undo last command"));
        connect(undoAct, SIGNAL(triggered()), this, SLOT(undo()));

	copyAct = new QAction(QIcon(":/images/editcopy.png"), tr("&Copy"), this);
	copyAct->setShortcut(tr("Ctrl+C"));
	copyAct->setStatusTip(tr("Copy the current selection's contents to the "
				"clipboard"));
	connect(copyAct, SIGNAL(triggered()), this, SLOT(copy()));

	pasteAct = new QAction(QIcon(":/images/editpaste.png"), tr("&Paste"), this);
	pasteAct->setShortcut(tr("Ctrl+V"));
	pasteAct->setStatusTip(tr("Paste the clipboard's contents into the current "
				"selection"));
	connect(pasteAct, SIGNAL(triggered()), this, SLOT(paste()));

	
	settingsAct= new QAction(tr("&Settings"), this);
	settingsAct->setStatusTip (tr("Edit application's settings"));
	connect (settingsAct, SIGNAL(triggered()), this, SLOT(settings()));
	
	closeAct = new QAction(tr("Cl&ose"), this);
	closeAct->setShortcut(tr("Ctrl+W"));
	closeAct->setShortcutContext(Qt::WidgetShortcut);
	closeAct->setStatusTip(tr("Close the active window"));
	connect(closeAct, SIGNAL(triggered()),
			workspace, SLOT(closeActiveWindow()));

	closeAllAct = new QAction(tr("Close &All"), this);
	closeAllAct->setStatusTip(tr("Close all the windows"));
	connect(closeAllAct, SIGNAL(triggered()),
			workspace, SLOT(closeAllWindows()));

	tileAct = new QAction(tr("&Tile"), this);
	tileAct->setStatusTip(tr("Tile the windows"));
	connect(tileAct, SIGNAL(triggered()), workspace, SLOT(tile()));

	cascadeAct = new QAction(tr("&Cascade"), this);
	cascadeAct->setStatusTip(tr("Cascade the windows"));
	connect(cascadeAct, SIGNAL(triggered()), workspace, SLOT(cascade()));

	nextAct = new QAction(tr("Ne&xt"), this);
	nextAct->setShortcut(tr("Ctrl+F6"));
	nextAct->setStatusTip(tr("Move the focus to the next window"));
	connect(nextAct, SIGNAL(triggered()),
			workspace, SLOT(activateNextWindow()));

	previousAct = new QAction(tr("Pre&vious"), this);
	previousAct->setShortcut(tr("Ctrl+Shift+F6"));
	previousAct->setStatusTip(tr("Move the focus to the previous "
				"window"));
	connect(previousAct, SIGNAL(triggered()),
			workspace, SLOT(activatePreviousWindow()));

	separatorAct = new QAction(this);
	separatorAct->setSeparator(true);

	aboutAct = new QAction(tr("&About"), this);
	aboutAct->setStatusTip(tr("Show the application's About box"));
	connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));

	aboutQtAct = new QAction(tr("About &Qt"), this);
	aboutQtAct->setStatusTip(tr("Show the Qt library's About box"));
	connect(aboutQtAct, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

	appendInstrAct = new QAction(QIcon(":/images/append.png"), tr("Append instruction"), this);
	appendInstrAct->setStatusTip(tr("Appends instruction to the end of current instruction"));
	connect(appendInstrAct, SIGNAL(triggered()), this, SLOT(appendInstruction()));

	insertInstrAct = new QAction(QIcon(":/images/insert.png"), tr("Insert instruction"), this);
	insertInstrAct->setStatusTip(tr("Inserts instruction before current instruction"));
	connect(insertInstrAct, SIGNAL(triggered()), this, SLOT(insertInstruction()));

	editInstrAct = new QAction(QIcon(":/images/edit.png"), tr("Edit instruction"), this);
	editInstrAct->setStatusTip(tr("Edits current instruction"));
	connect(editInstrAct, SIGNAL(triggered()), this, SLOT(editInstruction()));

	removeInstrAct = new QAction(QIcon(":/images/remove.png"), tr("Remove instruction"), this);
	removeInstrAct->setShortcut(tr("Delete"));
	removeInstrAct->setStatusTip(tr("Removes selected instruction"));
	connect(removeInstrAct, SIGNAL(triggered()), this, SLOT(removeInstruction()));

	editSchemeAct = new QAction(QIcon(":/images/editscheme.png"), tr("Edit scheme properties"), this);
	editSchemeAct->setStatusTip(tr("Edit scheme name and comment"));
	connect(editSchemeAct, SIGNAL(triggered()), this, SLOT(editScheme()));

	editVarsAct = new QAction(QIcon(":/images/editvars.png"), tr("Edit variables"), this);
	editVarsAct->setStatusTip(tr("Edit variables' name and value"));
	connect(editVarsAct, SIGNAL(triggered()), this, SLOT(editVariables()));

	exportAct = new QAction(QIcon(":/images/export.png"), tr("Export scheme"), this);
	exportAct->setStatusTip(tr("Export scheme to PNG image file"));
	connect(exportAct, SIGNAL(triggered()), this, SLOT(exportScheme()));

	exportSVGAct = new QAction(QIcon(":/images/exportsvg.png"), tr("Export scheme to SVG image"), this);
	exportSVGAct->setStatusTip(tr("Export scheme to SVG image file"));
	connect(exportSVGAct, SIGNAL(triggered()), this, SLOT(exportSchemeSVG()));
	
	putSVGAct = new QAction(QIcon(":/images/putsvg.png"), tr("Put SVG scheme file to server"), this);
	exportSVGAct->setStatusTip(tr("Put SVG file to server"));
	connect(putSVGAct, SIGNAL(triggered()), this, SLOT(putSchemeSVG()));
	
	putNSSAct = new QAction(QIcon(":/images/putnss.png"), tr("Put NSS scheme file to server"), this);
	putNSSAct->setStatusTip(tr("Put NSS scheme file to server"));
	connect(putNSSAct, SIGNAL(triggered()), this, SLOT(putSchemeNSS()));

	emailAct = new QAction(QIcon(":/images/mailsend.png"), tr("Email scheme"), this);
	emailAct->setStatusTip(tr("Email this scheme as attachment"));
	connect(emailAct, SIGNAL(triggered()), this, SLOT(emailScheme()));

	runAct = new QAction(QIcon(":/images/run.png"), tr("Run"), this);
	runAct->setShortcut (tr("F9"));
	runAct->setStatusTip (tr("Run scheme"));
	connect(runAct, SIGNAL(triggered()), this, SLOT(run()));

	stopAct = new QAction(QIcon(":/images/stop.png"), tr("Stop"), this);
	stopAct->setStatusTip (tr("Stop execution"));
	connect(stopAct, SIGNAL(triggered()), this, SLOT(stop()));
	
	nextInstrAct = new QAction(QIcon(":/images/next.png"), tr("Next"), this);
	nextAct->setShortcut (tr("F8"));
	nextInstrAct->setStatusTip (tr("Next instruction"));
	connect(nextInstrAct, SIGNAL(triggered()), this, SLOT(next()));
	
	contAct = new QAction(QIcon(":/images/cont.png"), tr("Continue"), this);
	contAct->setStatusTip (tr("Continue execution"));
	connect(contAct, SIGNAL(triggered()), this, SLOT(cont()));
}

void MainWindow::createMenus()
{
	fileMenu = menuBar()->addMenu(tr("&File"));
	fileMenu->addAction(newAct);
	fileMenu->addAction(openAct);
	fileMenu->addAction(saveAct);
	fileMenu->addAction(saveAsAct);
        fileMenu->addAction(saveAllAct);
	fileMenu->addSeparator();
	fileMenu->addAction(exitAct);

	editMenu = menuBar()->addMenu(tr("&Edit"));
        editMenu->addAction(undoAct);
        editMenu->addAction(cutAct);
	editMenu->addAction(copyAct);
	editMenu->addAction(pasteAct);
	editMenu->addAction(settingsAct);

	schemeMenu = menuBar ()->addMenu (tr("&Scheme"));
	schemeMenu->addAction (editSchemeAct);
	schemeMenu->addAction (editVarsAct);
	schemeMenu->addAction (exportAct);
	schemeMenu->addAction (exportSVGAct);
	schemeMenu->addAction (putSVGAct);
	schemeMenu->addAction (putNSSAct);
	//schemeMenu->addAction (emailAct);

	instrMenu = menuBar()->addMenu (tr("&Instructions"));
	instrMenu->addAction (appendInstrAct);
	instrMenu->addAction (insertInstrAct);
	instrMenu->addAction (editInstrAct);
	instrMenu->addAction (removeInstrAct);

	execMenu = menuBar ()->addMenu (tr("E&xecution"));
	execMenu->addAction (runAct);
	execMenu->addAction (stopAct);
	execMenu->addAction (nextInstrAct);
	execMenu->addAction (contAct);

	windowMenu = menuBar()->addMenu(tr("&Window"));
	connect(windowMenu, SIGNAL(aboutToShow()), this, SLOT(updateWindowMenu()));

	menuBar()->addSeparator();

	helpMenu = menuBar()->addMenu(tr("&Help"));
	helpMenu->addAction(aboutAct);
	helpMenu->addAction(aboutQtAct);
}

void MainWindow::createToolBars()
{
	fileToolBar = addToolBar(tr("File"));
	fileToolBar->addAction(newAct);
	fileToolBar->addAction(openAct);
	fileToolBar->addAction(saveAct);
	fileToolBar->addAction(saveAsAct);

	editToolBar = addToolBar(tr("Edit"));
        editToolBar->addAction(undoAct);
	editToolBar->addAction(cutAct);
	editToolBar->addAction(copyAct);
	editToolBar->addAction(pasteAct);

	schemeToolBar = addToolBar(tr("Scheme"));
	schemeToolBar->addAction (editSchemeAct);
	schemeToolBar->addAction (editVarsAct);
	schemeToolBar->addAction (exportAct);
	schemeToolBar->addAction (exportSVGAct);
	schemeToolBar->addAction (putSVGAct);
	schemeToolBar->addAction (putNSSAct);
	//schemeToolBar->addAction (emailAct);

	instrToolBar = addToolBar(tr("Instructions"));
	instrToolBar->addAction (appendInstrAct);
	instrToolBar->addAction (insertInstrAct);
	instrToolBar->addAction (editInstrAct);
	instrToolBar->addAction (removeInstrAct);
	
	execToolbar = addToolBar (tr("Execution"));
	execToolbar->addAction (runAct);
	execToolbar->addAction (stopAct);
	execToolbar->addAction (nextInstrAct);
	execToolbar->addAction (contAct);
}

void MainWindow::createStatusBar()
{
	statusBar()->showMessage(tr("Ready"));
}

void MainWindow::readSettings()
{
	move (settingsDialog->position ());
	resize (settingsDialog->size ());
}

void MainWindow::writeSettings()
{
	settingsDialog->setPosition (pos());
	settingsDialog->setSize (size());
}

NSScheme *MainWindow::activeNSScheme()
{
	if (workspace->activeWindow ())
		return qobject_cast<NSScheme *>(workspace->activeWindow ());
	else 
		return 0;
}

NSScheme *MainWindow::findNSScheme (const QString &fileName)
{
	QString canonicalFilePath = QFileInfo(fileName).canonicalFilePath ();

	foreach (QWidget *window, workspace->windowList ()) {
		NSScheme *mdiChild = qobject_cast<NSScheme*> (window);
		if (mdiChild->currentFile () == canonicalFilePath)
			return mdiChild;
	}
	return 0;
}

Instruction* MainWindow::clipboardInstruction ()
{
	qDebug("MainWindow::clipboardInstruction");
	return p_clipboardInstruction;
}

void MainWindow::setClipboardInstruction (Instruction *i)
{
	if (p_clipboardInstruction)
		delete p_clipboardInstruction;

	p_clipboardInstruction = i;
}

SettingsDialog* MainWindow::getSettingsDialog () const
{
	return settingsDialog;
}

bool MainWindow::createExecutionThread ()
{
	NSScheme *scheme = qobject_cast<NSScheme *>(workspace->activeWindow ());
	if (! scheme) {
                return false;
	}
        if ((scheme->schemeType() == NSSchemeWizard::PROCEDURE) || (scheme->schemeType() == NSSchemeWizard::FUNCTION)) {
            QMessageBox::warning (this, tr("Execution"), tr("Cannot run procedure or function."));
            return false;
        }

	execThread = new ExecutionThread(scheme);
	connect(execThread, SIGNAL(finished()), scheme, SLOT(on_executionThread_finished()));
	connect(execThread, SIGNAL(finished()), this, SLOT(on_thread_finished()));	
#ifdef RUN_IS_STEPPING
        execThread->setSingleStepping (true);
        execThread->executeNextStep ();
#endif
	setExecutionActions (true);

        return true;
}

bool MainWindow::allSchemesSaved ()
{
    foreach (QWidget *window, workspace->windowList ()) {
            NSScheme *scheme = qobject_cast<NSScheme*> (window);

            if (scheme && scheme->modified())
                    return false;
    }

    return true;
}

void MainWindow::run ()
{
	if (workspace->activeWindow ()) {
		NSScheme *s = activeNSScheme ();
		if (! s) {
			QMessageBox::warning (this, tr("Execution"), tr("No active NS scheme"));
			return;
		}
		if (! s->containsPascalCode ()) {
			QMessageBox::warning (this, tr("Execution"), tr("Scheme does not contain Pascal code"));
			return;
		}
                if (! allSchemesSaved ()) {
                        if (QMessageBox::question (this,
                                    tr("Execution"),
                                    tr("<center>All schemes have to be saved before execution.<p>Save all now?"),
                                    QMessageBox::SaveAll | QMessageBox::Cancel,
                                    QMessageBox::Cancel)
                            == QMessageBox::SaveAll) {
                                saveAll();
                                if (! allSchemesSaved ())
                                    return;
                        } else
                                return;
                }
		if (execThread == 0) {
                        if (createExecutionThread ())
                            execThread->start ();
		} else {
			QMessageBox::warning (this, tr("Execution"), tr("Already executing scheme"));
		}
	}
}

void MainWindow::setExecutionActions (bool running)
{
	runAct->setEnabled (! running);
	stopAct->setEnabled (running);
#ifdef RUN_IS_STEPPING
	nextInstrAct->setEnabled (running);
#else
	nextInstrAct->setEnabled (true);
#endif
	contAct->setEnabled (running);
}

void MainWindow::on_thread_finished()
{
	setExecutionActions (false);

        if (execThread) {
                execThread->deleteLater();
        }
//		execThread->wait (1000);
//		delete execThread;
        execThread = 0;
}

void MainWindow::stopExecutionThread ()
{
	if (execThread) {
		execThread->stopSchemeExecution ();
	}
}

void MainWindow::stop ()
{
        stopExecutionThread ();

	//on_thread_finished ();
}

void MainWindow::next ()
{
	if (workspace->activeWindow () == 0) {
		return;
	}
#ifdef RUN_IS_STEPPING
	if (execThread) {
		execThread->executeNextStep ();
	}
#else
	if (execThread) {
		execThread->executeNextStep ();
	} else {
                if (createExecutionThread ()) {
                    execThread->setSingleStepping(true);
                    execThread->executeNextStep();
                    execThread->start();
                }
	}
#endif
}

void MainWindow::cont ()
{
	if (execThread) {
		execThread->setSingleStepping (false);
		execThread->executeNextStep ();
	}
}

void MainWindow::rereadVariables ()
{
	if (m_variableEditor->isVisible ())
		editVariables ();
}

