#include "imp_idents.h"
#include "exprtree.h"
#include "bisonproxy.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

IdentsMap identsMap;

BaseValue::BaseValue ()
{
	d = Unknown;
}

BaseValue::BaseValue (BaseValueDeterminant _d, long v)
{
	d = _d;
	val = v;
}

BaseValue::BaseValue (BaseValueDeterminant _d, double fv)
{
	d = _d;
	fval = fv;
}

BaseValue::BaseValue (BaseValueDeterminant _d, QString sv)
{
	d = _d;
	sval = new QString(sv);
}

BaseValue::BaseValue (BaseValueDeterminant _d, BaseValue *r)
    :d(_d), ref(r)
{
}

void BaseValue::setValue (BaseValue &bv)
{
    //qDebug("old d=%d ", this->d);
    //qDebug("old value=%s ", qPrintable(toString()));
    //qDebug("new d=%d ", bv.d);
    //qDebug("new value=%s", qPrintable(bv.toString()));

	switch (this->d) {
		case Long: {
			switch (bv.d) {
				case Long:
					this->val = bv.val;                            
				break;
				case Double:
					this->val = (long) bv.fval;
				break;
                                case Ref:
                                        if (bv.ref)
                                            setValue (*(bv.ref));
                                        else
                                            this->val = 0;
                                break;
				case String:
				case Unknown:
				break;
			}
		}
		break;
		case Double:
			switch (bv.d) {
				case Long:
					this->fval = bv.val;
				break;
				case Double:
					this->fval = bv.fval;
				break;
                                case Ref:
                                        if (bv.ref)
                                            setValue (*(bv.ref));
                                        else
                                            this->fval = 0.0;
                                break;
				case String:
				case Unknown:
				break;
			}
		break;
		case String:
			switch (bv.d) {
				case String:
					*(this->sval) = *(bv.sval);
				break;
                                case Ref:
                                        if (bv.ref)
                                            setValue (*(bv.ref));
                                        else
                                            sval = 0;
                                break;
				case Unknown:
				case Long:
				case Double:
					sval = 0;
				break;
			}
                case Ref:
                        switch (bv.d) {
                            case Ref:
                                ref = bv.ref;
                            break;
                            case Long:
                                if (ref)
                                    ref->setValue(bv);
                                else
                                    ref = &bv;
                            break;
                            case Double:
                                if (ref)
                                    ref->setValue(bv);
                                else
                                    ref = &bv;
                            break;
                            case Unknown:
                            case String:
                            break;
                        }

                        //ref->setValue (bv.ref);
                break;
		case Unknown:
		break;
	}

        qDebug("new d=%d ", this->d);
        qDebug("new value=%s ", qPrintable(toString()));
}

QString BaseValue::toString () const
{
	switch (d) {
		case Unknown:
			break;
		case Long:
			return QString::number (val);
		case Double:
			return QString::number (fval);
		case String:
			if (sval) { return *sval; }
                        else { return QString::null; }
                case Ref:
                        if (ref)
                            return ref->toString ();
	}

	return QString::null;
}

QString Type::typeConstructorString (TypeConstructor tc)
{
    if (tc == Variable) {
            return "Variable";
    } else if (tc == Array) {
        return "Array";
    } else if (tc == Function) {
        return "Function";
    } else if (tc == Product) {
        return "Product";
    } else if (tc == Procedure) {
        return "Procedure";
    } else
        return "?";
}

QString Type::toString () const
{
#ifdef DEBUGTC
	qDebug() << "type constructor (UVAPF)" << tc;
#endif
        if (tc == Variable) {
		return "number"; 
	} else if (tc == Array) {
                return QString("array[1..%2] of %1").arg (subtype ? subtype->toString() : "?").arg (arrayDimension);
	} else if (tc == Function) {
		return QString("function from %1 to number").arg (subtype ? subtype->toString() : "?");
	} else if (tc == Product) {
		QString s = subtype ? subtype->toString() : "?";
		if (subtype2) {
			return QString("%1, %2").arg (s).arg (subtype2->toString());
		} else {
			return s;
		}
	} else if (tc == Procedure) {
		return QString("procedure with arguments %1").arg (subtype ? subtype->toString() : "?");
	}
	
	return "?";
}

