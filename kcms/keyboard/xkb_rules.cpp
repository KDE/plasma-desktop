/*
 *  Copyright (C) 2010 Andriy Rysin (rysin@kde.org)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "xkb_rules.h"
#include "config-workspace.h"

#include <KLocalizedString>

#include <QDir>
#include <QRegExp>
#include <QTextDocument> // for Qt::escape
#include <QXmlAttributes>

#include <QtConcurrent>
//#include <libintl.h>
//#include <locale.h>

#include "x11_helper.h"

// for findXkbRuleFile
#include <QX11Info>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/XKBlib.h>
#include <X11/extensions/XKBrules.h>
#include <fixx11h.h>
#include <config-workspace.h>



class RulesHandler : public QXmlDefaultHandler
{
public:
	RulesHandler(Rules* rules_, bool fromExtras_):
		rules(rules_),
		fromExtras(fromExtras_){}

    bool startElement(const QString &namespaceURI, const QString &localName,
                      const QString &qName, const QXmlAttributes &attributes);
    bool endElement(const QString &namespaceURI, const QString &localName,
                    const QString &qName);
    bool characters(const QString &str);
//    bool fatalError(const QXmlParseException &exception);
//    QString errorString() const;

private:
//    QString getString(const QString& text);

    QStringList path;
    Rules* rules;
    const bool fromExtras;
};

static QString translate_xml_item(const QString& itemText)
{
	return i18n(itemText.toHtmlEscaped().toUtf8());
	//	return QString::fromUtf8(dgettext("xkeyboard-config", itemText.toAscii()));
}

static QString translate_description(ConfigItem* item)
{
	return item->description.isEmpty()
			? item->name : translate_xml_item(item->description);
}

static bool notEmpty(const ConfigItem* item)
{
  return ! item->name.isEmpty();
}

template<class T>
void removeEmptyItems(QList<T*>& list)
{
#ifdef __GNUC__
#if __GNUC__ == 4 && (__GNUC_MINOR__ == 8 && __GNUC_PATCHLEVEL__ < 3) || (__GNUC_MINOR__ == 7 && __GNUC_PATCHLEVEL__ < 4)
#warning Compiling with a workaround for GCC < 4.8.3 || GCC < 4.7.4 http://gcc.gnu.org/bugzilla/show_bug.cgi?id=58800
    Q_FOREACH(T* x, list) {
        ConfigItem *y = static_cast<ConfigItem*>(x);
        if (y->name.isEmpty()) {
            list.removeAll(x);
        }
    }
#else
    QtConcurrent::blockingFilter(list, notEmpty);
#endif
#endif
}

static
void postProcess(Rules* rules)
{
	//TODO remove elements with empty names to safeguard us
	removeEmptyItems(rules->layoutInfos);
	removeEmptyItems(rules->modelInfos);
	removeEmptyItems(rules->optionGroupInfos);

//	setlocale(LC_ALL, "");
//	bindtextdomain("xkeyboard-config", LOCALE_DIR);
	foreach(ModelInfo* modelInfo, rules->modelInfos) {
		modelInfo->vendor = translate_xml_item(modelInfo->vendor);
		modelInfo->description = translate_description(modelInfo);
	}

	foreach(LayoutInfo* layoutInfo, rules->layoutInfos) {
		layoutInfo->description = translate_description(layoutInfo);

		removeEmptyItems(layoutInfo->variantInfos);
		foreach(VariantInfo* variantInfo, layoutInfo->variantInfos) {
			variantInfo->description = translate_description(variantInfo);
		}
	}
	foreach(OptionGroupInfo* optionGroupInfo, rules->optionGroupInfos) {
		optionGroupInfo->description = translate_description(optionGroupInfo);

		removeEmptyItems(optionGroupInfo->optionInfos);
		foreach(OptionInfo* optionInfo, optionGroupInfo->optionInfos) {
			optionInfo->description = translate_description(optionInfo);
		}
	}
}


Rules::Rules():
	version("1.0")
{
}

QString Rules::getRulesName()
{
	XkbRF_VarDefsRec vd;
	char *tmp = NULL;

	if (XkbRF_GetNamesProp(QX11Info::display(), &tmp, &vd) && tmp != NULL ) {
		// 			qCDebug(KCM_KEYBOARD) << "namesprop" << tmp ;
                const QString name(tmp);
                XFree(tmp);
		return name;
	}

	return QString::null;
}

QString Rules::findXkbDir()
{
	QString xkbParentDir;

	QString base(XLIBDIR);
	if( base.count('/') >= 3 ) {
		// .../usr/lib/X11 -> /usr/share/X11/xkb vs .../usr/X11/lib -> /usr/X11/share/X11/xkb
		QString delta = base.endsWith("X11") ? "/../../share/X11" : "/../share/X11";
		QDir baseDir(base + delta);
		if( baseDir.exists() ) {
			xkbParentDir = baseDir.absolutePath();
		}
		else {
			QDir baseDir(base + "/X11");	// .../usr/X11/lib/X11/xkb (old XFree)
			if( baseDir.exists() ) {
				xkbParentDir = baseDir.absolutePath();
			}
		}
	}

	if( xkbParentDir.isEmpty() ) {
		xkbParentDir = "/usr/share/X11";
	}

	return xkbParentDir + "/xkb";
}

static QString findXkbRulesFile()
{
	QString rulesFile;
	QString rulesName = Rules::getRulesName();

	if ( ! rulesName.isNull() ) {
		QString xkbDir = Rules::findXkbDir();
		rulesFile = QString("%1/rules/%2.xml").arg(xkbDir, rulesName);
	}

	return rulesFile;
}

static
void mergeRules(Rules* rules, Rules* extraRules)
{
	rules->modelInfos.append( extraRules->modelInfos );
	rules->optionGroupInfos.append( extraRules->optionGroupInfos );	// need to iterate and merge?

	QList<LayoutInfo*> layoutsToAdd;
	foreach(LayoutInfo* extraLayoutInfo, extraRules->layoutInfos) {
		LayoutInfo* layoutInfo = findByName(rules->layoutInfos, extraLayoutInfo->name);
		if( layoutInfo != NULL ) {
			layoutInfo->variantInfos.append( extraLayoutInfo->variantInfos );
			layoutInfo->languages.append( extraLayoutInfo->languages );
		}
		else {
			layoutsToAdd.append(extraLayoutInfo);
		}
	}
	rules->layoutInfos.append(layoutsToAdd);
	qCDebug(KCM_KEYBOARD) << "Merged from extra rules:" << extraRules->layoutInfos.size() << "layouts," << extraRules->modelInfos.size() << "models," << extraRules->optionGroupInfos.size() << "option groups";

	// base rules now own the objects - remove them from extra rules so that it does not try to delete them
	extraRules->layoutInfos.clear();
	extraRules->modelInfos.clear();
	extraRules->optionGroupInfos.clear();
}


const char Rules::XKB_OPTION_GROUP_SEPARATOR = ':';

Rules* Rules::readRules(ExtrasFlag extrasFlag)
{
	Rules* rules = new Rules();
	QString rulesFile = findXkbRulesFile();
	if( ! readRules(rules, rulesFile, false) ) {
		delete rules;
		return NULL;
	}
	if( extrasFlag == Rules::READ_EXTRAS ) {
		QRegExp regex("\\.xml$");
		Rules* rulesExtra = new Rules();
		QString extraRulesFile = rulesFile.replace(regex, ".extras.xml");
		if( readRules(rulesExtra, extraRulesFile, true) ) {	// not fatal if it fails
			mergeRules(rules, rulesExtra);
		}
		delete rulesExtra;
	}
	return rules;
}


Rules* Rules::readRules(Rules* rules, const QString& filename, bool fromExtras)
{
	QFile file(filename);
	if( !file.open(QFile::ReadOnly | QFile::Text) ) {
		qCritical() << "Cannot open the rules file" << file.fileName();
		return NULL;
	}

	RulesHandler rulesHandler(rules, fromExtras);

	QXmlSimpleReader reader;
	reader.setContentHandler(&rulesHandler);
	reader.setErrorHandler(&rulesHandler);

	QXmlInputSource xmlInputSource(&file);

	qCDebug(KCM_KEYBOARD) << "Parsing xkb rules from" << file.fileName();

	if( ! reader.parse(xmlInputSource) ) {
		qCritical() << "Failed to parse the rules file" << file.fileName();
		delete rules;
		return NULL;
	}

	postProcess(rules);

	return rules;
}

bool RulesHandler::startElement(const QString &/*namespaceURI*/, const QString &/*localName*/,
                      const QString &qName, const QXmlAttributes &attributes)
{
	path << QString(qName);

	QString strPath = path.join("/");
	if( strPath.endsWith("layoutList/layout/configItem") ) {
			rules->layoutInfos << new LayoutInfo(fromExtras);
	}
	else if( strPath.endsWith("layoutList/layout/variantList/variant") ) {
		rules->layoutInfos.last()->variantInfos << new VariantInfo(fromExtras);
	}
	else if( strPath.endsWith("modelList/model") ) {
		rules->modelInfos << new ModelInfo();
	}
	else if( strPath.endsWith("optionList/group") ) {
		rules->optionGroupInfos << new OptionGroupInfo();
		rules->optionGroupInfos.last()->exclusive = (attributes.value("allowMultipleSelection") != "true");
	}
	else if( strPath.endsWith("optionList/group/option") ) {
		rules->optionGroupInfos.last()->optionInfos << new OptionInfo();
	}
	else if( strPath == ("xkbConfigRegistry") && ! attributes.value("version").isEmpty()  ) {
		rules->version = attributes.value("version");
		qCDebug(KCM_KEYBOARD) << "xkbConfigRegistry version" << rules->version;
	}
	return true;
}

