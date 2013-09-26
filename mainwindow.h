/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation and appearing in the file LICENSE.GPL    *
 *   included in the packaging of this file.                               *
 *                                                                         *
 ***************************************************************************/

/*
 * \author Aleksander Wojdyga <awojdyga@pluton.pol.lublin.pl>
 *
 * http://pluton.pol.lublin.pl/~awojdyga/nsbuilder/
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include <QMap>

class QAction;
class QMenu;
class QWorkspace;
class NSScheme;
class QSignalMapper;
class Instruction;
class NSSchemeWizard;
class SettingsDialog;
class ExecutionThread;
class VariableEditor;
class NaveValueDialog;

class MainWindow : public QMainWindow
{
	Q_OBJECT

	public:
	MainWindow();
	~MainWindow();
	Instruction* clipboardInstruction ();
	void setClipboardInstruction (Instruction *i);
	QString lastUsedDir () const;
	void setLastUsedDir (const QString& d);
	SettingsDialog* getSettingsDialog () const;
	
	NSScheme *findSchemeForName (const QString &name);
        void stopExecutionThread ();

        QMap<QString, QString>& getNameValueMap ();

	protected:
	void closeEvent(QCloseEvent *event);
        void dropEvent(QDropEvent *event);
        void dragEnterEvent(QDragEnterEvent *event);

	private slots:
	void newFile ();
	void open ();
	void save ();
	void saveAs ();
        void saveAll ();
        void undo ();
        void cut ();
	void copy ();
	void paste ();
	void settings ();
	void about ();
	void appendInstruction ();
	void insertInstruction ();
	void editInstruction ();
	void removeInstruction ();
	void editScheme ();
	void editVariables ();
	void exportScheme ();
	void exportSchemeSVG ();
	void putSchemeSVG ();
	void putSchemeNSS ();
	void emailScheme ();
	void updateMenus ();
	void updateWindowMenu ();
	NSScheme *createNSScheme (NSSchemeWizard *wizard = 0);
	void run ();
	void stop ();
	void next ();
	void cont ();
	
	void on_thread_finished();
	void rereadVariables ();
	
	private:
	void do_open (const QString& fileName);
        bool createExecutionThread ();

	void createActions();
	void createMenus();
	void createToolBars();
	void createStatusBar();
	void readSettings();
	void writeSettings();
	NSScheme *activeNSScheme();
	NSScheme *findNSScheme(const QString &fileName);
        bool allSchemesSaved ();

	QWorkspace *workspace;
	QSignalMapper *windowMapper;

	QMenu *fileMenu;
	QMenu *editMenu;
	QMenu *schemeMenu;
	QMenu *instrMenu;
	QMenu *execMenu;
	QMenu *windowMenu;
	QMenu *helpMenu;

	QToolBar *fileToolBar;
	QToolBar *editToolBar;
	QToolBar *instrToolBar;
	QToolBar *schemeToolBar;
	QToolBar *execToolbar;

	QAction *newAct;
	QAction *openAct;
	QAction *saveAct;
	QAction *saveAsAct;
        QAction *saveAllAct;
	QAction *exitAct;
        QAction *undoAct;
	QAction *cutAct;
	QAction *copyAct;
	QAction *pasteAct;
	QAction *settingsAct;
	QAction *closeAct;
	QAction *closeAllAct;
	QAction *tileAct;
	QAction *cascadeAct;
	QAction *nextAct;
	QAction *previousAct;
	QAction *separatorAct;
	QAction *aboutAct;
	QAction *aboutQtAct;
	QAction *appendInstrAct;
	QAction *insertInstrAct;
	QAction *editInstrAct;
	QAction *removeInstrAct;
	QAction *editSchemeAct;
	QAction *editVarsAct;
	QAction *exportAct;
	QAction *exportSVGAct;
	QAction *putSVGAct;
	QAction *putNSSAct;
	QAction *emailAct;
	QAction *runAct;
	QAction *stopAct;
	QAction *nextInstrAct;
	QAction *contAct;

	QString m_lastUsedDir;

	Instruction* p_clipboardInstruction;

	ExecutionThread *execThread;

	VariableEditor *m_variableEditor;

	void setExecutionActions (bool running);

        NaveValueDialog *m_nameValueDialog;
};

extern MainWindow *mainWin;

#endif
