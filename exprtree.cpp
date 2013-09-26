#include "exprtree.h"
#include "bisonproxy.h"
#include "nsscheme.h"
#include "mainwindow.h"
#include <math.h>
#include "callinstruction.h"
#include "executionthread.h"
#include <ctime>

QString syntaxErrorText;
bool typeCheckFailed;

unsigned int textlen = 0;

struct drzewo_skladn *program;

QVector<QString> builtinFuncIdents; 
QVector<QString> builtinFuncArgs;
QVector<QString> builtinFuncResults;
QVector<Type*> builtinFuncArgType;

FunctionMap functionMap;

void initiate_builtin_functions ()
{
	builtinFuncIdents << "sin"; builtinFuncArgs << "number";        builtinFuncResults << "number";builtinFuncArgType << new Type(0, Variable);
	
	builtinFuncIdents << "cos"; builtinFuncArgs << "number";        builtinFuncResults << "number";builtinFuncArgType << new Type(0, Variable);

	builtinFuncIdents << "tg";  builtinFuncArgs << "number";        builtinFuncResults << "number";builtinFuncArgType << new Type(0, Variable);
	
	builtinFuncIdents << "ctg"; builtinFuncArgs << "number";        builtinFuncResults << "number";builtinFuncArgType << new Type(0, Variable);

        builtinFuncIdents << "exp"; builtinFuncArgs << "number";        builtinFuncResults << "number";builtinFuncArgType << new Type(0, Variable);

        builtinFuncIdents << "ln"; builtinFuncArgs << "number";        builtinFuncResults << "number";builtinFuncArgType << new Type(0, Variable);
        /*Type *t = new Type(0, Product);
	t->subtype = new Type(0, Variable) ;
	t->subtype2 = new Type(0, Variable) ;
        builtinFuncArgType << t;
        */
	builtinFuncIdents << "log"; builtinFuncArgs << "number, number";builtinFuncResults << "number";
        Type *t = new Type(0, Product);
	t->subtype = new Type(0, Variable) ;
	t->subtype2 = new Type(0, Variable) ;
	builtinFuncArgType << t;
	
	builtinFuncIdents << "sqrt";builtinFuncArgs << "number";        builtinFuncResults << "number";builtinFuncArgType << new Type(0, Variable);
	
	builtinFuncIdents << "abs"; builtinFuncArgs << "number";        builtinFuncResults << "number";builtinFuncArgType << new Type(0, Variable);
}

QString drzewo_skladn::typeToString (ProgramVariables *vars)
{
	switch (typ) {
		case INSTR_SEKWENCJA:
		case INSTR_PODSTAWIENIE:
		case INSTR_SELEKCJA:
		case INSTR_PETLA:
		case INSTR_PUSTA:
			return QString::null;
		break;
		case INSTR_PROC:
			if (skladnik[0]) {
				return QString("%1 with arguments %2").arg("procedure").arg(skladnik[0]->typeToString (vars));
			} else {
				return "procedure";
			}
		break;
		case ATOM_LICZBA:
			return "number";
		break;
		case ATOM_NAPIS:
			return "string";
		break;
		case ATOM_IDENT:
			if (vars)
				return vars->byName (zmienna->ident)->t.toString ();
			else
				return zmienna->t.toString ();
		break;
		case OPER_ARYTM:
			if (skladnik[0] && skladnik[1]) {
qDebug("typ skladnik[0]=%s typ skladnik[1]=%s", qPrintable(skladnik[0]->typeToString (vars)), qPrintable(skladnik[1]->typeToString (vars)));
				if ((skladnik[0]->typeToString (vars) == "number")
					&& (skladnik[1]->typeToString (vars) == "number")) {
					return "number";
				} else {
					return QString::null;
				}
			} else {
				return QString::null;
			}
		break;
		case ATOM_LOG:
		case OPER_LOG:
		case OPER_REL:
		case OPER_NOT:
		case BWYR:
			return QString::null;
		break;
		case LISTA_LWYR:
			if (skladnik[1] && skladnik[0]) {
				return QString("%1, %2")
					.arg (skladnik[0]->typeToString (vars))
					.arg (skladnik[1]->typeToString (vars));
			} else {
				return "number";
			}
		break;
		case LISTA_WARTOSCI:
			if (skladnik[1]) {
				return QString("number, %1").arg (skladnik[1]->typeToString (vars));
			} else {
				return "number";
			}
		break;
		case WYR_INDEKS:
			if (vars->map ()->contains (zmienna->ident)) {
                                //return vars->byName (zmienna->ident)->t.subtype->toString ();
#ifdef DEBUGTC
qDebug() << "comparing types: " << vars->byName (zmienna->ident)->t.arrayIndexType () << " <?> " << skladnik[2]->typeToString(vars) << endl;
#endif
                                if (vars->byName (zmienna->ident)->t.arrayIndexType () == skladnik[2]->typeToString(vars)) {
                                        return "number";
                                } else {
                                        return QString::null;
                                }
			} else {
				return QString::null;
			}
		break;
		case FUNC_CALL:
			if (builtinFuncIdents.contains (skladnik[0]->zmienna->ident))
				return builtinFuncResults[builtinFuncIdents.indexOf (skladnik[0]->zmienna->ident)];
                        else {
                            NSScheme *func = functionMap.getFunctionSchemeForName(skladnik[0]->zmienna->ident);
                            if (func) // tutaj uważamy że funkcja zwraca tylko liczbe
                                return "number";
                            else
                                return QString::null;
                        }
		break;
	}

	return QString::null;
}