bool RulesHandler::endElement(const QString &/*namespaceURI*/, const QString &/*localName*/, const QString &/*qName*/)
{
	path.removeLast();
	return true;
}

bool RulesHandler::characters(const QString &str)
{
	if( !str.trimmed().isEmpty() ) {
		QString strPath = path.join("/");
		if( strPath.endsWith("layoutList/layout/configItem/name") ) {
			if( rules->layoutInfos.last() != NULL ) {
				rules->layoutInfos.last()->name = str.trimmed();
//				qCDebug(KCM_KEYBOARD) << "name:" << str;
			}
			// skipping invalid entry
		}
		else if( strPath.endsWith("layoutList/layout/configItem/description") ) {
			rules->layoutInfos.last()->description = str.trimmed();
//			qCDebug(KCM_KEYBOARD) << "descr:" << str;
		}
		else if( strPath.endsWith("layoutList/layout/configItem/languageList/iso639Id") ) {
			rules->layoutInfos.last()->languages << str.trimmed();
//			qCDebug(KCM_KEYBOARD) << "\tlang:" << str;
		}
		else if( strPath.endsWith("layoutList/layout/variantList/variant/configItem/name") ) {
			rules->layoutInfos.last()->variantInfos.last()->name = str.trimmed();
//			qCDebug(KCM_KEYBOARD) << "\tvariant name:" << str;
		}
		else if( strPath.endsWith("layoutList/layout/variantList/variant/configItem/description") ) {
			rules->layoutInfos.last()->variantInfos.last()->description = str.trimmed();
//			qCDebug(KCM_KEYBOARD) << "\tvariant descr:" << str;
		}
		else if( strPath.endsWith("layoutList/layout/variantList/variant/configItem/languageList/iso639Id") ) {
			rules->layoutInfos.last()->variantInfos.last()->languages << str.trimmed();
//			qCDebug(KCM_KEYBOARD) << "\tvlang:" << str;
		}
		else if( strPath.endsWith("modelList/model/configItem/name") ) {
			rules->modelInfos.last()->name = str.trimmed();
//			qCDebug(KCM_KEYBOARD) << "name:" << str;
		}
		else if( strPath.endsWith("modelList/model/configItem/description") ) {
			rules->modelInfos.last()->description = str.trimmed();
//			qCDebug(KCM_KEYBOARD) << "\tdescr:" << str;
		}
		else if( strPath.endsWith("modelList/model/configItem/vendor") ) {
			rules->modelInfos.last()->vendor = str.trimmed();
//			qCDebug(KCM_KEYBOARD) << "\tvendor:" << str;
		}
		else if( strPath.endsWith("optionList/group/configItem/name") ) {
			rules->optionGroupInfos.last()->name = str.trimmed();
//			qCDebug(KCM_KEYBOARD) << "name:" << str;
		}
		else if( strPath.endsWith("optionList/group/configItem/description") ) {
			rules->optionGroupInfos.last()->description = str.trimmed();
//			qCDebug(KCM_KEYBOARD) << "\tdescr:" << str;
		}
		else if( strPath.endsWith("optionList/group/option/configItem/name") ) {
			rules->optionGroupInfos.last()->optionInfos.last()->name = str.trimmed();
//			qCDebug(KCM_KEYBOARD) << "name:" << str;
		}
		else if( strPath.endsWith("optionList/group/option/configItem/description") ) {
			rules->optionGroupInfos.last()->optionInfos.last()->description = str.trimmed();
//			qCDebug(KCM_KEYBOARD) << "\tdescr:" << str;
		}
	}
	return true;
}

