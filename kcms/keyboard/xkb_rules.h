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


#ifndef XKB_RULES_H_
#define XKB_RULES_H_

#include <QtXml/QXmlDefaultHandler>
#include <QtCore/QList>
#include <QtCore/QStringList>


struct ConfigItem {
	QString name;
	QString description;
};

template <class T>
inline T* findByName(QList<T*> list, QString name) {
	foreach(T* info, list) {
		if( info->name == name )
			return info;
	}
	return NULL;
}

struct VariantInfo: public ConfigItem {
	QList<QString> languages;
	const bool fromExtras;

	VariantInfo(bool fromExtras_):
		fromExtras(fromExtras_) {}
};

struct LayoutInfo: public ConfigItem {
	QList<VariantInfo*> variantInfos;
	QList<QString> languages;
	const bool fromExtras;

//	LayoutInfo() {}
	LayoutInfo(bool fromExtras_):
		fromExtras(fromExtras_) {}
	~LayoutInfo() { foreach(VariantInfo* variantInfo, variantInfos) delete variantInfo; }

	VariantInfo* getVariantInfo(const QString& variantName) const {
	   	return findByName(variantInfos, variantName);
	}

	bool isLanguageSupportedByLayout(const QString& lang) const;
	bool isLanguageSupportedByDefaultVariant(const QString& lang) const;
	bool isLanguageSupportedByVariants(const QString& lang) const;
	bool isLanguageSupportedByVariant(const VariantInfo* variantInfo, const QString& lang) const;
};

struct ModelInfo: public ConfigItem {
	QString vendor;
};

struct OptionInfo: public ConfigItem {
};

struct OptionGroupInfo: public ConfigItem {
	QList<OptionInfo*> optionInfos;
	bool exclusive;

	~OptionGroupInfo() { foreach(OptionInfo* opt, optionInfos) delete opt; }

	const OptionInfo* getOptionInfo(const QString& optionName) const {
    	return findByName(optionInfos, optionName);
    }
};

struct Rules {
    enum ExtrasFlag { NO_EXTRAS, READ_EXTRAS };

	static const char XKB_OPTION_GROUP_SEPARATOR;

	QList<LayoutInfo*> layoutInfos;
    QList<ModelInfo*> modelInfos;
    QList<OptionGroupInfo*> optionGroupInfos;
    QString version;

    Rules();

	~Rules() {
		foreach(LayoutInfo* layoutInfo, layoutInfos) delete layoutInfo;
		foreach(ModelInfo* modelInfo, modelInfos) delete modelInfo;
		foreach(OptionGroupInfo* optionGroupInfo, optionGroupInfos) delete optionGroupInfo;
	}

    const LayoutInfo* getLayoutInfo(const QString& layoutName) const {
    	return findByName(layoutInfos, layoutName);
    }

    const OptionGroupInfo* getOptionGroupInfo(const QString& optionGroupName) const {
    	return findByName(optionGroupInfos, optionGroupName);
    }

    static Rules* readRules(ExtrasFlag extrasFlag);
    static Rules* readRules(Rules* rules, const QString& filename, bool fromExtras);
    static QString getRulesName();
};

#endif /* XKB_RULES_H_ */
