#include "nsscheme.h"
#include "instructionwizard.h"
#include "instructioneditor.h"
#include "selection.h"
#include "iteration.h"
#include "simpleinstruction.h"
#include "inputinstruction.h"
#include "outputinstruction.h"
#include "callinstruction.h"
#include "returninstruction.h"
#include "questiondialog.h"
#include "settingsdialog.h"
#include "executionthread.h"

#include <QMessageBox>
#include <QVBoxLayout>
#include <QFile>
#include <QFileInfo>
#include <QFileDialog>
#include <QInputDialog>
#include <QTimer>
#include <QPainter>

// TODO
// rozmiary tablic powinny być do przejrzenia i modyfikacji

int NSScheme::sequenceNumber = 0;

QString escape (const QString s) 
{
	QString r = s;

	r.replace ("&", "&amp;");
	r.replace ("<", "&lt;");
	r.replace (">", "&gt;");

	return r;
}

NSScheme::NSScheme(MainWindow *parent)
        :QWidget(parent), p_mainParent(parent), m_schemeType(NSSchemeWizard::SEQUENCE), m_closing(false), putFile(0)
{
	setObjectName ("nsscheme");

        QPalette p = palette ();
        p.setBrush (QPalette::Background, QBrush(Qt::lightGray));
        setPalette (p);

	setFont (p_mainParent->getSettingsDialog ()->schemeFont ());

        headerLabel = new QLabel();
	headerLabel->setWordWrap (true);
	headerLabel->setTextFormat (Qt::RichText);
	headerLabel->setVisible (true);     

//        proceduralLabel = new QLabel();
//        proceduralLabel->setFrameShape (QFrame::Box);
//        proceduralLabel->setStyleSheet ("background-color: white");
//        proceduralLabel->setTextFormat (Qt::RichText);
//        proceduralLabel->setVisible (false);

        m_instructions = new Sequence(this, 0, 0);
        p_activeInstruction = 0; //m_instructions;

        QVBoxLayout *layout = new QVBoxLayout();
        layout->setSpacing (0);
        layout->setMargin (4);
        layout->addWidget (headerLabel/*, 0*/);
//        layout->addWidget (proceduralLabel, 0);
        layout->addWidget (m_instructions/*, 10*/);

        setLayout (layout);

	m_variables = new ProgramVariables();
        connect (m_variables, SIGNAL(indexOutOfBounds(QString)), this, SLOT(on_indexOutOfBounds(QString)), Qt::BlockingQueuedConnection);

	initQHttp ();
}

NSScheme::~NSScheme()
{
	delete m_variables;
}

void NSScheme::on_indexOutOfBounds (const QString& varName)
{
        QMessageBox::warning(this, tr("Execution error"), tr("Index for array %1 out of bounds").arg(varName), QMessageBox::Ok);
        p_mainParent->stopExecutionThread ();
}

void NSScheme::on_instructionActivated (Instruction *i)
{
	if (p_activeInstruction)
		p_activeInstruction->setActive (false);
	p_activeInstruction = i;
}


QString NSScheme::schemeAuthor () const
{
	return m_author;
}

void NSScheme::updateHeaderLabel ()
{
        if (m_schemeType == NSSchemeWizard::SEQUENCE)
                headerLabel->setText (headerTemplate.arg (m_author).arg (m_schemeComment));
        else
                headerLabel->setText (proceduralTemplate
                                      .arg(m_author)
                                      .arg(m_schemeComment)
                                      .arg(NSSchemeWizard::typeToString(m_schemeType))
                                      .arg(m_schemeName));
        //headerLabel->resize (headerLabel->sizeHint ());
}

void NSScheme::updateProceduralLabel ()
{
        updateHeaderLabel();
//        proceduralLabel->setText (proceduralTemplate.arg (m_schemeName));
//        proceduralLabel->resize (proceduralLabel->sizeHint ());
}

void NSScheme::setSchemeAuthor (const QString& author)
{
	m_author = author;
	updateHeaderLabel ();
	setModified (true);
}
			
QString NSScheme::schemeName () const
{
	return m_schemeName;
}

void NSScheme::setSchemeName (QString name)
{
	m_schemeName = name;
	setObjectName (m_schemeName);
        updateProceduralLabel ();
	setModified (true);
}

QString NSScheme::schemeComment () const 
{
	return m_schemeComment;
}

void NSScheme::setSchemeComment (QString newComment)
{
	m_schemeComment = newComment;
	updateHeaderLabel ();
	setModified (true);
}

NSSchemeWizard::SchemeType NSScheme::schemeType () const
{
	return m_schemeType;
}

