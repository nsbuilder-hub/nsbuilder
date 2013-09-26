#include "bisonproxy.h"
#include "imp_yacc.h"
#include "exprtree.h"

#include <QtGlobal>

ProgramStatement::ProgramStatement()
:tree(0)
{
}

ProgramStatement::ProgramStatement(drzewo_skladn* t)
:tree(t)
{
}

drzewo_skladn* ProgramStatement::syntacticTree ()
{
	//Q_ASSERT(tree != 0);
	return tree;
}

ProgramStatement* ProgramStatement::scan_buffer (const QString& buf)
{
	syntaxErrorText = QString::null;
        typeCheckFailed = false;
	textlen = 0;
	identsMap.clear ();
	ProgramStatement *result = 0;
        char * b = strdup(buf.toAscii ().data ());
        imp_scan_string (b);

	if (impparse () == 0) {
#ifdef DEBUG3
		qDebug() << "after impparse: program=" << program << " scanned length=" << textlen;
#endif
		if (program) {
			result = new ProgramStatement(program);
			program = 0;
		}
	}
	
	free(b);
	return result;
}

ProgramStatement* ProgramStatement::scan_buffer (const QString& buf, ProgramVariables *vars)
{
	syntaxErrorText = QString::null;
        typeCheckFailed = false;
	textlen = 0;
	identsMap.clear ();
        char * b = strdup(buf.toAscii ().data ());
	ProgramStatement *result = 0;
	
#ifdef DEBUG3
qDebug() << "buffer: " << buf.toAscii ();
#endif
        imp_scan_string (b);

	if (impparse () == 0) {
#ifdef DEBUG3
                qDebug() << "after impparse: program=" << program << " scanned length=" << textlen;
#endif
		if (program) {
			result = new ProgramStatement(program);
			program = 0;

			foreach (QString k, identsMap.keys ()){
				if (! vars->map ()->contains (k))
					vars->map ()->insert (k, identsMap[k]);
			}
		}
	} else {
#ifdef DEBUG3
qDebug () << "impparse failed";
#endif
	}
	
	free(b);
	return result;
}

VarIndex::VarIndex()
:tc(Undefined), next(0)
{
}

VarIndex::~VarIndex()
{
	if (next)
		delete next;
}

VarIndex VarIndex::integerIndex (unsigned int i)
{
	VarIndex vi;
	vi.tc = Array;
	vi.bv.d = Long;
	vi.bv.val = i;
	return vi;
}

VarIndex VarIndex::multiIndex (QList<unsigned int> ind, QList<unsigned int> dims)
{
        VarIndex vi;
        vi.tc = Array;
        vi.bv.d = Long;

        unsigned int i = ind.back (); ind.pop_back();
        int result = 0;
        unsigned int d = 1;
        bool error = false;

        do { qDebug() << i << dims.back () << d << result;
            if (i > dims.back ()) {
                result = -1;
                error = true;
            } else {
                result += i * d;
            }
            d = dims.back (); dims.pop_back();
            if (ind.count() > 0) { i = ind.back () - 1; ind.pop_back(); }
        } while (! error && (ind.count() >= 0) && (dims.count() > 0));
qDebug() << "index result:" << result;
        vi.bv.val = result;
        return vi;
}

ProgramVariables::ProgramVariables()
{
	m_map = new IdentsMap();
}

ProgramVariables::~ProgramVariables()
{
	foreach (ident_val_t *id, *m_map) {
		delete id;
	}
	m_map->clear ();
	delete m_map;
}

IdentsMap *ProgramVariables::map ()
{
	return m_map;
}

ident_val_t* ProgramVariables::byName (QString n)
{
	ident_val_t *found;
	
	if (m_map->contains (n)) {
		found = m_map->value (n);
	} else {
                qDebug() << "new variable in program variables" << n;
		found = new ident_val_t(n.toLocal8Bit ().constData (), 0L);
		m_map->insert (n, found);
	}
	
	return found;
}

BaseValue& ProgramVariables::getVariableValue (const QString &name)
{
        ident_val_t *var = byName (name);
        return var->v.bval;
}

void ProgramVariables::setVariableValue (const QString &name, BaseValue v)
{
	ident_val_t *var = byName (name);
	if (var->t.tc == Variable) {
                var->setValue (v);
/*#ifdef DEBUGX
qDebug ("now %s has value %s", qPrintable(var->ident), qPrintable(var->valueToString()));
#endif*/
                emit valueChanged (name);
	} else {
#ifdef DEBUGX
                qDebug ("%s is not a simple variable", qPrintable(name));
#endif
        }
}

BaseValue& ProgramVariables::getIndexedVariableValue (const QString& name, VarIndex i)
{
        ident_val_t *var = byName (name);
        if ((i.tc == Array) && (var->t.tc == Array)) {
                QVector<BaseValue> *vector = var->v.indval;
                long ind = i.bv.val;

                if ((ind < vector->size ()) && (ind >= 0)) {
                        return (*vector)[ind];
                } else {
                        emit indexOutOfBounds (name);                        
                }
        }

        return nullBaseValue;
}

