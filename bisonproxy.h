#ifndef BISONPROXY_H
#define BISONPROXY_H

#include <QStringList>
#include <QtXml>

#include "imp_idents.h"

struct drzewo_skladn;
struct ident_val_t;

int impparse ();
int imp_scan_string (const char * str);

struct VarIndex {
        TypeConstructor tc;
        BaseValue bv;
        struct VarIndex *next;

        VarIndex();
        ~VarIndex();

        static VarIndex integerIndex (unsigned int i);
        static VarIndex multiIndex (QList<unsigned int> ind, QList<unsigned int> dims);
};

class ProgramVariables : public QObject {
        Q_OBJECT

        public:
        ProgramVariables();
        ~ProgramVariables();

        IdentsMap *map ();

        ident_val_t* byName (QString n);

        void formatXMLNode (QDomDocument& document, QDomNode& parent);
        bool setAsXMLNode (QDomNode& node);

        BaseValue& getVariableValue (const QString &name);
        void setVariableValue (const QString &name, BaseValue v);
        BaseValue& getIndexedVariableValue (const QString& name, VarIndex i);
        void setIndexedVariableValue (const QString& name, VarIndex i, BaseValue v);

        signals:
        void valueChanged (const QString &varName);
        void indexOutOfBounds (const QString &varName);

        private:
        IdentsMap *m_map;
        BaseValue nullBaseValue;
};

class ProgramStatement {
        public:
        ProgramStatement(drzewo_skladn* t);

        drzewo_skladn* syntacticTree ();

        static ProgramStatement* scan_buffer (const QString& buf);
        static ProgramStatement* scan_buffer (const QString& buf, ProgramVariables *vars);

        private:
        ProgramStatement();

        drzewo_skladn* tree;
};


#endif