/* uzycie identyfikatora jako zmiennej (w regule IDENT) zmienia
	mu typ z Undefined na Variable
   uzycie identyfikatora jako tablicy (w regule IDENT [ AWYR ])
	zmienia mu typ z Undefined na Array
   jesli nie mozna identyfikatorowi zmienic typu to blad skladniowy
*/
bool drzewo_skladn::typeCheck(ProgramVariables *vars, QString &failed, TypeConstructorMap identTypes)
{
#ifdef DEBUGTC
        qDebug(QString("typ=%1").arg(typ).toLocal8Bit ());
#endif
        // procedura mocno rekursywna, korzysta z setTypeConstructor
	if (typ == ATOM_IDENT) {
            failed = zmienna->ident;
#ifdef DEBUGTC
            qDebug(QString("ATOM_IDENT tc=%1 ident=%2").arg(vars->byName(failed)->t.tc).arg(failed).toLocal8Bit ());
#endif
                if (identTypes.contains (failed)) {
                    qDebug() << "(bound) type constructor for" << failed << "has to be" << Type::typeConstructorString (identTypes[failed]);
                    return vars->byName (zmienna->ident)->setTypeConstructor (identTypes[failed]);
                } else {
                    qDebug() << "type constructor for" << failed << "has to be number";
                    return vars->byName (zmienna->ident)->setTypeConstructor (Variable);
                }
	} else if (typ == ATOM_LICZBA) {
#ifdef DEBUGTC
                qDebug(QString("ATOM_LICZBA").toLocal8Bit ());
#endif
                return true;
	} else if (typ == WYR_INDEKS) {
                failed = zmienna->ident;
                ident_val_t *zm = vars->byName (failed);
#ifdef DEBUGTC
qDebug() << QString("WYR_INDEKS tc=%2 id=%1 dimCount=%3").arg (failed).arg(zm->t.tc).arg(skladnik[2]->dimensionCount ());
#endif                
                bool result = zm->setArrayType (skladnik[2]->dimensionCount ());
                result = result && skladnik[2]->typeCheck (vars, failed);
                //result = result && skladnik[2]->typeToString (vars).compare(zm->t.arrayIndexType ());
		return result;
	} else if (typ == FUNC_CALL) {
		QString fid = skladnik[0]->zmienna->ident;
                failed = fid;
#ifdef DEBUGTC
                qDebug(QString("FUNC_CALL %1").arg (fid).toLocal8Bit ());
#endif
                if (! vars->byName (fid)->setTypeConstructor (Function)) {
#ifdef DEBUGTC
                        qDebug(QString("function call ident %1 is not function").arg (fid).toLocal8Bit ());
#endif
                        return false;
		}
		int ind;
		if ((ind = builtinFuncIdents.indexOf (fid)) >= 0) {
                        if (! skladnik[1]->typeCheck (vars, failed)) {
				return false;
			}
			vars->byName (fid)->t.subtype = builtinFuncArgType[ind];
                        //wypisz_ds(skladnik[1]);
#ifdef DEBUGTC
                        qDebug(QString("comparing %1 with %2 for %3")
                               .arg (builtinFuncArgs[ind])
                               .arg (skladnik[1]->typeToString (vars))
                               .arg (builtinFuncIdents[ind]).toLocal8Bit ());
#endif
                        return builtinFuncArgs[ind] == skladnik[1]->typeToString (vars);
                } else if (functionMap.isUserDefined (fid)) {
                        /*TypeConstructorMap map;
                        NSScheme *function = functionMap.getFunctionSchemeForName(fid);

                        foreach (QString a, function->schemeArguments()) {
                            qDebug() << a << Type::typeConstructorString(function->variables()->map()->value(a)->t.tc);
                            map.insert(a, function->variables()->map()->value(a)->t.tc);
                        }

                        if (! skladnik[1]->typeCheck (vars, failed, map)) {
                                return false;
                        }*/
#ifdef DEBUGTC
                        qDebug() << "comparing"
                                 << functionMap.getFunctionSchemeForName(fid)->argumentsType()
                                 << "with"
                                 << skladnik[1]->typeToString(vars)
                                 << "for"
                                 << fid;
#endif
                        return functionMap.getFunctionSchemeForName(fid)->argumentsType() == skladnik[1]->typeToString(vars);
                } else {
#ifdef DEBUGTC
                        qDebug(QString("%1 is not builtin function").arg (fid).toLocal8Bit ());
#endif
                        return false;
		}
        } else if (typ == LISTA_WARTOSCI || typ == LISTA_LWYR) {
#ifdef DEBUGTC
                qDebug(QString("LISTA_WARTOSCI/LISTA_LWYR").toLocal8Bit ());
#endif
                bool result = true;

                if (skladnik[0])
                        result = skladnik[0]->typeCheck(vars, failed);

                if (result && skladnik[1])
                            result = skladnik[1]->typeCheck (vars, failed);

                return result;
	} else if (typ == INSTR_PROC) {
                QString pid = zmienna->ident;
#ifdef DEBUGTC
qDebug() << "INSTR_PROC" << pid;
#endif
                if (! vars->byName (pid)->setTypeConstructor (Procedure)) {
#ifdef DEBUGTC
qDebug(QString("ident %1 is not a procedure").arg(pid).toLocal8Bit ());
#endif
                        failed = pid;
                        return false;
		}
		NSScheme *scheme;
                //if ((scheme = mainWin->findSchemeForName (zmienna->ident))) { // czy to jest identyfikator procedury
                if ((scheme = functionMap.getProcedureSchemeForName(zmienna->ident))) {
                        // czy to poniżej jest sensowne? jak przekazać tablicę?
//                        if (! skladnik[2]->typeCheck (vars)) { // czy argumenty zostały użyte niepoprawnie
//				qDebug("procedure's arguments have wrong type");
//				return false;
//			}

			ProgramVariables *schemeVars = scheme->variables ();
			drzewo_skladn *d = skladnik[2];
			foreach (QString n, scheme->schemeArguments ()) {
#ifdef DEBUGTC
qDebug() << QString("d->typ=%2 formal argument %1").arg(schemeVars->byName (n)->toString ()).arg(d->typ);
#endif
                                drzewo_skladn *x;
				if (! d) {
#ifdef DEBUGTC
                                        qDebug("not enough arguments");
#endif
                                        return false;
				}
                                if (d->typ == LISTA_LWYR
                                    || d->typ == LISTA_WARTOSCI) {
					if (! (x = d->skladnik[0])) {
#ifdef DEBUGTC
                                                qDebug("not enough arguments");
#endif
                                                return false;
					}
                                } else if (d->typ == ATOM_IDENT
                                           || d->typ == WYR_INDEKS
                                           || d->typ == ATOM_LICZBA
                                           || d->typ == OPER_ARYTM
                                           || d->typ == FUNC_CALL) {
					x = d;
				} else {
#ifdef DEBUGTC
                                        qDebug() << "impossible typ=" << d->typ;
#endif
                                        return false;
				}

#ifdef DEBUGTC
                                qDebug() << QString("actual argument: %1").arg(vars->byName(x->zmienna->ident)->toString());
#endif
                                /*if (schemeVars->byName (n)->t.toString ()
                                    != vars->byName(x->zmienna->ident)->t.toString()) {
#ifdef DEBUGTC
qDebug("arguments type mismatch");
#endif
                                */
                                if (schemeVars->byName (n)->t.toString () != x->typeToString(vars)) {
                                        failed = x->zmienna->ident;
                                        return false;
                                }

				d = d->skladnik[1];
			}

			if (d) {
                                failed = pid;
#ifdef DEBUGTC
qDebug("too many arguments");
#endif
                        }
			return ! d; // powinno być (! d) and true, 
			          // tzn. doszliśmy do końca listy parametrów formalnych 
				  // i przerobiliśmy wszystkie parametry aktualne
		} else {
                    // tutaj sygnal czy wyjatek ?
#ifdef DEBUGTC
qDebug(QString("procedure %1 not found").arg (zmienna->ident).toLocal8Bit ());
#endif
                        return false;
		}
	} else {
		bool ok = true;
		
		if (skladnik[0])
                        ok = ok && (skladnik[0]->typeCheck (vars, failed));
		if (! ok)
			return false;
		
		if (skladnik[1])
                        ok = ok && (skladnik[1]->typeCheck (vars, failed));
		if (! ok)
			return false;
		
		if (skladnik[2])
                        ok = ok && (skladnik[2]->typeCheck (vars, failed));

		return ok;
        }
}

