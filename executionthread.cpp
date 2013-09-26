#include "executionthread.h"
#include "sequence.h"
#include "instruction.h"
#include "nsscheme.h"
#include "settingsdialog.h"
#include "bisonproxy.h"

#include <QTimer>

ExecutionThread::ExecutionThread(NSScheme *_scheme):
  QThread(0/*_scheme*/), scheme(_scheme), m_execution(false), singlestep(false)
{
	connect(this,SIGNAL(getInput(ExecutionThread*,const QString&,const QString&,const QString&)),
                scheme,SLOT(getInput(ExecutionThread*,const QString&,const QString&,const QString&)),Qt::BlockingQueuedConnection);
	connect(this,SIGNAL(showMessage(ExecutionThread*,const QString&,const QString&)),
                scheme,SLOT(showMessage(ExecutionThread*,const QString&,const QString&)),Qt::BlockingQueuedConnection);
	connect(scheme,SIGNAL(closing()),this,SLOT(on_scheme_closing()));

	scheme->assignArraySizes ();
}

ExecutionThread::~ExecutionThread()
{
	disconnect(this,SIGNAL(getInput(ExecutionThread*,const QString&,const QString&,const QString&)),
		   scheme,SLOT(getInput(ExecutionThread*,const QString&,const QString&,const QString&)));
	disconnect(this,SIGNAL(showMessage(ExecutionThread*,const QString&,const QString&)),
		   scheme,SLOT(showMessage(ExecutionThread*,const QString&,const QString&)));
}

void ExecutionThread::on_scheme_closing ()
{
	stopSchemeExecution ();
	executeNextStep ();
}

void ExecutionThread::showMessage (const QString& caption, const QString& message)
{
	emit showMessage (this, caption, message);
//	ioSemaphore.acquire ();
}

int ExecutionThread::getIntegerInputValue (const QString& caption, const QString& message)
{
	qDebug("[ENTER] ExecutionThread::getIntegerInputValue");
	emit getInput(this,"int",caption,message);
//	ioSemaphore.acquire ();
	int result = inputValue.toInt ();
	qDebug(QString("[EXIT] ExecutionThread::getIntegerInputValue return value=%1").arg (result).toLocal8Bit ());
	return result;
}

double ExecutionThread::getDoubleInputValue (const QString& caption, const QString& message)
{
	qDebug("[ENTER] ExecutionThread::getDoubleInputValue");
        emit getInput(this,"double",caption,message);
//	ioSemaphore.acquire ();
        double result = inputValue.toDouble ();
        qDebug(QString("[EXIT] ExecutionThread::getDoubleInputValue return value=%1").arg (result).toLocal8Bit ());
	return result;
}

void ExecutionThread::stopSchemeExecution ()
{
	m_execution = false;
}

bool ExecutionThread::isSingleStepping () const
{
	return singlestep;
}

void ExecutionThread::setSingleStepping (bool v)
{
	singlestep = v;
}

void ExecutionThread::run ()
{
	Sequence *seq = scheme->instructions ();

//	IdentsMap &ids = *(scheme->variables ()->map ());
//	foreach (QString c, ids.keys ()) {
//		qDebug(QString("%1 -> %2").arg(c).arg((long)(ids[c])).toLocal8Bit ());
//	}

	if (seq->count () > 0) {
		Instruction *i = seq->instructionAt (0);

		m_execution = true;

		try {
			bool continue_execution;
			do { 
                                if (i) do {
					qDebug(i->toString ().toLocal8Bit ());
					if (qobject_cast<Sequence*> (i) == 0) {
						i->setRunning (true);

						if (singlestep)
							waitForNextStep ();
						else
							msleep (settingsDialog->executionSleep ());
					}

					bool shouldwait = false;
					
					i = i->execute (this, &shouldwait);
					
					if (shouldwait)
						waitForNextStep ();
				} while ((i != 0) && m_execution);

				if (! i) { // zakończyliśmy wykonywania schematu jako wywolania procedury
					Instruction *caller = 0;
                                        continue_execution = loadExecutionFrame (caller, i);
                                        //continue_execution = i != 0;
					if (caller) {
						qDebug(QString("return from call to '%1'").arg(caller->contents ()).toLocal8Bit ());
                                                caller->setRunning (false);
					}
					if (i) 
						qDebug(QString("next instruction is '%1'").arg (i->contents ()).toLocal8Bit ());
				} else
					continue_execution = false;
				// if (było wywołanie procedury)
				//     odtwórz ramkę: przywróć stare wskaźniki zmiennym, 
				//     znajdź następną instrukcję w poprzednim schemacie
				// else
			} while (continue_execution);
		} catch (long ex) {
			qDebug() << "Exception: " << ex;
			showMessage (tr("Execution failed!"), tr("There was a runtime error during scheme exection"));
			stopSchemeExecution ();
		}

		if (i != 0) {
			i->setRunning (false);
		}
	}
}

void ExecutionThread::waitForNextStep ()
{
	semaphore.acquire ();
}

void ExecutionThread::executeNextStep ()
{
	semaphore.release ();
}

void ExecutionThread::saveExecutionFrame (Instruction *i, Instruction *c, NSScheme *s)
{
	QStringList args = s->schemeArguments ();
	ProgramVariables *vars = s->variables ();

	Frame f(i,c,s);
	foreach (QString a, args) {
		f.varsSet.insert (vars->map ()->value (a));
	}

        foreach (QString a, args) {
                f.valueMap.insert(a, vars->map()->value(a)->v);
        }

	frameStack.push (f);
}

bool ExecutionThread::loadExecutionFrame (Instruction *&caller, Instruction *&next)
{
	if (frameStack.empty ()) {
                caller = 0;
                next = 0;
                return false;
        } else {
		Frame f = frameStack.pop ();
                caller = f.caller;
		foreach (ident_val_t *id, f.varsSet) {
			f.calledScheme->variables ()->map ()->insert (id->ident, id);
		}

                foreach (QString name, f.valueMap.keys()) {
                        if (! f.calledScheme->isArgumentByRef(name))
                                f.calledScheme->variables ()->map ()->value (name)->v = f.valueMap.value (name);
                }
                next = f.instr;
                return true;
	}
}

