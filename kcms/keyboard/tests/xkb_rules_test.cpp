/*
    SPDX-FileCopyrightText: 2011 Andriy Rysin <rysin@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <QtTest>

#include "../xkb_rules.h"

#include <qdom.h>
#include <qxml.h>

static const Rules::ExtrasFlag readExtras = Rules::NO_EXTRAS;

class RulesTest : public QObject
{
    Q_OBJECT

    Rules *rules;

private Q_SLOTS:
    void initTestCase()
    {
        rules = Rules::readRules(readExtras);
    }

    void cleanupTestCase()
    {
        delete rules;
    }

    void testRules()
    {
        QVERIFY(rules != nullptr);
        QVERIFY(rules->modelInfos.size() > 0);
        QVERIFY(rules->layoutInfos.size() > 0);
        QVERIFY(rules->optionGroupInfos.size() > 0);
    }

    void testModel()
    {
        foreach (const ModelInfo *modelInfo, rules->modelInfos) {
            QVERIFY(modelInfo != nullptr);
            QVERIFY(modelInfo->name.length() > 0);
            QVERIFY(modelInfo->description.length() > 0);
            //        	QVERIFY( ! modelInfo->vendor.isEmpty() );
        }
    }

    void testLayouts()
    {
        foreach (const LayoutInfo *layoutInfo, rules->layoutInfos) {
            QVERIFY(layoutInfo != nullptr);
            QVERIFY(!layoutInfo->name.isEmpty());
            //        	const char* desc = layoutInfo->name.toUtf8() ;
            //        	qDebug() << layoutInfo->name;
            QVERIFY(!layoutInfo->description.isEmpty());

            foreach (const VariantInfo *variantInfo, layoutInfo->variantInfos) {
                QVERIFY(variantInfo != nullptr);
                QVERIFY(!variantInfo->name.isEmpty());
                QVERIFY(!variantInfo->description.isEmpty());
            }
            foreach (const QString &language, layoutInfo->languages) {
                QVERIFY(!language.isEmpty());
            }
        }
    }

    void testOptionGroups()
    {
        foreach (const OptionGroupInfo *optionGroupInfo, rules->optionGroupInfos) {
            QVERIFY(optionGroupInfo != nullptr);
            QVERIFY(!optionGroupInfo->name.isEmpty());
            QVERIFY(!optionGroupInfo->description.isEmpty());
            // optionGroupInfo->exclusive

            foreach (const OptionInfo *optionInfo, optionGroupInfo->optionInfos) {
                QVERIFY(optionInfo != nullptr);
                QVERIFY(!optionInfo->name.isEmpty());
                QVERIFY(!optionInfo->description.isEmpty());
            }
        }
    }

    void testExtras()
    {
        Rules *rulesWithExtras = Rules::readRules(Rules::READ_EXTRAS);
        QVERIFY2(rulesWithExtras->layoutInfos.size() > rules->layoutInfos.size(), "Rules with extras should have more layouts");

        foreach (const LayoutInfo *layoutInfo, rules->layoutInfos) {
            QVERIFY(!layoutInfo->fromExtras);
        }

        bool foundFromExtras = false, foundNonExtras = false;
        foreach (const LayoutInfo *layoutInfo, rulesWithExtras->layoutInfos) {
            if (layoutInfo->fromExtras)
                foundFromExtras = true;
            if (!layoutInfo->fromExtras)
                foundNonExtras = true;
            layoutInfo->languages.size(); // make sure we can access all merged objects
            layoutInfo->variantInfos.size(); // make sure we can access all merged objects
        }
        QVERIFY(foundNonExtras);
        QVERIFY(foundFromExtras);
    }

    void testWriteNewXml()
    {
        QDomDocument doc(QStringLiteral("xkbConfigRegistry"));
        QDomElement root = doc.createElement(QStringLiteral("xkbConfigRegistry"));
        root.setAttribute(QStringLiteral("version"), QStringLiteral("2.0"));
        doc.appendChild(root);

        QDomElement modelList = doc.createElement(QStringLiteral("modelList"));
        root.appendChild(modelList);
        foreach (const ModelInfo *modelInfo, rules->modelInfos) {
            QDomElement model = doc.createElement(QStringLiteral("model"));
            model.setAttribute(QStringLiteral("name"), modelInfo->name);
            model.setAttribute(QStringLiteral("description"), modelInfo->description);
            model.setAttribute(QStringLiteral("vendor"), modelInfo->vendor);
            modelList.appendChild(model);
        }

        QDomElement layoutList = doc.createElement(QStringLiteral("layoutList"));
        foreach (const LayoutInfo *layoutInfo, rules->layoutInfos) {
            QDomElement layout = doc.createElement(QStringLiteral("layout"));
            layout.setAttribute(QStringLiteral("name"), layoutInfo->name);
            layout.setAttribute(QStringLiteral("description"), layoutInfo->description);

            QDomElement langList = doc.createElement(QStringLiteral("languageList"));
            foreach (const QString &lang, layoutInfo->languages) {
                QDomElement langNode = doc.createElement(QStringLiteral("lang"));
                langNode.setAttribute(QStringLiteral("iso639Id"), lang);
                langList.appendChild(langNode);
            }
            if (langList.hasChildNodes()) {
                layout.appendChild(langList);
            }

            QDomElement variantList = doc.createElement(QStringLiteral("variantList"));
            foreach (const VariantInfo *variantInfo, layoutInfo->variantInfos) {
                QDomElement variant = doc.createElement(QStringLiteral("variant"));
                variant.setAttribute(QStringLiteral("name"), variantInfo->name);
                variant.setAttribute(QStringLiteral("description"), variantInfo->description);

                QDomElement langList = doc.createElement(QStringLiteral("languageList"));
                foreach (const QString &lang, variantInfo->languages) {
                    QDomElement langNode = doc.createElement(QStringLiteral("lang"));
                    langNode.setAttribute(QStringLiteral("iso639Id"), lang);
                    langList.appendChild(langNode);
                }
                if (langList.hasChildNodes()) {
                    variant.appendChild(langList);
                }

                variantList.appendChild(variant);
            }
            if (variantList.hasChildNodes()) {
                layout.appendChild(variantList);
            }

            layoutList.appendChild(layout);
        }
        root.appendChild(layoutList);

        QDomElement optionGroupList = doc.createElement(QStringLiteral("optionList"));
        foreach (const OptionGroupInfo *optionGroupInfo, rules->optionGroupInfos) {
            QDomElement optionGroup = doc.createElement(QStringLiteral("optionGroup"));
            optionGroup.setAttribute(QStringLiteral("name"), optionGroupInfo->name);
            optionGroup.setAttribute(QStringLiteral("description"), optionGroupInfo->description);
            optionGroup.setAttribute(QStringLiteral("exclusive"), optionGroupInfo->exclusive);

            foreach (const OptionInfo *optionGroupInfo, optionGroupInfo->optionInfos) {
                QDomElement option = doc.createElement(QStringLiteral("option"));
                option.setAttribute(QStringLiteral("name"), optionGroupInfo->name);
                option.setAttribute(QStringLiteral("description"), optionGroupInfo->description);
                optionGroup.appendChild(option);
            }

            optionGroupList.appendChild(optionGroup);
        }
        root.appendChild(optionGroupList);

        QFile file(QStringLiteral("base2.xml"));
        if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
            qWarning() << "Failed to open layout memory xml file for writing" << file.fileName();
            QFAIL("failed");
        }

        QTextStream out(&file);
        out << doc.toString();
    }

    void testRulesVersion()
    {
        QVERIFY(!rules->version.isEmpty());

        Rules *rules10 = new Rules();
        Rules::readRules(rules10, QStringLiteral("config/base.xml"), false);
        QCOMPARE(rules10->version, QString("1.0"));
        delete rules10;

        Rules *rules11 = new Rules();
        Rules::readRules(rules11, QStringLiteral("config/base.1.1.xml"), false);
        QCOMPARE(rules11->version, QString("1.1"));

        foreach (const LayoutInfo *layoutInfo, rules11->layoutInfos) {
            QVERIFY(layoutInfo != nullptr);
            QVERIFY(!layoutInfo->name.isEmpty());
            QVERIFY(!layoutInfo->description.isEmpty());

            foreach (const VariantInfo *variantInfo, layoutInfo->variantInfos) {
                QVERIFY(variantInfo != nullptr);
                QVERIFY(!variantInfo->name.isEmpty());
                QVERIFY(!variantInfo->description.isEmpty());
            }
        }

        delete rules11;
    }

    void loadRulesBenchmark()
    {
        QBENCHMARK {
            Rules *rules = Rules::readRules(readExtras);
            delete rules;
        }
    }
};

// need kde libs for config-workspace.h used in xkb_rules.cpp
QTEST_MAIN(RulesTest)

#include "xkb_rules_test.moc"