void NSScheme::setSchemeType (NSSchemeWizard::SchemeType t)
{
        //if (t == m_schemeType)
        //    return;

        if (m_schemeType == NSSchemeWizard::FUNCTION)
            functionMap.unregisterFunction (this);

        if (m_schemeType == NSSchemeWizard::PROCEDURE)
            functionMap.unregisterProcedure (this);

        // registerFunction i registerProcedure sprawdzają schemeType
        m_schemeType = t;

        if (t == NSSchemeWizard::FUNCTION)
            functionMap.registerFunction (this);

        if (t == NSSchemeWizard::PROCEDURE)
            functionMap.registerProcedure (this);

        if (m_schemeType == NSSchemeWizard::PROCEDURE
            || m_schemeType == NSSchemeWizard::FUNCTION) {
        }

        updateProceduralLabel ();

        setModified (true);
}

Sequence* NSScheme::instructions ()
{
	return m_instructions;
}

void NSScheme::setInstructions (Sequence *s)
{
	if (m_instructions) {
		QLayoutItem *p = layout ()->takeAt (1);
		delete p;
		delete m_instructions;
	}

	m_instructions = s;

	if (m_instructions) {
		layout ()->addWidget (m_instructions);
		m_instructions->setParent (this);
	}
}

Sequence* NSScheme::findParentSequence ()
{
	Sequence *parentSequence = 0;
	
	if ((parentSequence = qobject_cast<Sequence*> (p_activeInstruction))) {
	} else {
		parentSequence = qobject_cast<Sequence*> (p_activeInstruction->parentInstruction ());
	}

	return parentSequence;
}

Instruction* NSScheme::createInstructionFromWizard ()
{
	Instruction *i = 0;
	
        instructionWizard->reset (this);

	if (instructionWizard->execute () == QDialog::Accepted) {
		int itype = instructionWizard->getInstructionType ();

		if (itype & ::InstructionWizardDialog::Clipboard) {
			i = p_mainParent->clipboardInstruction ()->copyOf ();
			i->setScheme (this);
			qDebug("this is clipboard instruction");
		} else {
			qDebug(QString("itype=%1").arg (itype).toLocal8Bit ());
			switch (itype) {
				case ::InstructionWizardDialog::Instruction:
					i = new SimpleInstruction(this, 0, 0);
				break;
				case ::InstructionWizardDialog::Selection:
				{
					Selection *sel = new Selection(this, 0, 0);
					i = sel;
				}
				break;
				case ::InstructionWizardDialog::Iteration:
				{
                                        Iteration *iter = new Iteration(this, Iteration::WHILEDO, 0, 0);
					i = iter;
				}
				break;
				case ::InstructionWizardDialog::Input:
				{
					InputInstruction *ii = new InputInstruction(this, 0, 0);
					i = ii;
				}
				break;
				case ::InstructionWizardDialog::Output:
				{
					OutputInstruction *oi = new OutputInstruction(this, 0, 0);
					i = oi;
				}
				break;
				case ::InstructionWizardDialog::ProcedureCall:
				{
					CallInstruction *ci = new CallInstruction(this, 0, 0);
					i = ci;
				}
                                break;
                                case ::InstructionWizardDialog::FunctionValueReturn:
                                {
                                        ReturnInstruction *ri = new ReturnInstruction(this, 0, 0);
                                        i = ri;
                                }
				break;
			}
		}

		if (i) {
			// Jesli nie jest ze schowka to byla wprowadzona
			// recznie - wtedy ustawmy zawartosc i sprawdzmy jej poprawnosc
			if ((itype & ::InstructionWizardDialog::Clipboard) == 0) {
				i->setContents (instructionWizard->instructionText ());
				i->setComment (instructionWizard->instructionComment ());
				i->setPixmap (instructionWizard->pixmap ());
			}
			i->validateContents ();
			QTimer::singleShot (0, i, SLOT(show()));
			if (! i->valid ()) {
				QTimer::singleShot (0, this, SLOT(showSyntaxError ()));
			}
		}
	}

	return i;
}

void NSScheme::showSyntaxError ()
{
	QMessageBox::warning (this, 
                              tr("Syntax error"),
                              (typeCheckFailed ? tr("Type check failed for identifier %1").arg(syntaxErrorText)
                                               : syntaxErrorText.isEmpty () ?
                                                      tr("Instruction contents too short")
                                                    : tr("String that caused syntax error: %1").arg (syntaxErrorText)),
			      QMessageBox::Ok, 
			      QMessageBox::NoButton);
}

void NSScheme::appendInstruction ()
{
	if (! p_activeInstruction) {
		QMessageBox::warning (this, tr("No sequence"), tr("Select sequence instruction to append"));
		return;
	}
	
	Sequence *parentSequence = findParentSequence ();

	if (parentSequence == 0) {
		return;
	}

	Instruction *i = createInstructionFromWizard ();

	if (i) {
                commands.doAction (Action::newAddInstruction(parentSequence, i, parentSequence->count()));
                //parentSequence->appendInstruction (i);
		setModified (true);
	}
}