void ProgramVariables::setIndexedVariableValue (const QString& name, VarIndex i, BaseValue v)
{
	ident_val_t *var = byName (name);
	if ((i.tc == Array) && (var->t.tc == Array)) {
		QVector<BaseValue> *vector = var->v.indval;
		long ind = i.bv.val;
#ifdef DEBUGX
qDebug("vector.size=%i index=%li", vector->size(), ind);
#endif
                if ((ind < vector->size ()) && (ind >= 0)) {
			(*vector)[ind] = v;
			emit valueChanged (name);
		} else {
                        emit indexOutOfBounds (name);
#ifdef DEBUGX
qDebug ("wrong index %ld for variable %s", ind, qPrintable(name));
#endif
                }
	} else {
#ifdef DEBUGX
qDebug ("%s is not an array variable", qPrintable(name));
#endif
        }
}

bool ProgramVariables::setAsXMLNode (QDomNode& node)
{
	bool retval = true;
	QDomNodeList nodeList = node.childNodes ();

	for (unsigned i = 0; retval && (i < nodeList.length ()); ++i) {
		QDomNode n = nodeList.item (i);

		if (n.nodeName () == "variable") {
			QDomNodeList varChList = n.childNodes ();

			QString varName;
			ident_val_t *varStruct = 0;

			for (unsigned k = 0; k < varChList.length (); ++k) {
				QDomNode vn = varChList.item (k);

				if (vn.nodeName () == "name") {
					varName = vn.firstChild ().nodeValue ();
				} else if (vn.nodeName () == "type") {
					QDomNode st = vn.firstChild ();

					if (st.nodeName () == "long") {
						varStruct = new ident_val_t(0, 0L);
                                                varStruct->setTypeConstructor(Variable);
					} else if (st.nodeName () == "array") {
						QDomAttr size = st.toElement ().attributeNode ("size");
						long l = size.value ().toLong ();

                                                QDomAttr dims = st.toElement().attributeNode ("dims");
                                                QList<QByteArray> dimsList = dims.value().toAscii().split(',');

						varStruct = new ident_val_t(0, 0L);
                                                varStruct->setArrayDimensions (dimsList);
                                                //varStruct->setArrayType (dimsList.count ());
						varStruct->setArraySize (l);
					}
				} else if (vn.nodeName () == "value") {
					QDomNode st = vn.firstChild ();

					if (st.nodeType () == QDomNode::TextNode) {
						if (varStruct && (varStruct->t.tc == Variable)) {
							varStruct->v.bval.val = st.toText ().data ().toLong ();
							varStruct->v.bval.d = Long;
						}
					} else if (st.nodeName () == "element") {
						QDomNodeList elemList = vn.childNodes ();

						if (varStruct && (varStruct->t.tc == Array)) {
							Q_ASSERT(varStruct->v.indval != 0);
							for (unsigned l = 0; l < qMin(elemList.length (),(uint) varStruct->t.arraySize); ++l) {
								qDebug(elemList.item (l).firstChild ().toText ().data ().toLocal8Bit ());
								(*(varStruct->v.indval))[l+1].val = elemList.item (l).firstChild ().toText ().data ().toLong ();
								(*(varStruct->v.indval))[l+1].d = Long;
							}
						}
					}
				}
			}
			if (varStruct)
				varStruct->ident = varName;
			else {
				varStruct = new ident_val_t(0, 0L);
				varStruct->ident = varName;
			}
			m_map->insert (varName, varStruct);
                        qDebug() << "read variable" << varName << m_map->value(varName)->toString();
		} else {
			qDebug (QString("nodeName=%1 != variable").arg (n.nodeName ()).toLocal8Bit ());
			retval = false;
		}
	}

	return retval;
}

void ProgramVariables::formatXMLNode (QDomDocument& document, QDomNode& parent)
{
	QDomElement variables = document.createElement ("variables");

	foreach (ident_val_t *id, *m_map) {
		if ((id->t.tc == Variable) || (id->t.tc == Array)) {
			QDomElement var = document.createElement ("variable");

			QDomText v_name = document.createTextNode (id->ident);
			QDomElement name = document.createElement ("name");
			name.appendChild (v_name);

			QDomElement type = document.createElement ("type");
			QDomElement value = document.createElement ("value");
			if (id->t.tc == Variable) {
				QDomElement t = document.createElement ("long");
				type.appendChild (t);
				
				QDomText v_value = document.createTextNode (QString::number ((id->v.bval.d == Long) ? id->v.bval.val : id->v.bval.fval));
				value.appendChild (v_value);
			} else if (id->t.tc == Array) {
				QDomElement a = document.createElement ("array");
				QDomElement t = document.createElement ("long");

                                QDomAttr s = document.createAttribute ("size");
				s.setValue (QString::number (id->t.arraySize));
				a.setAttributeNode (s);

                                QDomAttr d = document.createAttribute ("dims");
                                d.setValue (id->t.arrayDimensionsString());
                                a.setAttributeNode (d);

				a.appendChild (t);
				type.appendChild (a);

				for (int i = 1; i <= id->t.arraySize; ++i) {
					QDomElement e = document.createElement ("element");
					Q_ASSERT(id->v.indval != 0);
					Q_ASSERT(id->v.indval->size () == id->t.arraySize + 1);
					BaseValue bv = id->v.indval->at (i);
					e.appendChild (document.createTextNode (bv.toString ()));
					value.appendChild (e);
				}
			}
			
			var.appendChild (name);
			var.appendChild (type);
			var.appendChild (value);

			variables.appendChild (var);
			}
		}

	parent.appendChild (variables);
}

