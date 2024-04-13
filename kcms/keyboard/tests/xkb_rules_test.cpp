/*
    SPDX-FileCopyrightText: 2011 Andriy Rysin <rysin@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <QTest>

#include "../xkb_rules.h"

#include <qdom.h>

class RulesTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testRules()
    {
        QVERIFY(Rules::self().modelInfos.size() > 0);
        QVERIFY(Rules::self().layoutInfos.size() > 0);
        QVERIFY(Rules::self().optionGroupInfos.size() > 0);
    }

    void testModel()
    {
        for (const ModelInfo &modelInfo : std::as_const(Rules::self().modelInfos)) {
            QVERIFY(modelInfo.name.length() > 0);
            QVERIFY(modelInfo.description.length() > 0);
            //        	QVERIFY( ! modelInfo->vendor.isEmpty() );
        }
    }

    void testLayouts()
    {
        for (const LayoutInfo &layoutInfo : std::as_const(Rules::self().layoutInfos)) {
            QVERIFY(!layoutInfo.name.isEmpty());
            //        	const char* desc = layoutInfo->name.toUtf8() ;
            //        	qDebug() << layoutInfo->name;
            QVERIFY(!layoutInfo.description.isEmpty());

            for (const VariantInfo &variantInfo : layoutInfo.variantInfos) {
                QVERIFY(!variantInfo.name.isEmpty());
                QVERIFY(!variantInfo.description.isEmpty());
            }
            for (const QString &language : layoutInfo.languages) {
                QVERIFY(!language.isEmpty());
            }
        }
    }

    void testOptionGroups()
    {
        for (const OptionGroupInfo &optionGroupInfo : std::as_const(Rules::self().optionGroupInfos)) {
            QVERIFY(!optionGroupInfo.name.isEmpty());
            QVERIFY(!optionGroupInfo.description.isEmpty());
            // optionGroupInfo->exclusive

            for (const OptionInfo &optionInfo : optionGroupInfo.optionInfos) {
                QVERIFY(!optionInfo.name.isEmpty());
                QVERIFY(!optionInfo.description.isEmpty());
            }
        }
    }

    void testWriteNewXml()
    {
        QDomDocument doc(QStringLiteral("xkbConfigRegistry"));
        QDomElement root = doc.createElement(QStringLiteral("xkbConfigRegistry"));
        root.setAttribute(QStringLiteral("version"), QStringLiteral("2.0"));
        doc.appendChild(root);

        QDomElement modelList = doc.createElement(QStringLiteral("modelList"));
        root.appendChild(modelList);
        for (const ModelInfo &modelInfo : std::as_const(Rules::self().modelInfos)) {
            QDomElement model = doc.createElement(QStringLiteral("model"));
            model.setAttribute(QStringLiteral("name"), modelInfo.name);
            model.setAttribute(QStringLiteral("description"), modelInfo.description);
            model.setAttribute(QStringLiteral("vendor"), modelInfo.vendor);
            modelList.appendChild(model);
        }

        QDomElement layoutList = doc.createElement(QStringLiteral("layoutList"));
        for (const LayoutInfo &layoutInfo : std::as_const(Rules::self().layoutInfos)) {
            QDomElement layout = doc.createElement(QStringLiteral("layout"));
            layout.setAttribute(QStringLiteral("name"), layoutInfo.name);
            layout.setAttribute(QStringLiteral("description"), layoutInfo.description);

            QDomElement langList = doc.createElement(QStringLiteral("languageList"));
            for (const QString &lang : layoutInfo.languages) {
                QDomElement langNode = doc.createElement(QStringLiteral("lang"));
                langNode.setAttribute(QStringLiteral("iso639Id"), lang);
                langList.appendChild(langNode);
            }
            if (langList.hasChildNodes()) {
                layout.appendChild(langList);
            }

            QDomElement variantList = doc.createElement(QStringLiteral("variantList"));
            for (const VariantInfo &variantInfo : layoutInfo.variantInfos) {
                QDomElement variant = doc.createElement(QStringLiteral("variant"));
                variant.setAttribute(QStringLiteral("name"), variantInfo.name);
                variant.setAttribute(QStringLiteral("description"), variantInfo.description);

                QDomElement langList = doc.createElement(QStringLiteral("languageList"));
                for (const QString &lang : variantInfo.languages) {
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
        for (const OptionGroupInfo &optionGroupInfo : std::as_const(Rules::self().optionGroupInfos)) {
            QDomElement optionGroup = doc.createElement(QStringLiteral("optionGroup"));
            optionGroup.setAttribute(QStringLiteral("name"), optionGroupInfo.name);
            optionGroup.setAttribute(QStringLiteral("description"), optionGroupInfo.description);
            optionGroup.setAttribute(QStringLiteral("exclusive"), optionGroupInfo.exclusive);

            for (const OptionInfo &optionInfo : optionGroupInfo.optionInfos) {
                QDomElement option = doc.createElement(QStringLiteral("option"));
                option.setAttribute(QStringLiteral("name"), optionInfo.name);
                option.setAttribute(QStringLiteral("description"), optionInfo.description);
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
};

// need kde libs for config-workspace.h used in xkb_rules.cpp
QTEST_MAIN(RulesTest)

#include "xkb_rules_test.moc"