void NSScheme::insertInstruction ()
{
	if (! p_activeInstruction) {
		QMessageBox::warning (this, tr("No sequence"), tr("Select sequence instruction to insert"));
		return;
	}
	
	Sequence *parentSequence = findParentSequence ();
	
	if (parentSequence == 0) {
		return;
	}

	Instruction *i = createInstructionFromWizard ();

	if (i) {
		QLayout *l = parentSequence->layout ();

		if (l) {
                        int ind = l->indexOf (p_activeInstruction);
                        ind = ind >= 0 ? ind : 0;
                        commands.doAction (Action::newAddInstruction(parentSequence, i, ind));
                        //parentSequence->insertInstruction (i, ind);
			setModified (true);
		}
	} else {
		qDebug("no instruction created");
	}
}

void NSScheme::editInstruction ()
{
	if (! p_activeInstruction) {
		QMessageBox::warning (this, tr("No instruction"), tr("Select instruction to edit"));
		return;
	}
	
	if (qobject_cast<Sequence*> (p_activeInstruction))
		return;

	editorForm->reset ();
	editorForm->setInstructionText (p_activeInstruction->contents ());
	editorForm->setInstructionComment (p_activeInstruction->comment ());
	editorForm->setPixmap (p_activeInstruction->pixmap ());
	if (QuestionDialog::showEditorForm () == QDialog::Accepted) {
                commands.doAction (Action::newEditInstruction(p_activeInstruction,
                                                              p_activeInstruction->contents(),
                                                              editorForm->instructionText(),
                                                              p_activeInstruction->comment(),
                                                              editorForm->instructionComment()));
                //p_activeInstruction->setContents (editorForm->instructionText ());
                //p_activeInstruction->setComment (editorForm->instructionComment ());
                //p_activeInstruction->setPixmap (editorForm->pixmap ());
		setModified (true);
		p_activeInstruction->validateContents ();
		if (! p_activeInstruction->valid ()) {
			QTimer::singleShot (0, this, SLOT(showSyntaxError ()));
		}
	}
}

void NSScheme::removeInstruction ()
{
	if (! p_activeInstruction) {
		QMessageBox::warning (this, tr("No instruction"), tr("Select instruction to remove"));
		return;
	}
	
	Sequence *parentSequence = findParentSequence ();

	if (parentSequence == 0) {
		return;
	}

	QLayout *l = parentSequence->layout ();

	if (l) {
                commands.doAction (Action::newRemoveInstruction(parentSequence, p_activeInstruction, parentSequence->indexOf(p_activeInstruction)));
                //parentSequence->removeInstructionAt (l->indexOf (p_activeInstruction));
		setModified (true);
	}
}

bool NSScheme::hasUndoneCommands ()
{
    return ! commands.isEmpty ();
}

void NSScheme::exportScheme ()
{
	if (p_activeInstruction) 
		p_activeInstruction->setActive (false);

        QPixmap i = QPixmap::grabWidget (m_instructions);
        QPixmap h = QPixmap::grabWidget (headerLabel);
        QPixmap p(i.width (), i.height () + (m_schemeType == NSSchemeWizard::SEQUENCE ? 0 : h.height ()));
        QPainter painter(&p);
        if (m_schemeType == NSSchemeWizard::SEQUENCE) {
            painter.drawPixmap (0, 0, i);
        } else {
            painter.drawPixmap (0, 0, h);
            painter.drawPixmap (0, h.height (), i);
        }

	QFileDialog fd(this);
	fd.setDirectory (p_mainParent->lastUsedDir ());
	fd.setDefaultSuffix ("png");
	fd.setFilters (QStringList() << tr("PNG images (*.png)") << tr("Any files (*.*)"));
	fd.setAcceptMode (QFileDialog::AcceptSave);

	if (fd.exec () == QDialog::Accepted) {
		QString fn = fd.selectedFiles ().front ();
			
		if (p.save (fn, "PNG")) {
			QMessageBox::information (this, tr("NS Builder - export status"), tr("Export to image file succesful"));
		} else {
			QMessageBox::warning (this, tr("NS Builder - export status"), tr("Export to image file failed"));
		}
	}

	p_mainParent->setLastUsedDir (fd.directory ().path ());

	if (p_activeInstruction) 
		p_activeInstruction->setActive (true);
}

bool NSScheme::do_svgExport (QString svgFileName, QString &errorString)
{
	QDomDocument document = toSVGDocument ();
	QFile svgFile(svgFileName);
	bool retval = true;

	if (svgFile.open (QIODevice::WriteOnly)) {
		if (svgFile.write (document.toByteArray (2)) < 0) {
			retval = false;
		}
		svgFile.close ();
	} else {
		retval = false;
	}
	
	errorString = svgFile.errorString ();
	
	return retval;
}

