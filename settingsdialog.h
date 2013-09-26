#ifndef __SETTINGSDIALOG_H
#define __SETTINGSDIALOG_H

#include "ui_settingsdialog.h"

#include <QSettings>
#include <QColor>
#include <QDialog>

class SettingsDialog : public QDialog, public Ui_SettingsDialog {
	Q_OBJECT

	public:
	SettingsDialog(QWidget *parent);
	
	QPoint position ();
	void setPosition (const QPoint &p);

	QSize size ();
	void setSize (const QSize &s);

	QFont schemeFont ();
	void setSchemeFont (const QFont &f);

	QColor selectionColor ();
	void setSelectionColor (const QColor&c);

	QColor invalidColor ();
	void setInvalidColor (const QColor &c);

	QColor executionColor ();
	void setExecutionColor (const QColor &c);
	
	int executionSleep ();
	void setExecutionSleep (int v);

	QString putHostName ();
	void setPutHostName (const QString &);
	
	int putPortNumber ();
	void setPutPortNumber (int);
	
	QString putAppName ();
	void setPutAppName (const QString &a);

	public slots:
	int exec ();
	virtual void accept ();
	void on_selectColorPB_clicked ();
	void on_invalidColorPB_clicked ();
	void on_selectFontPB_clicked ();
	void on_executionColorPB_clicked ();
	
	private:
	Ui::SettingsDialog ui;
	QSettings *appSettings;
	QColor tmpColor;
	QColor tmpInvalidColor;
	QColor tmpExecutionColor;
	QFont tmpSchemeFont;
	int tmpExecutionSleep;

	private slots:
};

extern SettingsDialog *settingsDialog;
#endif