QList<unsigned int> Type::arrayDimensions () const
{
        QList<unsigned int> retval;

        if (this->tc == Variable)
                ; //retval << 1;
        else if (this->tc == Array) {
                retval = this->subtype->arrayDimensions ();
                retval.push_front (this->arrayDimension);
        }

        return retval;
}

QString Type::arrayDimensionsString () const
{
        QString retval;

        foreach (unsigned int d, arrayDimensions()) {
                retval.append(QString::number(d)).append(',') ;
        }

        retval.chop(1);

        return retval;
}

QString Type::arrayIndexType () const
{
    QString retval;

    foreach (unsigned int d, arrayDimensions()) {
        Q_UNUSED(d);
        retval.append ("number, ");
    }

    retval.chop(2);

    return retval;
}

ident_val_t::ident_val_t(const char *id)
:t(this), ident(id)
{
	v.indval = 0;
}

ident_val_t::ident_val_t(const char *id, double _v)
:t(this), ident(id)
{
	v.bval.d = Double;
	v.bval.fval = _v;
}

ident_val_t::ident_val_t(const char *id, long _v)
:t(this), ident(id)
{
	v.bval.d = Long;
	v.bval.val = _v;
}

ident_val_t::~ident_val_t()
{
	Type *t = this->t.subtype;
	while (t) {
		Type *tmp = t->subtype;
		free (t);
		t = tmp;
	}
}

void ident_val_t::setValue (ident_val_t *zm)
{
    if (t.tc == Variable) {
        v.bval.setValue(zm->v.bval);
    } else if (t.tc == Array) {
        v.indval = zm->v.indval;
    }
#ifdef DEBUGX
    qDebug ("now %s has value %s", qPrintable(ident), qPrintable(valueToString()));
#endif
}

void ident_val_t::setValue (BaseValue &bv)
{
    if (t.tc == Variable) {
        v.bval.setValue(bv);
    }
#ifdef DEBUGX
    qDebug ("now %s has value %s", qPrintable(ident), qPrintable(valueToString()));
#endif
}

void ident_val_t::setReferenceValue ()
{
    v.bval.d = Ref;
    v.bval.ref = 0;
}

bool ident_val_t::setTypeConstructor (TypeConstructor _tc)
{
	if (t.tc == Undefined) {
		if (_tc == Array) {
			v.indval = 0;
			t.arraySize = 0;
                        t.subtype = new Type(this, Variable); // po tym poznac swiezy typ tablicowy
			qDebug("array variable %s", qPrintable(ident));
		}
		t.tc = _tc;

                switch (t.tc) {
                    case Variable:
                        v.bval.d = Double;
                    break;
                    case Unknown:
                    case Array:
                    case Product:
                    case Function:
                    case Procedure:
                    break;
                }

		return true;
	} else {
		return t.tc == _tc;
	}
}

bool ident_val_t::setArrayType (int dimCount)
{
        if (! setTypeConstructor (Array))
            return false;
#ifdef DEBUGD
qDebug() << "dimCount:" << dimCount;
#endif
        if (t.subtype->tc != Variable) { // typ juz zainicjowany
            if (t.arrayDimensions().count() == dimCount)
                return true;
            else
                return false;
        }

        if (dimCount >= 1) {
                Type *tx = t.subtype;
                while (dimCount > 1) {
                        tx->tc = Array;
                        tx->subtype = new Type(0);
                        tx = tx->subtype;
                        dimCount--;
                }
                tx->tc = Variable;
        }
#ifdef DEBUGD
qDebug() << "type:" << t.toString();
#endif
        return true;
}

bool ident_val_t::setArraySize (int size)
{
	if (t.tc == Array) {
		// uzywam vector indeksowanego od 0
		if (v.indval == 0) {
			v.indval = new QVector<BaseValue>();
#ifdef DEBUG4
qDebug ("new pointer %p", v.indval);
#endif




		}
		v.indval->resize (size + 1); //= new QVector<BaseValue>(size + 1);
		BaseValue bv(Double, 0.0);
		v.indval->fill (bv, size + 1);
		//for (int i = 1; i <= size; ++i) (*(v.indval))[i].setValue (bv);
		t.arraySize = size;
	}

	return t.tc == Array;
}