void NSScheme::exportSchemeSVG ()
{
	if (p_activeInstruction) 
		p_activeInstruction->setActive (false);

	QFileDialog fd(this);
	fd.setDirectory (p_mainParent->lastUsedDir ());
	fd.setDefaultSuffix ("svg");
	fd.setFilters (QStringList() << tr("SVG images (*.svg)") << tr("Any files (*.*)"));
	fd.setAcceptMode (QFileDialog::AcceptSave);

	if (fd.exec () == QDialog::Accepted) {
		QString fn = fd.selectedFiles ().front ();
		QString errorString;
		
		if (do_svgExport (fn, errorString)) {
			QMessageBox::information (this, tr("NS Builder - export status"), tr("Export to SVG image file succesful"));
		} else {
			QMessageBox::warning (
				this, 
				tr("NS Builder - export status"), 
				tr("Export to SVG image file failed:\n%1").arg (errorString)
			);
		}
	}

	p_mainParent->setLastUsedDir (fd.directory ().path ());

	if (p_activeInstruction) 
		p_activeInstruction->setActive (true);

}

void NSScheme::emailScheme ()
{
	// TODO
}

QDomDocument NSScheme::toSVGDocument ()
{
	QDomDocument document("svg");
	
	QDomElement root = document.createElement ("svg");
	QDomAttr width = document.createAttribute ("width");
	width.setValue (QString("%1").arg (this->width ()));
	root.setAttributeNode (width);
	QDomAttr height = document.createAttribute ("height");
	height.setValue (QString("%1").arg (this->height ()));
	root.setAttributeNode (height);
	QDomAttr viewBox = document.createAttribute ("viewBox");
        viewBox.setValue (QString("0 0 %1 %2").arg (this->width ()).arg (this->height () + headerLabel->height ()));
	root.setAttributeNode (viewBox);
	QDomAttr xmlns = document.createAttribute ("xmlns");
	xmlns.setValue ("http://www.w3.org/2000/svg");
	root.setAttributeNode (xmlns);
	QDomAttr version = document.createAttribute ("version");
	version.setValue ("1.2");
	root.setAttributeNode (version);
	QDomAttr fontsize = document.createAttribute ("font-size");
	fontsize.setValue (QString("%1").arg (fontInfo ().pixelSize ()));
	root.setAttributeNode (fontsize);

        if (m_schemeType == NSSchemeWizard::PROCEDURE || m_schemeType == NSSchemeWizard::FUNCTION) {
            QDomElement g = document.createElement ("g");

            g.appendChild(
                        m_instructions->createSVGTextNode(document,0,0,
                                QString("%1 %2").arg(NSSchemeWizard::typeToString(m_schemeType))
                                           .arg(m_schemeName)));
            m_instructions->formatSVGNode (document, g);
	
            root.appendChild (g);
        } else {
            m_instructions->formatSVGNode (document, root);
        }
	document.appendChild (root);
	
	return document;
}

QDomDocument NSScheme::toXMLDocument ()
{
	QDomDocument document("nsscheme");

	QDomElement root = document.createElement ("nsscheme");
	QDomAttr type = document.createAttribute ("type");
	type.setValue (NSSchemeWizard::typeToString (m_schemeType));
	root.setAttributeNode (type);

	document.appendChild (root);

	QDomElement position = document.createElement ("position");
	QDomElement x_pos = document.createElement ("x");
	QDomElement y_pos = document.createElement ("y");
	QDomText x_text = document.createTextNode (QString::number(this->pos (). x ()));
	QDomText y_text = document.createTextNode (QString::number(this->pos (). y ()));
	x_pos.appendChild (x_text);
	y_pos.appendChild (y_text);
	position.appendChild (x_pos);
	position.appendChild (y_pos);
	root.appendChild (position);
	
	QDomElement size = document.createElement ("size");
	QDomElement width = document.createElement ("width");
	QDomElement height = document.createElement ("height");
	QDomText w_text = document.createTextNode (QString::number(this->width ()));
	QDomText h_text = document.createTextNode (QString::number(this->height ()));
	width.appendChild (w_text);
	height.appendChild (h_text);
	size.appendChild (width);
	size.appendChild (height);
	root.appendChild (size);

	QDomElement name = document.createElement ("name");
	QDomText n_text = document.createTextNode (m_schemeName);
	name.appendChild (n_text);
	root.appendChild (name);

	QDomElement comment = document.createElement ("comment");
	QDomText c_text = document.createTextNode (m_schemeComment);
	comment.appendChild (c_text);
	root.appendChild (comment);

	QDomElement author = document.createElement ("author");
	QDomText a_text = document.createTextNode (m_author);
	author.appendChild (a_text);
	root.appendChild (author);
	
	QDomElement pascalCode = document.createElement ("pascalCode");
	QDomAttr contains = document.createAttribute ("contains");
	if (containsPascalCode ()) {
		contains.setValue ("yes");
	} else {
		contains.setValue ("no");
	}
	pascalCode.setAttributeNode (contains);
	root.appendChild (pascalCode);
	
	QDomElement checkSyntax = document.createElement ("checkSyntax");
	QDomAttr checkSyntaxEnabled = document.createAttribute ("enabled");
	if (this->checkSyntax ()) {
		checkSyntaxEnabled.setValue ("yes");
	} else {
		checkSyntaxEnabled.setValue ("no");
	}
	checkSyntax.setAttributeNode (checkSyntaxEnabled);
	root.appendChild (checkSyntax);
	
	m_variables->formatXMLNode (document, root);

	m_instructions->formatXMLNode (document, root);	

	return document;
}

