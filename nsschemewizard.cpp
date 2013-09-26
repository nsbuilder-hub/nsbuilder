#include "nsschemewizard.h"
#include "nsscheme.h"

NSSchemeWizard *schemeWizard = 0;

NSSchemeWizard::NSSchemeWizard(QWidget *parent)
:QDialog(parent)
{
	ui.setupUi (this);
}

void NSSchemeWizard::fillDataFrom (NSScheme *s)
{
	setSchemeAuthor (s->schemeAuthor ());
	setSchemeName (s->schemeName ());
	setSchemeComment (s->schemeComment ());
	setSchemeType (s->schemeType ());
	setCheckSyntax (s->checkSyntax ());
	setContainsPascalCode (s->containsPascalCode ());
}

void NSSchemeWizard::setDataTo (NSScheme *s)
{
	s->setSchemeAuthor (schemeAuthor ());
	s->setSchemeName (schemeName ());
	s->setSchemeComment (schemeComment ());
	s->setSchemeType (schemeType ());
	s->setCheckSyntax (checkSyntax ());
	s->setContainsPascalCode (containsPascalCode ());
}

void NSSchemeWizard::reset ()
{
	setSchemeAuthor (QString::null);
	setSchemeName (QString::null);
	setSchemeComment (QString::null);
	setSchemeType (SEQUENCE);
	setCheckSyntax (true);
	setContainsPascalCode (true);
}

void NSSchemeWizard::on_sequenceRB_clicked (bool v)
{
	if (v) 
		m_schemeType = SEQUENCE;
}

void NSSchemeWizard::on_procedureRB_clicked (bool v)
{
	if (v) 
		m_schemeType = PROCEDURE;
}

void NSSchemeWizard::on_functionRB_clicked (bool v)
{
	if (v) 
		m_schemeType = FUNCTION;
}

NSSchemeWizard::SchemeType NSSchemeWizard::schemeType ()
{
	return m_schemeType;
}

void NSSchemeWizard::setSchemeType (NSSchemeWizard::SchemeType t)
{
	m_schemeType = t;
	switch (m_schemeType) {
		case SEQUENCE:
			ui.sequenceRB->setChecked (true);
			break;
		case PROCEDURE:
			ui.procedureRB->setChecked (true);
			break;
		case FUNCTION:
			ui.functionRB->setChecked (true);
			break;
	}
}
void NSSchemeWizard::on_schemeNameLE_textChanged (const QString &s)
{
	m_schemeName = s;
}

void NSSchemeWizard::on_schemeCommentTE_textChanged ()
{
	m_schemeComment = ui.schemeCommentTE->toPlainText ();
}

void NSSchemeWizard::on_schemeAuthorLE_textChanged (const QString &s)
{
	m_schemeAuthor = s;
}

QString NSSchemeWizard::schemeAuthor ()
{
	return m_schemeAuthor;
}

void NSSchemeWizard::setSchemeAuthor (const QString& s)
{
	m_schemeAuthor = s;
	ui.schemeAuthorLE->setText (m_schemeAuthor);
}

QString NSSchemeWizard::schemeName ()
{
	return m_schemeName;
}

void NSSchemeWizard::setSchemeName (const QString& s)
{
	m_schemeName = s;
	ui.schemeNameLE->setText (m_schemeName);
}

QString NSSchemeWizard::schemeComment ()
{
	return m_schemeComment;
}

void NSSchemeWizard::setSchemeComment (const QString& s)
{
	m_schemeComment = s;
	ui.schemeCommentTE->setPlainText (m_schemeComment);
}
	
QString NSSchemeWizard::typeToString (NSSchemeWizard::SchemeType t)
{
	if (t == NSSchemeWizard::SEQUENCE) {
		return "sequence";
	} else if (t == NSSchemeWizard::PROCEDURE) {
		return "procedure";
	} else if (t == NSSchemeWizard::FUNCTION) {
		return "function";
	}

	return QString::null;
}

NSSchemeWizard::SchemeType NSSchemeWizard::stringToType (const QString &s)
{
	if (s.toLower () == "sequence")
		return NSSchemeWizard::SEQUENCE;
	else if (s.toLower () == "procedure")
		return NSSchemeWizard::PROCEDURE;
	else if (s.toLower () == "function") 
		return NSSchemeWizard::FUNCTION;
		
	return NSSchemeWizard::SEQUENCE;
}

bool NSSchemeWizard::containsPascalCode ()
{
	return m_pascalCode;
}

void NSSchemeWizard::setContainsPascalCode (bool v)
{
	m_pascalCode = v;
	ui.pascalCodeGB->setChecked (v);
}

bool NSSchemeWizard::checkSyntax ()
{
	return m_checkSyntax;
}

void NSSchemeWizard::setCheckSyntax (bool v)
{
	m_checkSyntax = v;
	ui.checkSyntaxCB->setChecked (v);
}

void NSSchemeWizard::on_checkSyntaxCB_clicked (bool v)
{
	setCheckSyntax (v);
}

void NSSchemeWizard::on_pascalCodeGB_clicked (bool v)
{
	setContainsPascalCode (v);
}