int drzewo_skladn::dimensionCount ()
{
        if (typ == LISTA_WARTOSCI || typ == LISTA_LWYR) {
                return 1 + skladnik[1]->dimensionCount ();
        } else {
                return 1;
        }
}

QList<unsigned int> indecesList (Instruction *instruction, struct drzewo_skladn* statement, ProgramVariables *vars, QList<unsigned int> dims)
{
    QList<unsigned int> inds;
    if (statement != 0) {
        switch (statement->typ) {
        case LISTA_WARTOSCI:
        case LISTA_LWYR: {
            struct drzewo_skladn *st = statement;
            while (st->typ == LISTA_WARTOSCI || st->typ == LISTA_LWYR) {
                BaseValue b = execute_statement(instruction, st->skladnik[0], vars, dims);
                inds << ((b.d == Long) ? b.val : b.fval);
qDebug() << inds.back ();
                st = st->skladnik[1];
            }
            BaseValue b = execute_statement(instruction, st, vars, dims);
            inds << ((b.d == Long) ? b.val : b.fval);
qDebug() << inds.back ();
        }
            break;
        default: {
            BaseValue b = execute_statement(instruction, statement, vars, dims);
            inds << ((b.d == Long) ? b.val : b.fval);
        }
        }
    }
    return inds;
}

