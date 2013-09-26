#ifndef __EXECUTIONTHREAD_H
#define __EXECUTIONTHREAD_H

#include <QThread>
#include <QSemaphore>
#include <QMessageBox>
#include <QInputDialog>
#include <QVariant>
#include <QStack>
#include <QSet>
#include <QMap>

#include "imp_idents.h"

class NSScheme;
class Instruction;
class ProgramVariables;

class ExecutionThread: public QThread {
	Q_OBJECT
	
	public:
	ExecutionThread(NSScheme *_scheme);
	~ExecutionThread();
	
	void run ();

	void stopSchemeExecution ();
	
	void executeNextStep ();
	
	/**
	 * @param i instrukcja nastepna do wykonania
	 * @param c instrukcja wywolania procedury
	 **/
	void saveExecutionFrame (Instruction *i, Instruction *c, NSScheme *s);
        bool loadExecutionFrame (Instruction *&caller, Instruction *&next);
	
	bool isSingleStepping () const;
	void setSingleStepping (bool v);

	int getIntegerInputValue (const QString& caption, const QString& message);
	double getDoubleInputValue (const QString& caption, const QString& message);
	void showMessage (const QString& caption, const QString& message);

	friend class NSScheme;
	
	signals:
	void getInput(ExecutionThread *executor, const QString& type, const QString& caption, const QString& message);
	void showMessage (ExecutionThread *executor, const QString& caption, const QString& message);
	
	protected slots:
	void on_scheme_closing ();

	protected:
	QVariant inputValue;
	QSemaphore ioSemaphore;

        private:
	NSScheme *scheme;
	bool m_execution;
	bool singlestep;
	
	QSemaphore semaphore;
	
        void waitForNextStep ();
	
	struct Frame {
		Instruction *instr;
		Instruction *caller;
                QMap<QString, Value> valueMap;
		QSet<ident_val_t*> varsSet;
		NSScheme *calledScheme;
		Frame(Instruction *i = 0, Instruction *c = 0, NSScheme *cs = 0): instr(i), caller(c), calledScheme(cs) {}
	};
	QStack<Frame> frameStack;
};

#endif