QString NSScheme::currentFile () const
{
	return m_fileName;
}

QString NSScheme::userFriendlyCurrentFile () const
{
	return QFileInfo(m_fileName).fileName ();
}

bool NSScheme::modified ()
{
	return m_modified;
}
	
void NSScheme::setModified (bool v)
{
        if (m_modified == v)
                return;

	m_modified = v;
	
	qDebug(m_modified ? "modified" : "not modified");

	if (m_modified) {
		setWindowTitle (userFriendlyCurrentFile () + " [*]");
	} else {
		setWindowTitle (userFriendlyCurrentFile ());
	}

	setWindowModified (v);
}

void NSScheme::newFile ()
{
	if (! m_schemeName.isEmpty ()) {
		if (m_schemeType == NSSchemeWizard::SEQUENCE)
			m_fileName = QString("%1.nss").arg (m_schemeName);
		else 
			m_fileName = QString("%1.nss").arg (m_schemeName.section ('(', 0, 0));
	} else {
		m_fileName = tr("scheme%1.nss").arg (sequenceNumber++);
	}

	m_isUntitled = true;
	setModified (true);
}

bool NSScheme::save ()
{
	if (m_isUntitled)
		return saveAs ();

	bool retval = true;
	
	assignArraySizes ();

	QDomDocument document = toXMLDocument ();
	QFile xmlFile(m_fileName);

	if (xmlFile.open (QIODevice::WriteOnly)) {
		if (xmlFile.write (document.toByteArray (2)) < 0) {
			QMessageBox::information (this, "NSBuilder - file output error", xmlFile.errorString ());
			retval = false;
		}
		xmlFile.close ();
		setModified (false);
	} else {
		QMessageBox::information (this, "NSBuilder - file output error", xmlFile.errorString ());
		retval = false;
	}

	return retval;
}

bool NSScheme::saveAs ()
{
	bool retval = true;
	
	QFileDialog fd(this);
	fd.setAcceptMode (QFileDialog::AcceptSave);
	fd.setFilters (QStringList() << tr("NS schemes (*.nss)") << tr("Any files (*.*)"));
	fd.setDirectory (p_mainParent->lastUsedDir ());
	fd.selectFile (userFriendlyCurrentFile ());
	fd.setWindowTitle (tr("NSBuilder - choose a filename to save under"));
	
	QString fn;

	if ((fd.exec () == QDialog::Accepted) 
		&& ! (fn = fd.selectedFiles ().front ()).isEmpty ()) {
		m_fileName = fn;
		m_isUntitled = false;
		retval = save ();
	} else {
		retval = false;
	}

	return retval;
}

bool NSScheme::loadFile (const QString& fileName)
{
	QDomDocument doc("nsscheme");
	
	QFile file(fileName);
	
	if (! file.open (QIODevice::ReadOnly)) {
		QMessageBox::warning (this, "NSBuilder - file input error", file.errorString ());
		return false;
	}
	
	if (! doc.setContent (&file)) {
		file.close();
		QMessageBox::warning (this, tr("NSBuilder - file load error"), tr("Error loading NS Scheme xml file"));
		return false;
	}

	file.close();

	m_fileName = fileName;
	m_isUntitled = false;

	return fromXMLDocument (doc);
}

