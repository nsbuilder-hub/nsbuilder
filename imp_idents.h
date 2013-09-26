#ifndef __IMP_IDENTS_H
#define __IMP_IDENTS_H

#include <QMap>
#include <QString>
#include <QVector>

enum TypeConstructor { Undefined, 
	               Variable,
		       Array,
		       Product,
		       Function,
		       Procedure
                     };

struct ident_val_t;

struct Type { 
	ident_val_t *ident;
        TypeConstructor tc;
        // rozmiar wektora zawierającego elementy tablicy
        int arraySize;
        // wymiar tablicy
        unsigned int arrayDimension;
	Type *subtype, *subtype2;

	Type(ident_val_t *_ident, TypeConstructor _tc = Undefined)
		: ident(_ident), tc(_tc), subtype(0), subtype2(0) {}

        QString toString () const;
        QList<unsigned int> arrayDimensions () const;
        QString arrayDimensionsString () const;
        QString arrayIndexType () const;
        static QString typeConstructorString (TypeConstructor tc);
};

enum BaseValueDeterminant { Unknown, Long, Double, String, Ref };
struct BaseValue {
	BaseValueDeterminant d;
	union {
		long val;
		double fval; // wartość zmiennoprzecinkowa (float)
		QString *sval;
                BaseValue *ref;
	};

	BaseValue ();
	BaseValue (BaseValueDeterminant _d, long v);
	BaseValue (BaseValueDeterminant _d, double fv);
	BaseValue (BaseValueDeterminant _d, QString sv);
        BaseValue (BaseValueDeterminant _d, BaseValue *r);
        void setValue (BaseValue &bv);
	QString toString () const;
        int toInt () const;
};

struct Value {
	BaseValue bval;
	QVector<BaseValue> *indval;
};

struct ident_val_t {
	Type t;
	QString ident;                                                           
	Value v;

	ident_val_t(const char *id);
	ident_val_t(const char *id, long _v);
	ident_val_t(const char *id, double _v);

	~ident_val_t();

        /* Ustawienie zmiennej jako referencji, gdy jest parametrem procedure lub funkcji */
        void setReferenceValue ();
        void setValue (ident_val_t *zm);
        void setValue (BaseValue &bv);
	bool setTypeConstructor (TypeConstructor _tc);
        bool setArrayType (int dimCount);
	bool setArraySize (int size);
        bool setArrayDimensions (QList<QByteArray> dims);

        QString valueToString () const;
	bool setValueFromString (QString s);
        QString toString() const;
};                                                                              

typedef QMap<QString, ident_val_t*> IdentsMap;

extern IdentsMap identsMap;

#endif