BaseValue execute_statement (Instruction *instruction, struct drzewo_skladn* statement, ProgramVariables *vars/*IdentsMap &idents*/, QList<unsigned int> dims)
{
	double v1 = 0, v2 = 0;
        BaseValue retval;
        retval.d = Double;
	retval.val = 0;

	if (statement != 0) {
#ifdef DEBUGX
qDebug() << "statement type=" << statement->typ;
//wypisz_ds (statement);
#endif
		switch (statement->typ) {
			case INSTR_PODSTAWIENIE:
				if (statement->skladnik[1]) {
                                        BaseValue val = execute_statement (instruction, statement->skladnik[1], vars);
#ifdef DEBUGX
qDebug ("%s:=%s\n", statement->skladnik[0]->zmienna->ident.toLocal8Bit ().constData (), val.toString ().toLocal8Bit ().constData ());
#endif
					QString vn = statement->skladnik[0]->zmienna->ident;
					if (statement->skladnik[0]->typ == ATOM_IDENT) {
						vars->setVariableValue (vn, val);
					} else if (statement->skladnik[0]->typ == WYR_INDEKS) {
                                                QList<unsigned int> dims = vars->byName (vn)->t.arrayDimensions ();
                                                QList<unsigned int> inds = indecesList (instruction, statement->skladnik[0]->skladnik[2], vars, dims);
                                                qDebug() << "dims:" << dims;
                                                qDebug() << "inds:" << inds;
                                                vars->setIndexedVariableValue (vn, VarIndex::multiIndex (inds, dims), val);
                                                //BaseValue b = execute_statement (instruction, statement->skladnik[0]->skladnik[2], vars);
                                                //unsigned int ind = (b.d == Long) ? b.val : (unsigned int) b.fval;
                                                //vars->setIndexedVariableValue (vn, VarIndex::integerIndex (ind), val);
					}
				}
				break;
			case ATOM_LICZBA:
				retval = statement->val;
			break;
			case ATOM_IDENT:
                                retval.setValue(vars->byName (statement->zmienna->ident)->v.bval);
                                //retval = vars->byName (statement->zmienna->ident)->v.bval;
			break;
			case ATOM_LOG:
				retval = statement->val;
			break;
			case OPER_ARYTM: {
				BaseValue b;
				if (statement->skladnik[0]) {
                                        b = execute_statement (instruction, statement->skladnik[0], vars);
					v1 = (b.d == Long) ? b.val : b.fval;
				}
				if (statement->skladnik[1]) {
                                        b = execute_statement (instruction, statement->skladnik[1], vars);
					v2 = (b.d == Long) ? b.val : b.fval;
				}
#ifdef DEBUGX
qDebug ("arytm: %lf%c%lf\n", v1, statement->oper[0], v2);
#endif
				retval.d = Double;
				switch (statement->oper[0]) {
					case '+': {
						retval.fval = v1 + v2;
					}
					break;
					case '*': {
						retval.fval = v1 * v2;
					}
					break;
					case '-': {
						retval.fval = v1 - v2;
					}
					break;
					case '/': {
						if (v2 == 0)
							throw 0L;
						else
							retval.fval = v1 / v2;
					}
					break;
					case 'm': /* mod */ {
						retval.d = Long;
						retval.val = (int)v1 % (int)v2;
					} 
					break;
					case 'd': /* idiv */ {
                                                if ((int)v2 == 0)
                                                    throw 0L;
                                                else {
                                                    retval.d = Long;
                                                    retval.val = (int)v1 / (int)v2;
                                                }
					} 
					break;
				}
			}
			break;
			case OPER_LOG: {
				BaseValue b;
				if (statement->skladnik[0]) {
                                        b = execute_statement (instruction, statement->skladnik[0], vars);
					v1 = (b.d == Long) ? b.val : b.fval;
				}
				
				if (statement->skladnik[1]) {
                                        b = execute_statement (instruction, statement->skladnik[1], vars);
					v2 = (b.d == Long) ? b.val : b.fval;
				}

				retval.d = Long;
				if (strcmp (statement->oper, "and") == 0) {
					retval.val = ((bool)v1) && ((bool) v2);
				} else if (strcmp (statement->oper, "or") == 0) {
					retval.val = ((bool)v1) || ((bool) v2);
				} else if (strcmp (statement->oper, "xor") == 0) {
					retval.val = ((bool)v1) != ((bool) v2);
				}
			}
			break;
			case OPER_NOT: {
				BaseValue b;

				if (statement->skladnik[0]) {
                                        b = execute_statement (instruction, statement->skladnik[0], vars);
					retval.val = ! ((b.val == Long) ? b.val : (int) b.fval);
			       } else 
					retval.val = false;

				retval.d = Long;
			}
			break;
			case OPER_REL: {
				BaseValue b;

				if (statement->skladnik[0]) {
                                        b = execute_statement (instruction, statement->skladnik[0], vars);
					v1 = (b.d == Long) ? b.val : b.fval;
				}
				if (statement->skladnik[1]) {
                                        b = execute_statement (instruction, statement->skladnik[1], vars);
					v2 = (b.d == Long) ? b.val : b.fval;
				}
					
				retval.d = Long;
				if (strcmp (statement->oper, "=")== 0)
					retval.val = v1 == v2;
				else if (strcmp (statement->oper, "<=") == 0)
					retval.val = v1 <= v2;
				else if (strcmp (statement->oper, "<") == 0)
					retval.val = v1 < v2;
				else if (strcmp (statement->oper, ">") == 0)
					retval.val = v1 > v2;
				else if (strcmp (statement->oper, ">=") == 0)
					retval.val = v1 >= v2;
				else if (strcmp (statement->oper, "<>") == 0)
					retval.val = v1 != v2;
			}
			break;
                        case WYR_INDEKS: {
                                QList<unsigned int> dims = vars->byName (statement->zmienna->ident)->t.arrayDimensions ();
                                QList<unsigned int> inds = indecesList (instruction, statement->skladnik[2], vars, dims);
                                qDebug() << "dims:" << dims;
                                qDebug() << "inds:" << inds;
                                retval = vars->getIndexedVariableValue (statement->zmienna->ident, VarIndex::multiIndex (inds, dims));
                        }
			break;
			case FUNC_CALL: {
				if ("sin" == statement->skladnik[0]->zmienna->ident) {
                                        BaseValue b = execute_statement (instruction, statement->skladnik[1], vars);
					retval.d = Double;
					retval.fval = sin ((b.d == Long) ? b.val : b.fval);
				} else if ("cos" == statement->skladnik[0]->zmienna->ident) {
                                        BaseValue b = execute_statement (instruction, statement->skladnik[1], vars);
					retval.d = Double;
					retval.fval = cos ((b.d == Long) ? b.val : b.fval);
				} else if ("tg" == statement->skladnik[0]->zmienna->ident) {
                                        BaseValue b = execute_statement (instruction, statement->skladnik[1], vars);
					retval.d = Double;
					retval.fval = tan ((b.d == Long) ? b.val : b.fval);
				} else if ("ctg" == statement->skladnik[0]->zmienna->ident) {
                                        BaseValue b = execute_statement (instruction, statement->skladnik[1], vars);
					retval.d = Double;
					double d = tan ((b.d == Long) ? b.val : b.fval);
					if (d == 0)
						throw 0L;
					else
						retval.fval = 1 / d;
				} else if ("exp" == statement->skladnik[0]->zmienna->ident) {
                                        BaseValue b = execute_statement (instruction, statement->skladnik[1], vars);
                                        //BaseValue e = execute_statement (instruction, statement->skladnik[1]->skladnik[1], vars);
                                        //qDebug(QString("exp b=%1 e=%2").arg(b.toString ()).arg(e.toString ()).toLocal8Bit ());
                                        qDebug(QString("exp b=%1").arg(b.toString ()).toLocal8Bit ());
					retval.d = Double;
                                        retval.fval = exp ((b.d == Long) ? b.val : b.fval);
                                        //retval.fval = exp (((e.d == Long) ? e.val : e.fval) * log ((b.d == Long) ? b.val : b.fval));
                                } else if ("ln" == statement->skladnik[0]->zmienna->ident) {
                                        BaseValue b = execute_statement (instruction, statement->skladnik[1], vars);
                                        qDebug(QString("ln b=%1").arg(b.toString ()).toLocal8Bit ());
                                        retval.d = Double;
                                        retval.fval = log ((b.d == Long) ? b.val : b.fval);
				} else if ("log" == statement->skladnik[0]->zmienna->ident) {
                                        BaseValue b = execute_statement (instruction, statement->skladnik[1]->skladnik[0], vars);
                                        BaseValue e = execute_statement (instruction, statement->skladnik[1]->skladnik[1], vars);
					qDebug(QString("log b=%1 e=%2").arg(b.toString ()).arg(e.toString ()).toLocal8Bit ());

					double lb, le, d;
					lb = log ((b.d == Long) ? b.val : b.fval);
					le = log ((e.d == Long) ? e.val : e.fval);
					
                                        if (lb == 0)
						throw 0L; 
					else 
						d = le / lb;
					
                                        /*if (d <= 0)
						throw 0L;
                                        else*/ {
						retval.d = Double;
						retval.fval = d;
					}
				} else if ("sqrt" == statement->skladnik[0]->zmienna->ident) {
                                        BaseValue b = execute_statement (instruction, statement->skladnik[1], vars);
					retval.d = Double;
                                        double v = (b.d == Long) ? b.val : b.fval;
                                        if (v >= 0)
                                                retval.fval = sqrt (v);
                                        else
                                                throw 0L;
				} else if ("abs" == statement->skladnik[0]->zmienna->ident) {
                                        BaseValue b = execute_statement (instruction, statement->skladnik[1], vars);
					if (b.d == Long) {
						retval.d = Long;
						retval.val = labs (b.val);
					} else {
						retval.d = Double;
						retval.fval = fabs (b.fval);
					}
                                } else if (functionMap.isUserDefined(statement->skladnik[0]->zmienna->ident)) {
                                        NSScheme *scheme = functionMap.getFunctionSchemeForName(statement->skladnik[0]->zmienna->ident);
                                        if (instruction && scheme) {
                                                ExecutionThread *e = new ExecutionThread(scheme);
                                                e->saveExecutionFrame(0,0,scheme);
                                                associateArguments (vars, statement->skladnik[1], scheme, instruction->scheme());
                                                e->run();
                                                qDebug() << "waiting for function call to finish";
                                                e->wait();
                                                Instruction *nullInstr, *nullCaller;
                                                e->loadExecutionFrame(nullCaller, nullInstr);
#ifdef DEBUGX
    IdentsMap::const_iterator it = scheme->variables()->map ()->constBegin();
    while (it != scheme->variables()->map ()->constEnd()) {
            qDebug () << it.key() << "@" << it.value() << "v=" << it.value ()->valueToString ();
            ++it;
    }
#endif
                                                retval = scheme->functionValue();
                                        } else {
                                                qDebug() << "no function scheme or no enclosing instruction";
                                        }
                                }
			}
			break;
			case INSTR_PROC:
			break;
			case LISTA_WARTOSCI:
                        case LISTA_LWYR: {
                                BaseValue bv = execute_statement (instruction, statement->skladnik[0], vars);
                                unsigned int indv = (bv.d == Long) ? bv.val : (unsigned int) bv.fval;
                                qDebug() << "indv:" << indv << "dims:" << dims;
                                if (dims.count () > 0) {
                                    if (dims.count() > 1) {
                                        if (indv > dims.front ())
                                            throw 0L;
                                        dims.pop_front();
                                        indv = (indv - 1) * dims.front ();
                                    }
                                    if (statement->skladnik[1]) {
                                        BaseValue bvl = execute_statement(instruction, statement->skladnik[1], vars, dims);
                                        indv += (bvl.d == Long) ? bvl.val : (unsigned int) bvl.fval;
                                    }
                                }
                                retval.val = indv;
                        }
                        break;
			case INSTR_SEKWENCJA:
			case INSTR_SELEKCJA:
			case INSTR_PETLA:
			case INSTR_PUSTA:
			case BWYR:
			default:
			break;
		}
	}
#ifdef DEBUGX
qDebug ("retval.d=%d retval.val=%ld retval.fval=%lf retval.ref=%lx\n",
         retval.d,   retval.val,    retval.fval,    (unsigned long int) (void*) retval.ref);
#endif
	return retval;
}