bool NSScheme::fromXMLDocument (QDomDocument& document)
{
	bool retval = true;
	int w = 200, h = 350;
	int x = 0, y = 0;
	
	QDomNode root = document.documentElement ();

	if ((root.toElement ().tagName () == "nsscheme") && root.hasChildNodes ()) {
		QDomAttr type = root.attributes ().namedItem ("type").toAttr ();
		if (! type.isNull ()) {
			setSchemeType (NSSchemeWizard::stringToType (type.value ())); 
                } else {
                        setSchemeType (NSSchemeWizard::SEQUENCE);
                }

		QDomNodeList nodeList = root.childNodes ();

		for (unsigned i = 0; i < nodeList.length (); i++) {
			QDomElement e = nodeList.item (i).toElement ();

			if (! e.isNull ()) {
				if (e.tagName () == "sequence") {
					Sequence *s = new Sequence(this, this, 0);
					setInstructions (s);

					retval = s->setAsXMLNode (e);
				} else if (e.tagName () == "position") {
					QDomNodeList posList = e.childNodes ();

					for (unsigned j = 0; j < posList.length (); j++) {
						QDomElement el = posList.item (j).toElement ();

						if (! el.isNull ()) {
							if (el.tagName () == "x") {
								x = el.firstChild ().toText ().nodeValue ().toInt ();
							} else if (el.tagName () == "y") {
								y = el.firstChild ().toText ().nodeValue ().toInt ();
							}
						}
					}
				} else if (e.tagName () == "size") {
					QDomNodeList posList = e.childNodes ();

					for (unsigned j = 0; j < posList.length (); j++) {
						QDomElement el = posList.item (j).toElement ();

						if (! el.isNull ()) {
							if (el.tagName () == "width") {
								w = el.firstChild ().toText ().nodeValue ().toInt ();
							} else if (el.tagName () == "height") {
								h = el.firstChild ().toText ().nodeValue ().toInt ();
							}
						}
					}
				} else if ((e.tagName () == "name") && e.hasChildNodes ()) {
                                        setSchemeName (e.firstChild ().toText ().nodeValue ());
				} else if ((e.tagName () == "comment") && e.hasChildNodes ()) {
                                        setSchemeComment (e.firstChild ().toText ().nodeValue ());
				} else if ((e.tagName () == "author") && e.hasChildNodes ()) {
                                        setSchemeAuthor (e.firstChild ().toText ().nodeValue ());
				} else if ((e.tagName () == "variables") && e.hasChildNodes ()) {
					m_variables->setAsXMLNode (e);
				} else if (e.tagName () == "pascalCode") {
					QDomAttr contains = e.attributeNode ("contains");
					m_pascalCode = contains.value ().compare ("yes") == 0;
				} else if (e.tagName () == "checkSyntax") {
					QDomAttr checkSyntaxEnabled = e.attributeNode ("enabled");
					m_checkSyntax = checkSyntaxEnabled.value ().compare ("yes") == 0;
				}
			}
		}
                if (schemeType () == NSSchemeWizard::FUNCTION)
                    functionMap.registerFunction (this);
                if (schemeType () == NSSchemeWizard::PROCEDURE)
                    functionMap.registerProcedure (this);
	}

	this->move (x, y);
	this->resize (w, h);
	if (instructions ()) {
		instructions ()->setScheme (this);
                instructions ()->recursiveValidateContents ();
	}
	setModified (false);

	this->show ();

	return retval;
}

void NSScheme::closeEvent (QCloseEvent * e)
{
	bool accept = false;
	
	if (modified ()) {
		int retval = QMessageBox::question (
				this, 
				tr("Modified file"), 
				tr("Do you want to save changes\nbefore closing this file"), 
				QMessageBox::Yes, QMessageBox::No, QMessageBox::Cancel);
		if (retval == QMessageBox::Cancel) {
			accept = false;//e->ignore ();
		} else if (retval == QMessageBox::Yes) {
			if (save ())
				accept = true;//e->accept ();
			else 
				accept = false;//e->ignore ();
		} else { // QMessageBox::No
			accept = true;//e->accept ();
                }
	} else {
		accept = true;//e->accept ();
	}

	if (accept) {
                m_closing = true;
                emit closing();
		e->accept ();
                if (schemeType() == NSSchemeWizard::FUNCTION)
                    functionMap.unregisterFunction(this);
                if (schemeType() == NSSchemeWizard::PROCEDURE)
                    functionMap.unregisterProcedure(this);
	} else {
		e->ignore ();
	}
}

bool NSScheme::isClosing ()
{
    return m_closing;
}

void NSScheme::undo ()
{
        commands.undoAction ();
}

void NSScheme::cut ()
{
	if (p_activeInstruction) {
		p_mainParent->setClipboardInstruction (p_activeInstruction->copyOf ());
		removeInstruction ();
	}
}

void NSScheme::copy ()
{
	if (p_activeInstruction) {
		p_mainParent->setClipboardInstruction (p_activeInstruction->copyOf ());
		qDebug("instruction copied");
	}
}

void NSScheme::paste ()
{
	Instruction *i = p_mainParent->clipboardInstruction ();

	if (i) {
		i = i->copyOf ();
		qDebug("instruction pasted");

		if (qobject_cast<Sequence*> (i) == 0) {
			qDebug("aint no sequence");
			Sequence *s = findParentSequence ();

			if (s) {
				i->setScheme (this);
                                i->recursiveValidateContents ();
                                commands.doAction(Action::newAddInstruction(s, i, s->count()));
                                //i->setVisible (true);
                                //s->appendInstruction (i);
				setModified (true);
				qDebug("instruction appended");
			}
		} else {
			qDebug("we have a sequence");
		}
	}
}

QColor NSScheme::invalidColor ()
{
	SettingsDialog *sd = p_mainParent->getSettingsDialog ();
	if (sd) {
		return sd->invalidColor ();
	} else {
		return QColor(228,30,30);
	}
}

QColor NSScheme::selectionColor ()
{
	SettingsDialog *sd = p_mainParent->getSettingsDialog ();
	if (sd) {
		return sd->selectionColor ();
	} else {
		return QColor(0,255,0);
	}
}

QColor NSScheme::executionColor ()
{
	SettingsDialog *sd = p_mainParent->getSettingsDialog ();
	if (sd) {
		return sd->executionColor ();
	} else {
		return QColor(128,224,0);
	}
}

