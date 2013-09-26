#include "settingsdialog.h"

#include <QColorDialog>
#include <QFontDialog>
#include <QMessageBox>
#include <QDebug>

SettingsDialog *settingsDialog = 0;

SettingsDialog::SettingsDialog(QWidget *parent)
:QDialog(parent)
{
	ui.setupUi (this);
	appSettings = new QSettings("pollub", "NS Builder");
}

QPoint SettingsDialog::position ()
{
	return appSettings->value("pos", QPoint(200, 200)).toPoint();
}

void SettingsDialog::setPosition (const QPoint &p)
{
	appSettings->setValue ("pos", p);
}

QSize SettingsDialog::size ()
{
	return appSettings->value("size", QSize(400, 400)).toSize();
}

void SettingsDialog::setSize (const QSize &s)
{
	appSettings->setValue ("size", s);
}

QFont SettingsDialog::schemeFont ()
{
	QFont f;
	QString fontName = appSettings->value ("scheme-font").toString ();

	f.fromString (fontName);

	return f;
}

void SettingsDialog::setSchemeFont (const QFont &f)
{
	appSettings->setValue ("scheme-font", f);
}

QColor SettingsDialog::selectionColor ()
{
	QVariant c = QColor(0,255,0);

	QVariant color = appSettings->value ("selection-color", c);

	return color.value<QColor>();
}

void SettingsDialog::setSelectionColor (const QColor &c)
{
	appSettings->setValue ("selection-color", c);
}

QColor SettingsDialog::invalidColor ()
{
	QVariant c = QColor(228,30,30);

	QVariant color = appSettings->value ("invalid-color", c);

	return color.value<QColor> ();
}

void SettingsDialog::setInvalidColor (const QColor &c)
{
	appSettings->setValue ("invalid-color", c);
}

QColor SettingsDialog::executionColor ()
{
	QVariant c = QColor(128,224,0);

	QVariant color = appSettings->value ("execution-color", c);
	
	return color.value<QColor> ();
}

void SettingsDialog::setExecutionColor (const QColor &c)
{
	appSettings->setValue ("execution-color", c);
}

int SettingsDialog::exec ()
{
	ui.fontDisplayLE->setText ((tmpSchemeFont = schemeFont ()).toString ());
	ui.selectColorPB->setPalette (QPalette(tmpColor = selectionColor ()));
	ui.invalidColorPB->setPalette (QPalette(tmpInvalidColor = invalidColor ()));
	ui.executionColorPB->setPalette (QPalette(tmpExecutionColor = executionColor ()));
	
	return QDialog::exec ();
}

void SettingsDialog::on_invalidColorPB_clicked ()
{
	QColor c = QColorDialog::getColor (invalidColor (), this);
	
	if (c.isValid ()) {
		ui.invalidColorPB->setPalette (QPalette(c));
		tmpInvalidColor = c;
	}
}

void SettingsDialog::on_selectColorPB_clicked ()
{
	QColor c = QColorDialog::getColor (selectionColor (), this);
	
	if (c.isValid ()) {
		ui.selectColorPB->setPalette (QPalette(c));
		tmpColor = c;
	}
}

void SettingsDialog::on_executionColorPB_clicked ()
{
	QColor c = QColorDialog::getColor (executionColor (), this);

	if (c.isValid ()) {
		ui.executionColorPB->setPalette (QPalette(c));
		tmpExecutionColor = c;
	}
}

void SettingsDialog::on_selectFontPB_clicked ()
{
	tmpSchemeFont = QFontDialog::getFont (0, schemeFont (), this);
	ui.fontDisplayLE->setText (tmpSchemeFont.toString ());
}

void SettingsDialog::accept ()
{
	setSelectionColor (tmpColor);
	setInvalidColor (tmpInvalidColor);
	setExecutionColor (tmpExecutionColor);
	setSchemeFont (tmpSchemeFont);
	setExecutionSleep (tmpExecutionSleep = ui.executionDelaySB->value ());
	setPutPortNumber (ui.putPortNumberSB->value ());
	setPutHostName (ui.putHostNameLE->text ());
	setPutAppName (ui.putAppNameLE->text ());
	
	QDialog::accept ();
}

int SettingsDialog::executionSleep ()
{
	return appSettings->value ("execution-sleep", 1000).toInt ();
}

void SettingsDialog::setExecutionSleep (int v)
{
	appSettings->setValue ("execution-sleep", v);
}

QString SettingsDialog::putHostName ()
{
        return appSettings->value ("put-host-name", "pluton.cs.pollub.pl").toString ();
}

void SettingsDialog::setPutHostName (const QString &h)
{
	appSettings->setValue ("put-host-name", h);
}

int SettingsDialog::putPortNumber ()
{
        return appSettings->value ("put-port-number", "80").toInt ();
}

void SettingsDialog::setPutPortNumber (int n)
{
	appSettings->setValue ("put-port-number", n);
}

void SettingsDialog::setPutAppName (const QString &a)
{
	appSettings->setValue ("put-app-name", a);
}

QString SettingsDialog::putAppName ()
{
        return appSettings->value ("put-app-name", "/~awojdyga/tablica/").toString ();
}
