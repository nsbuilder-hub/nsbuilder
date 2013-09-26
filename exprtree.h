#ifndef __EXPRTREE_H
#define __EXPRTREE_H

#include <QObject>
#include <QString>
#include <QVector>
#include <stdio.h>
#include <QMap>
#include "imp_idents.h"

/* tablica zawiera nazwy funkcji */
extern QVector<QString> builtinFuncIdents; 
extern QVector<QString> builtinFuncArgs;
extern QVector<QString> builtinFuncResults;
extern QVector<Type*> builtinFuncArgType;
void initiate_builtin_functions ();

class NSScheme;
class Instruction;

class FunctionMap: public QObject {
Q_OBJECT
public:

    void registerFunction (NSScheme *scheme);
    void unregisterFunction (NSScheme *scheme);

    void registerProcedure (NSScheme *scheme);
    void unregisterProcedure (NSScheme *scheme);

    bool isBuiltin (const QString& name);
    bool isUserDefined (const QString& name);

    NSScheme *getFunctionSchemeForName (const QString& name);
    NSScheme *getProcedureSchemeForName (const QString& name);

signals:
    void newCompoundStatement (const QString &name);
    void compoundStatementDeleted (const QString &name);
private:
    /* mapa nazw funkcji do schematów */
    QMap<QString, NSScheme*> userDefinedFunctions;
    /* mapa nazw procedure do schematów */
    QMap<QString, NSScheme*> userDefinedProcedures;

};

extern FunctionMap functionMap;

class ProgramVariables;

enum typ_skladnika {
	INSTR_SEKWENCJA, INSTR_PODSTAWIENIE, 
	INSTR_SELEKCJA, INSTR_PETLA, INSTR_PUSTA,
	ATOM_LICZBA, ATOM_IDENT, ATOM_LOG, 
	OPER_ARYTM, OPER_LOG, OPER_REL, OPER_NOT, 
	BWYR, INSTR_PROC, WYR_INDEKS, LISTA_WARTOSCI, 
	FUNC_CALL, ATOM_NAPIS, LISTA_LWYR
};

struct drzewo_skladn {                                                          
        enum typ_skladnika typ;                                                 
        struct ident_val_t* zmienna;
        BaseValue val;
        char *oper;
	struct drzewo_skladn* skladnik[3];                              

	drzewo_skladn(typ_skladnika t, ident_val_t *id):
		typ(t), zmienna(id)
	{
		skladnik[0] = skladnik[1] = skladnik[2] = 0;
	}

	drzewo_skladn(typ_skladnika t, long v):
		typ(t)
	{
		val.d = Long;
		val.val = v;
		skladnik[0] = skladnik[1] = skladnik[2] = 0;
	}

	drzewo_skladn(typ_skladnika t, double v):
		typ(t)
	{
		val.d = Double;
		val.fval = v;
		skladnik[0] = skladnik[1] = skladnik[2] = 0;
	}

	drzewo_skladn(typ_skladnika t, char *op):
		typ(t), oper(op)
	{
		skladnik[0] = skladnik[1] = skladnik[2] = 0;
	}
	
	~drzewo_skladn()
	{
		switch (typ) {
		case OPER_NOT:
		case OPER_REL:
		case OPER_LOG:
		case OPER_ARYTM:
                        free (oper);
			break;
		default:
			;
		};
	}
        typedef QMap<QString, TypeConstructor> TypeConstructorMap;
        bool typeCheck(ProgramVariables *vars, QString &failed, TypeConstructorMap identTypes = TypeConstructorMap());

        /* Liczba wymiarów, dla skalarnej 0, dla tablicy jednowymiarowej 1 itd. */
        int dimensionCount ();

	/* zwraca napisową reprezentację tego typu */
	QString typeToString (ProgramVariables *vars = 0);
};     

#define YYSTYPE drzewo_skladn*

extern struct drzewo_skladn *program;

void wypisz_ds (struct drzewo_skladn*);

/**
 * instruction - instrukcja zawierajaca wyrazenie
 * statement - wyrazenie do obliczenia
 * vars - zmienna programu
 * dims - wymiary tablicy
 */
BaseValue execute_statement (Instruction *instruction,
                             struct drzewo_skladn* statement,
                             ProgramVariables *vars/*IdentsMap &idents*/,
                             QList<unsigned int> dims=QList<unsigned int>());

extern QString syntaxErrorText;
extern bool typeCheckFailed;

#ifdef Q_WS_WIN
//extern YYSTYPE implval;
#endif

extern unsigned int textlen;

#endif