ProgramVariables* NSScheme::variables ()
{
	return m_variables;
}

void NSScheme::getInput(ExecutionThread *executor, const QString& type, const QString& caption, const QString& message)
{
        bool ok = true;

	if (type == "int") {
                executor->inputValue = QInputDialog::getInteger (this, caption, message, 0, -2147483647, 2147483647, 1, &ok);
	} else if (type == "double") {
                executor->inputValue = QInputDialog::getDouble (this, caption, message, 0, -2147483647, 2147483647, 1, &ok);
	}

        if (! ok)
            executor->stopSchemeExecution ();
}

void NSScheme::showMessage (ExecutionThread *executor, const QString& caption, const QString& message)
{
Q_UNUSED(executor);
	QMessageBox::information (this, caption, message);
//	executor->ioSemaphore.release ();
}

void NSScheme::on_executionThread_finished ()
{
	QMessageBox::information (this, tr("Execution"), tr("Scheme execution has finished"));
}

void NSScheme::assignArraySizes ()
{
	foreach (ident_val_t *id, *(m_variables->map ())) {
#ifdef DEBUG4
qDebug(QString("tc=%1 indval=%2 ident=%3")
			.arg (id->t.tc)
			.arg ((long)(void*)(id->v.indval))
			.arg (id->ident).toLocal8Bit ());
#endif
             if (id->t.tc == Array) {
			if (! id->v.indval) {
				id->v.indval = new QVector<BaseValue>();
#ifdef DEBUG4
qDebug(QString("indval=%1").arg ((long) (id->v.indval)).toLocal8Bit ());
#endif
                        }
#ifdef DEBUGD
qDebug(QString("indval.size=%1 id->t.arraySize=%2").arg (id->v.indval->size ()).arg (id->t.arraySize).toLocal8Bit ());
#endif
                        if (id->t.arraySize == 0) {
                                Type *t = &(id->t);
                                int d = 1;
                                long size = 1;
                                while (t && t->tc == Array) {
                                        long l = 0;
                                        while (0 >= (l = QInputDialog::getInteger (this,
                                                tr("Array dimension"),
                                                tr("Input number of elements in %2. dimension for array %1").arg (id->ident).arg(d), 10)))
                                            ;
                                        t->arrayDimension = l;
#ifdef DEBUGD
qDebug() << "l:" << l << "d:" << d;
#endif
                                        ++d;
                                        size *= l;
                                        t = t->subtype;
#ifdef DEBUGD
qDebug() << "t:" << t; if (t) qDebug() << "t->tc:" << t->tc;
#endif
                                }
#ifdef DEBUGD
qDebug() << "size:" << size;
#endif
                                id->setArraySize (size);
/*				long l = 0;
				while (0 >= (l = QInputDialog::getInteger (this, tr("Array size"), tr("Input size for array %1").arg (id->ident))))
					;
				id->setArraySize (l);
*/
                                /*id->v.indval->resize (l + 1);
				id->t.arraySize = l;*/
			}
		}
	}
}

bool NSScheme::containsPascalCode ()
{
	return m_pascalCode;
}

void NSScheme::setContainsPascalCode (bool v)
{
	m_pascalCode = v;
	setModified (true);
}

bool NSScheme::checkSyntax ()
{
	return m_checkSyntax;
}

void NSScheme::setCheckSyntax (bool v)
{
	m_checkSyntax = v;
	setModified (true);
}

QStringList NSScheme::schemeArguments ()
{
	QStringList result;
	
	if ((m_schemeType == NSSchemeWizard::FUNCTION) 
	    || (m_schemeType == NSSchemeWizard::PROCEDURE)) {
                QString n = m_schemeName;
                n.remove (QRegExp("\\bvar\\b")).remove (QRegExp("\\s+"));
                result = n.section ('(', 1, 1).section (')', 0, 0).split (',', QString::SkipEmptyParts);
	}

	return result;
}

bool NSScheme::isArgumentByRef (QString name)
{
        if ((m_schemeType == NSSchemeWizard::FUNCTION)
            || (m_schemeType == NSSchemeWizard::PROCEDURE)) {
                QStringList tmpresult = m_schemeName.section ('(', 1, 1).section (')', 0, 0).split (',', QString::SkipEmptyParts);
                foreach (QString a, tmpresult) {
                        if (a.contains(QRegExp("\\bvar\\b")) && a.contains(QRegExp(QString("\\b%1\\b").arg(name)))) {
                                return true;
                        }
                }
                return false;
        } else
                return false;
}

void NSScheme::putSchemeSVG ()
{
	QString svgFileName = m_fileName;
	svgFileName.replace (".nss", ".svg");
	QString errorString;
	
	if (do_svgExport (svgFileName, errorString)) {
		startPutFile (svgFileName);
	} else {
		QMessageBox::warning (
				this,
				tr("NS Builder - put status"), 
				tr("Export to SVG image file failed:\n%1").arg (errorString)
			);
	}
}

