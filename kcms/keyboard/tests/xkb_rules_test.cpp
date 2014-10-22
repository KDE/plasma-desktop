/*
 *  Copyright (C) 2011 Andriy Rysin (rysin@kde.org)
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

#include <QtTest/QtTest>

#include "../xkb_rules.h"

#include <qdom.h>
#include <qxml.h>


Q_LOGGING_CATEGORY(KCM_KEYBOARD, "kcm_keyboard")


static const Rules::ExtrasFlag readExtras = Rules::NO_EXTRAS;

class RulesTest : public QObject
{
    Q_OBJECT

	Rules* rules;

private Q_SLOTS:
    void initTestCase() {
    	rules = Rules::readRules(readExtras);
    }

    void cleanupTestCase() {
    	delete rules;
    }

    void testRules() {
        QVERIFY( rules != NULL );
        QVERIFY( rules->modelInfos.size() > 0 );
        QVERIFY( rules->layoutInfos.size() > 0 );
        QVERIFY( rules->optionGroupInfos.size() > 0 );
    }

    void testModel() {
        foreach(const ModelInfo* modelInfo, rules->modelInfos) {
        	QVERIFY( modelInfo != NULL);
        	QVERIFY( modelInfo->name.length() > 0 );
        	QVERIFY( modelInfo->description.length() > 0 );
//        	QVERIFY( ! modelInfo->vendor.isEmpty() );
        }
    }

    void testLayouts() {
        foreach(const LayoutInfo* layoutInfo, rules->layoutInfos) {
        	QVERIFY( layoutInfo != NULL);
        	QVERIFY( ! layoutInfo->name.isEmpty() );
//        	const char* desc = layoutInfo->name.toUtf8() ;
//        	qDebug() << layoutInfo->name;
        	QVERIFY( ! layoutInfo->description.isEmpty() );

        	foreach(const VariantInfo* variantInfo, layoutInfo->variantInfos) {
        		QVERIFY( variantInfo != NULL );
        		QVERIFY( ! variantInfo->name.isEmpty() );
        		QVERIFY( ! variantInfo->description.isEmpty() );
        	}
        	foreach(const QString& language, layoutInfo->languages) {
        		QVERIFY( ! language.isEmpty() );
        	}
        }
    }

    void testOptionGroups() {
        foreach(const OptionGroupInfo* optionGroupInfo, rules->optionGroupInfos) {
        	QVERIFY( optionGroupInfo != NULL);
        	QVERIFY( ! optionGroupInfo->name.isEmpty() );
        	QVERIFY( ! optionGroupInfo->description.isEmpty() );
        	// optionGroupInfo->exclusive

        	foreach(const OptionInfo* optionInfo, optionGroupInfo->optionInfos) {
        		QVERIFY( optionInfo != NULL );
        		QVERIFY( ! optionInfo->name.isEmpty() );
        		QVERIFY( ! optionInfo->description.isEmpty() );
        	}
        }
    }

    void testExtras() {
    	Rules* rulesWithExtras = Rules::readRules(Rules::READ_EXTRAS);
    	QVERIFY2(rulesWithExtras->layoutInfos.size() > rules->layoutInfos.size(), "Rules with extras should have more layouts");

    	foreach(const LayoutInfo* layoutInfo, rules->layoutInfos) {
    		QVERIFY( ! layoutInfo->fromExtras );
    	}

    	bool foundFromExtras = false, foundNonExtras = false;
    	foreach(const LayoutInfo* layoutInfo, rulesWithExtras->layoutInfos) {
    		if( layoutInfo->fromExtras ) foundFromExtras = true;
    		if( ! layoutInfo->fromExtras ) foundNonExtras = true;
    		layoutInfo->languages.size();	// make sure we can access all merged objects
    		layoutInfo->variantInfos.size();	// make sure we can access all merged objects
    	}
    	QVERIFY( foundNonExtras );
    	QVERIFY( foundFromExtras );
    }

    void testWriteNewXml() {
    	QDomDocument doc("xkbConfigRegistry");
    	QDomElement root = doc.createElement("xkbConfigRegistry");
    	root.setAttribute("version", "2.0");
    	doc.appendChild(root);

    	QDomElement modelList = doc.createElement("modelList");
    	root.appendChild(modelList);
    	foreach(const ModelInfo* modelInfo, rules->modelInfos) {
        	QDomElement model = doc.createElement("model");
        	model.setAttribute("name", modelInfo->name);
        	model.setAttribute("description", modelInfo->description);
        	model.setAttribute("vendor", modelInfo->vendor);
    		modelList.appendChild(model);
    	}

    	QDomElement layoutList = doc.createElement("layoutList");
    	foreach(const LayoutInfo* layoutInfo, rules->layoutInfos) {
        	QDomElement layout = doc.createElement("layout");
        	layout.setAttribute("name", layoutInfo->name);
        	layout.setAttribute("description", layoutInfo->description);

        	QDomElement langList = doc.createElement("languageList");
        	foreach(const QString& lang, layoutInfo->languages) {
            	QDomElement langNode = doc.createElement("lang");
            	langNode.setAttribute("iso639Id", lang);
            	langList.appendChild(langNode);
        	}
        	if( langList.hasChildNodes() ) {
        		layout.appendChild(langList);
        	}

        	QDomElement variantList = doc.createElement("variantList");
        	foreach(const VariantInfo* variantInfo, layoutInfo->variantInfos) {
            	QDomElement variant = doc.createElement("variant");
            	variant.setAttribute("name", variantInfo->name);
            	variant.setAttribute("description", variantInfo->description);

            	QDomElement langList = doc.createElement("languageList");
            	foreach(const QString& lang, variantInfo->languages) {
                	QDomElement langNode = doc.createElement("lang");
                	langNode.setAttribute("iso639Id", lang);
                	langList.appendChild(langNode);
            	}
            	if( langList.hasChildNodes() ) {
            		variant.appendChild(langList);
            	}

            	variantList.appendChild(variant);
        	}
        	if( variantList.hasChildNodes() ) {
        		layout.appendChild(variantList);
        	}

        	layoutList.appendChild(layout);
    	}
    	root.appendChild(layoutList);

    	QDomElement optionGroupList = doc.createElement("optionList");
    	foreach(const OptionGroupInfo* optionGroupInfo, rules->optionGroupInfos) {
        	QDomElement optionGroup = doc.createElement("optionGroup");
        	optionGroup.setAttribute("name", optionGroupInfo->name);
        	optionGroup.setAttribute("description", optionGroupInfo->description);
        	optionGroup.setAttribute("exclusive", optionGroupInfo->exclusive);

        	foreach(const OptionInfo* optionGroupInfo, optionGroupInfo->optionInfos) {
            	QDomElement option = doc.createElement("option");
            	option.setAttribute("name", optionGroupInfo->name);
            	option.setAttribute("description", optionGroupInfo->description);
            	optionGroup.appendChild(option);
        	}

        	optionGroupList.appendChild(optionGroup);
    	}
    	root.appendChild(optionGroupList);

    	QFile file("base2.xml");
        if( ! file.open( QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text) ) {
        	qWarning() << "Failed to open layout memory xml file for writing" << file.fileName();
        	QFAIL("failed");
        }

        QTextStream out(&file);
        out << doc.toString();
    }
    
    void testRulesVersion() {
    	QVERIFY(!rules->version.isEmpty());

    	Rules* rules10 = new Rules();
    	Rules::readRules(rules10, QString("config/base.xml"), false);
    	QCOMPARE(rules10->version, QString("1.0"));
    	delete rules10;

    	Rules* rules11 = new Rules();
    	Rules::readRules(rules11, QString("config/base.1.1.xml"), false);
    	QCOMPARE(rules11->version, QString("1.1"));

    	foreach(const LayoutInfo* layoutInfo, rules11->layoutInfos) {
        	QVERIFY( layoutInfo != NULL);
        	QVERIFY( ! layoutInfo->name.isEmpty() );
        	QVERIFY( ! layoutInfo->description.isEmpty() );

        	foreach(const VariantInfo* variantInfo, layoutInfo->variantInfos) {
        		QVERIFY( variantInfo != NULL );
        		QVERIFY( ! variantInfo->name.isEmpty() );
        		QVERIFY( ! variantInfo->description.isEmpty() );
        	}
        }

    	delete rules11;
    }

    void loadRulesBenchmark() {
    	QBENCHMARK {
    		Rules* rules = Rules::readRules(readExtras);
    		delete rules;
    	}
    }

};

// need kde libs for config-workspace.h used in xkb_rules.cpp
QTEST_MAIN(RulesTest)

#include "xkb_rules_test.moc"