bool ident_val_t::setArrayDimensions (QList<QByteArray> dims)
{
        setArrayType(dims.count());
        QByteArray buf = dims.front ();
        dims.pop_front ();
        t.arrayDimension = buf.toInt ();

        Type *typ = &t;
        foreach (QByteArray buf, dims) {
            if (typ->subtype) {
                typ->subtype->arrayDimension = buf.toInt();
                typ = typ->subtype;
            } else
                return false;
        }
        qDebug() << t.toString();
        return true;
}

QString ident_val_t::valueToString () const
{
	if (t.tc == Variable) {
		switch (v.bval.d) {
			case Long:
				return QString("%1").arg (v.bval.val);
			case Double:
				return QString("%1").arg (v.bval.fval);
			case String:
				return QString("%1").arg (*(v.bval.sval));
                        case Ref:
                                if (v.bval.ref)
                                    return v.bval.ref->toString();
			case Unknown:
			break;
		}
	} else if (t.tc == Array) {
                if ((v.indval == 0) || (t.arraySize == 0))
                    return QString::null;
		
		QString result("%1%2");
		int i;
		for (i = 1; i < t.arraySize; ++i) {
                        //qDebug(QString("indval[%1]=%2").arg(i).arg(v.indval->at (i).toString ()).toLocal8Bit ());
			result = result.arg (v.indval->at (i).toString ()).arg (",%1%2");
		}
                //qDebug(QString("indval[%1]=%2").arg(i).arg(v.indval->at (i).toString ()).toLocal8Bit ());
		result = result.arg (v.indval->at (i).toString ()).arg ("");
		return result;
	}

	return QString::null;
}

bool ident_val_t::setValueFromString (QString s)
{
	if (t.tc == Variable) {
		bool ok;
		v.bval.d = Double;
		v.bval.fval = s.toDouble (&ok);
		return ok;
	} else if (t.tc == Array) {
		Q_ASSERT(v.indval != 0);
		s.replace (",", ", ");
		ProgramStatement *statement = ProgramStatement::scan_buffer (s.toUtf8 ().constData ());

		if (statement && statement->syntacticTree ()) {
			if ((statement->syntacticTree ()->typ == ATOM_LICZBA) 
				|| (statement->syntacticTree ()->typ == LISTA_WARTOSCI)) {

				bool retval = true;
				drzewo_skladn *t = statement->syntacticTree ();
				int i = 1;
				while (t != 0) {
					if (t->typ == ATOM_LICZBA) {
						(*(v.indval))[i++] = t->val;
						qDebug(QString("ATOM_LICZBA [%1]=%2").arg (i-1).arg (t->val.toString ()).toLocal8Bit ());
					} else if (t->typ == LISTA_WARTOSCI) {
						(*(v.indval))[i++] = t->skladnik[0]->val;
						qDebug(QString("LISTA_WARTOSCI [%1]=%2").arg (i-1).arg (t->skladnik[0]->val.toString ()).toLocal8Bit ());
					} else if (t->typ == OPER_ARYTM) {
						ProgramVariables pv;
                                                (*(v.indval))[i++] = execute_statement (0, t->skladnik[0], &pv);
						qDebug(QString("AWYR [%1]=%2").arg (i-1).arg ((*(v.indval))[i-1].toString ()).toLocal8Bit ());
					}

					t = t->skladnik[1];
				}
						
				return retval;
			}
		}
	}

	return false;
}

QString ident_val_t::toString() const
{
    return QString("%1=%2:%3").arg(this->ident).arg(this->valueToString()).arg(this->t.toString());
}

int BaseValue::toInt () const
{
    switch (this->d) {
        case Unknown:
        case String:
        break;
        case Long:
            return this->val;
        break;
        case Double:
            return (int) this->fval;
        break;
        case Ref:
            if (this->ref)
                return this->ref->toInt();
        break;
    }

    return 0;
}