void NSScheme::putSchemeNSS ()
{
	startPutFile (m_fileName);
}

QString NSScheme::nameValueMapToAppURL ()
{
    QMap<QString, QString>& nvMap = p_mainParent->getNameValueMap();
    QString result = "?";

    QMapIterator<QString, QString> i(nvMap);
    while (i.hasNext ()) {
        i.next ();
        result.append (QString("%1=%2").arg(i.key ()).arg(i.value ()));
        if (i.hasNext ())
            result.append ("&");
    }
    qDebug() << result;
    return result;
}

void NSScheme::startPutFile (QString file)
{
	QFileInfo fi(file);
	QString fileName = fi.fileName();
	QString hostName = p_mainParent->getSettingsDialog ()->putHostName ();
	int portNumber = p_mainParent->getSettingsDialog ()->putPortNumber ();
        QString appName = QString("%1%2").arg(p_mainParent->getSettingsDialog ()->putAppName ()).arg(nameValueMapToAppURL());

	putFile = new QFile(file);
	if (putFile->open (QIODevice::ReadOnly)) {
            QByteArray data = putFile->readAll();

            QString boundary="-----------------------------NSBUILDER-1234567890";
            QByteArray dataToSend;

            dataToSend.append ("\r\n--" + boundary + "\r\n");
            dataToSend.append (QString("Content-Disposition: form-data; name=\"file\"; filename=\"%1\"\r\n").arg(fileName));
            dataToSend.append ("Content-Type: application/octet-stream\r\n\r\n");
            dataToSend.append (data);
            dataToSend.append ("\r\n--" + boundary + "--\r\n");

		wwwput->setHost (hostName, portNumber);
                QHttpRequestHeader header("POST", appName, 1, 1);
                header.setValue ("Host", QString("%1:%2").arg (hostName).arg (portNumber));
		header.setValue ("Connection", "keep-alive");
		header.setValue ("User-Agent", "NSBuilder/QHttp");
                header.setValue ("Referer", "NSBuilder");
                header.setValue ("Content-Type", QString("multipart/form-data; boundary=%1").arg(boundary));
                header.setValue ("Content-Length", QString::number (dataToSend.size())); //QString::number (fi.size ()));
                //header.setValue ("X-File-name", fileName);
		
                reqID = wwwput->request (header, dataToSend);
		qDebug () << "Request ID:" << reqID;
	}
}

void NSScheme::initQHttp ()
{
	wwwput = new QHttp();
	connect(wwwput, SIGNAL(requestFinished(int, bool)),
		this, SLOT(httpRequestFinished(int, bool)));
	connect(wwwput, SIGNAL(dataReadProgress (int, int)),
		this, SLOT(onDataReadProgress (int, int)));
	connect(wwwput, SIGNAL(dataSendProgress (int, int)),
		this, SLOT(onDataSendProgress (int, int)));
}

void NSScheme::onDataReadProgress (int done, int total)
{
	qDebug() << done << total;
}

void NSScheme::onDataSendProgress (int done, int total)
{
	qDebug() << done << total;
}

void NSScheme::httpRequestFinished (int id, bool error)
{
	if ((putFile) && (id == reqID)) {
            qDebug() << wwwput->lastResponse().toString();
            QString response = wwwput->readAll();
            int responseCode = wwwput->lastResponse().statusCode() / 100;
            bool responseError = !((responseCode == 2) || (responseCode == 3));

                if (error || responseError) {
                    QMessageBox::information (this, tr("NS Builder - put status"), tr("<b>Unable to put file on server</b><br/>%1").arg(response));
		} else {
                    QMessageBox::information (this, tr("NS Builder - put status"), tr("<b>File has been put on server</b><br/>%1").arg(response));
		}
		
		putFile->close ();
		delete putFile;
		putFile = 0;
		reqID = 0;
	}
}

BaseValue NSScheme::functionValue ()
{
    if (schemeType() == NSSchemeWizard::FUNCTION) {
        qDebug()  << "return value from this scheme:" << functionReturnValue.toString();
        return functionReturnValue;
    } else
        return BaseValue();
}

void NSScheme::setFunctionReturnValue (BaseValue bv)
{
    functionReturnValue = bv;
}

QString NSScheme::argumentsType()
{
    if ((schemeType() == NSSchemeWizard::FUNCTION) || (schemeType() == NSSchemeWizard::PROCEDURE)) {
            QString result;
            foreach (QString s, schemeArguments()) {
                    result += variables()->byName(s)->t.toString() + ", ";
            }
            result.truncate(result.length () - 2);
            return result;
    } else
            return QString::null;
}

void NSScheme::assertArgumentsRefValues ()
{
    if ((schemeType() != NSSchemeWizard::FUNCTION)
        && (schemeType() != NSSchemeWizard::PROCEDURE))
            return;

    foreach (QString n, schemeArguments()) {
        if (isArgumentByRef(n)) {
            m_variables->byName(n)->setReferenceValue();
        }
    }
}