void FunctionMap::registerFunction (NSScheme *scheme)
{
    // nazwa zawiera nawias z argumentami
    QString funcName = scheme->schemeName().section ('(', 0, 0).simplified();

    if ((scheme->schemeType() == NSSchemeWizard::FUNCTION) && ! funcName.isEmpty()) {
        userDefinedFunctions.insert(funcName, scheme);
        emit newCompoundStatement(funcName);
        qDebug() << "new function" << funcName;
    }
}

void FunctionMap::unregisterFunction (NSScheme *scheme)
{
    QString funcName = scheme->schemeName().section ('(', 0, 0).simplified();

    if (! funcName.isEmpty () && userDefinedFunctions.contains(funcName)) {
        userDefinedFunctions.remove(funcName);
        emit compoundStatementDeleted(funcName);
        qDebug() << "deleted function" << funcName;
    }
}

void FunctionMap::registerProcedure (NSScheme *scheme)
{
    QString procName = scheme->schemeName().section ('(', 0, 0).simplified();

    if ((scheme->schemeType() == NSSchemeWizard::PROCEDURE) && ! procName.isEmpty()) {
        userDefinedProcedures.insert(procName, scheme);
        emit newCompoundStatement(procName);
        qDebug() << "new procedure" << procName;
    }
}