bool LayoutInfo::isLanguageSupportedByLayout(const QString& lang) const
{
	if( languages.contains(lang) || isLanguageSupportedByVariants(lang) )
		return true;

//	// return yes if no languages found in layout or its variants
//	if( languages.empty() ) {
//		foreach(const VariantInfo* info, variantInfos) {
//			if( ! info->languages.empty() )
//				return false;
//		}
//		return true;
//	}

	return false;
}

bool LayoutInfo::isLanguageSupportedByVariants(const QString& lang) const
{
	foreach(const VariantInfo* info, variantInfos) {
		if( info->languages.contains(lang) )
			return true;
	}
	return false;
}

bool LayoutInfo::isLanguageSupportedByDefaultVariant(const QString& lang) const
{
	if( languages.contains(lang) )
		return true;

	if( languages.empty() && isLanguageSupportedByVariants(lang) )
		return true;

	return false;
}

bool LayoutInfo::isLanguageSupportedByVariant(const VariantInfo* variantInfo, const QString& lang) const
{
	if( variantInfo->languages.contains(lang) )
		return true;

	// if variant has no languages try to "inherit" them from layout
	if( variantInfo->languages.empty() && languages.contains(lang) )
		return true;

	return false;
}

#ifdef NEW_GEOMETRY

Rules::GeometryId Rules::getGeometryId(const QString& model) {
    QString xkbDir = Rules::findXkbDir();
    QString rulesName = Rules::getRulesName();
    QString ruleFileName = QString("%1/rules/%2").arg(xkbDir, rulesName);
    QFile ruleFile(ruleFileName);

    GeometryId defaultGeoId("pc", "pc104");

    if ( ! ruleFile.open(QIODevice::ReadOnly | QIODevice::Text) ){
        qCritical() << "Unable to open file" << ruleFileName;
        return defaultGeoId;
    }
    
    QString modelGeoId = model;
    bool inTable = false;
    QTextStream in(&ruleFile);
    
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();

	if( line.isEmpty() || QRegExp("^\\s*//").indexIn(line) != -1 )
	    continue;

        QRegExp modelGroupRegex("!\\s*(\\$[a-zA-Z0-9_]+)\\s*=(.*)");
        
        if( modelGroupRegex.indexIn(line) != -1 ) {
    	    QStringList parts = modelGroupRegex.capturedTexts();
    	    QString groupName = parts[1];
    	    QStringList models = parts[2].split(QRegExp("\\s+"), QString::SkipEmptyParts);
    	    
//    	    qCDebug(KCM_KEYBOARD) << "modelGroup definition" << groupName << ":" << models;
    	    if( models.contains(model) ) {
    	        modelGeoId = groupName;
    	    }
    	    continue;
        }


	if( inTable ) {
    	    QRegExp modelTableEntry ("\\s*(\\$?[a-zA-Z0-9_]+|\\*)\\s*=\\s*([a-zA-Z0-9_]+)\\(([a-zA-Z0-9_%]+)\\)");
    	    if( modelTableEntry.indexIn(line) == -1 ) {
    	       if( QRegExp("^!\\s*").indexIn(line) != -1 )
    	         break;
    	        
    		qWarning() << "could not parse geometry line" << line;
    		continue;
    	    }
        
    	    QStringList parts = modelTableEntry.capturedTexts();
    	    QString modelName = parts[1];
    	    QString fileName = parts[2];
    	    QString geoName = parts[3];
    	    if( geoName == "%m" ) {
    	      geoName = model;
    	    }
    	    if( modelName == "*" ) {
    		defaultGeoId = GeometryId(fileName, geoName);
    	    }
    	    
//    	    qCDebug(KCM_KEYBOARD) << "geo entry" << modelName << fileName << geoName;
        
    	    if( modelName == model ) {
    		return GeometryId(fileName, geoName);
    	    }
    	    
    	    continue;
        }

        QRegExp modelTableHeader ("!\\s+model\\s*=\\s*geometry");
        if( modelTableHeader.indexIn(line) != -1 ) {
    	    inTable = true;
    	    continue;
        }

    }

    return defaultGeoId;
}

#endif