void FunctionMap::unregisterProcedure (NSScheme *scheme)
{
    QString procName = scheme->schemeName().section ('(', 0, 0).simplified();

    if (! procName.isEmpty () && userDefinedProcedures.contains(procName)) {
        userDefinedProcedures.remove(procName);
        emit compoundStatementDeleted(procName);
        qDebug() << "deleted procedure" << procName;
    }
}

bool FunctionMap::isBuiltin (const QString& name)
{
    return builtinFuncIdents.contains(name);
}

bool FunctionMap::isUserDefined (const QString& name)
{
    return userDefinedFunctions.contains(name);
}

NSScheme* FunctionMap::getFunctionSchemeForName (const QString& name)
{
    return userDefinedFunctions[name];
}

NSScheme* FunctionMap::getProcedureSchemeForName (const QString& name)
{
    return userDefinedProcedures[name];
}

int gl = 0;

void wypisz_ds (struct drzewo_skladn* d)
{
	if (d) {
		switch (d->typ) {
			case INSTR_SEKWENCJA:
				printf ("%4d: sekwn\n", gl);
				wypisz_ds (d->skladnik[0]);
				wypisz_ds (d->skladnik[1]);
			break;
			case INSTR_PODSTAWIENIE:
				printf ("%4d: podst\n", gl);
				wypisz_ds (d->skladnik[0]);
				printf (" :=\n");
				gl++;
				wypisz_ds (d->skladnik[1]);
				gl--;
			break;
			case INSTR_SELEKCJA:
				printf ("%4d: selekcja\n", gl++);
				wypisz_ds (d->skladnik[0]);
				wypisz_ds (d->skladnik[1]);
				wypisz_ds (d->skladnik[2]);
				gl--;
			break;
			case INSTR_PETLA:
				printf ("%4d: petla\n", gl++);
				wypisz_ds (d->skladnik[0]);
				gl--;
			break;
			case INSTR_PUSTA:
				printf ("%4d: instr. pusta\n", gl);
			break;
			case ATOM_LICZBA:
				printf ("%4d: liczba %ld\n", gl, d->val.val);
			break;
			case ATOM_IDENT:
				printf ("%4d: ident %s\n", gl, d->zmienna->ident.toLocal8Bit ().constData ());
			break;
			case ATOM_LOG:
				printf ("%4d: stala logiczna %ld\n", gl, d->val.val);
			break;
			case ATOM_NAPIS:
				printf ("%4d: stala napisowa %s\n", gl, d->val.sval->toUtf8 ().constData ());
			break;
			case OPER_ARYTM:
				printf ("%4d: aoper %s\n", gl++, d->oper);
				wypisz_ds (d->skladnik[0]);
				wypisz_ds (d->skladnik[1]);
				gl--;
			break;
			case OPER_LOG:
				printf ("%4d: log oper %s\n", gl++, d->oper);
				wypisz_ds (d->skladnik[0]);
				wypisz_ds (d->skladnik[1]);
				gl--;
			break;
			case OPER_NOT:
				printf ("%4d: not oper %s\n", gl++, d->oper);
				wypisz_ds (d->skladnik[0]);
				gl--;
			break;
			case OPER_REL:
				printf ("%4d: rel oper %s\n", gl++, d->oper);
				wypisz_ds (d->skladnik[0]);
				wypisz_ds (d->skladnik[1]);
				gl--;
			break;
			case WYR_INDEKS:
				printf ("%4d: wyr indeks %s[]\n", gl++, d->zmienna->ident.toLocal8Bit ().constData ());
				wypisz_ds (d->skladnik[2]);
				gl--;
			break;
			case BWYR:
			break;
			case LISTA_LWYR:
			case LISTA_WARTOSCI:
				gl++;
				wypisz_ds (d->skladnik[0]);
				wypisz_ds (d->skladnik[1]);
				gl--;
			break;
			case INSTR_PROC:
                                printf ("%4d: call %s\n", gl++, d->zmienna->toString().toLocal8Bit().constData());
                                wypisz_ds (d->skladnik[2]);
                        break;
			case FUNC_CALL:
                                wypisz_ds (d->skladnik[0]);
			break;
		}
        } else {
                printf ("(null)\n");
        }
}

