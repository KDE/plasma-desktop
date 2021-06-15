/*
    SPDX-FileCopyrightText: 2011 Andriy Rysin <rysin@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "layout_memory_persister.h"
#include "debug.h"

#include <KConfigGroup>
#include <KSharedConfig>

#include <QDir>
#include <QFile>
#include <QStandardPaths>
#include <qdom.h>
#include <qxml.h>

#include "keyboard_config.h"
#include "layout_memory.h"

static const char VERSION[] = "1.0";
static const char DOC_NAME[] = "LayoutMap";
static const char ROOT_NODE[] = "LayoutMap";
static const char VERSION_ATTRIBUTE[] = "version";
static const char SWITCH_MODE_ATTRIBUTE[] = "SwitchMode";
static const char ITEM_NODE[] = "item";
static const QString CURRENT_LAYOUT_ATTRIBUTE(QStringLiteral("currentLayout"));
static const char OWNER_KEY_ATTRIBUTE[] = "ownerKey";
static const char LAYOUTS_ATTRIBUTE[] = "layouts";

static const char LIST_SEPARATOR_LM[] = ",";

static const char REL_SESSION_FILE_PATH[] = "/keyboard/session/layout_memory.xml";

static bool isDefaultLayoutConfig(const LayoutSet &layout, const QList<LayoutUnit> &defaultLayouts)
{
    if (defaultLayouts.isEmpty() || layout.layouts != defaultLayouts || layout.currentLayout != defaultLayouts.first()) {
        return false;
    }
    return true;
}

QString LayoutMemoryPersister::getLayoutMapAsString()
{
    if (!canPersist())
        return QLatin1String("");

    QDomDocument doc(DOC_NAME);
    QDomElement root = doc.createElement(ROOT_NODE);
    root.setAttribute(VERSION_ATTRIBUTE, VERSION);
    root.setAttribute(SWITCH_MODE_ATTRIBUTE, KeyboardConfig::getSwitchingPolicyString(layoutMemory.keyboardConfig.switchingPolicy));
    doc.appendChild(root);

    if (layoutMemory.keyboardConfig.switchingPolicy == KeyboardConfig::SWITCH_POLICY_GLOBAL) {
        if (!globalLayout.isValid())
            return QLatin1String("");

        QDomElement item = doc.createElement(ITEM_NODE);
        item.setAttribute(CURRENT_LAYOUT_ATTRIBUTE, globalLayout.toString());
        root.appendChild(item);
    } else {
        foreach (const QString &key, layoutMemory.layoutMap.keys()) {
            if (isDefaultLayoutConfig(layoutMemory.layoutMap[key], layoutMemory.keyboardConfig.getDefaultLayouts())) {
                continue;
            }

            QDomElement item = doc.createElement(ITEM_NODE);
            item.setAttribute(OWNER_KEY_ATTRIBUTE, key);
            item.setAttribute(CURRENT_LAYOUT_ATTRIBUTE, layoutMemory.layoutMap[key].currentLayout.toString());

            QString layoutSetString;
            foreach (const LayoutUnit &layoutUnit, layoutMemory.layoutMap[key].layouts) {
                if (!layoutSetString.isEmpty()) {
                    layoutSetString += LIST_SEPARATOR_LM;
                }
                layoutSetString += layoutUnit.toString();
            }
            item.setAttribute(LAYOUTS_ATTRIBUTE, layoutSetString);
            root.appendChild(item);
        }
    }

    return doc.toString();
}

bool LayoutMemoryPersister::save()
{
    QFileInfo fileInfo(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + REL_SESSION_FILE_PATH);

    QDir baseDir(fileInfo.absoluteDir());
    if (!baseDir.exists()) {
        if (!QDir().mkpath(baseDir.absolutePath())) {
            qCWarning(KCM_KEYBOARD) << "Failed to create directory" << baseDir.absolutePath();
        }
    }

    QFile file(fileInfo.absoluteFilePath());
    return saveToFile(file);
}

bool LayoutMemoryPersister::restore()
{
    QFile file(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + REL_SESSION_FILE_PATH);
    if (!file.exists()) {
        return false;
    }
    return restoreFromFile(file);
}

bool LayoutMemoryPersister::saveToFile(const QFile &file_)
{
    QString xml = getLayoutMapAsString();
    if (xml.isEmpty())
        return false;

    QFile file(file_.fileName()); // so we don't expose the file we open/close to the caller
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        qCWarning(KCM_KEYBOARD) << "Failed to open layout memory xml file for writing" << file.fileName();
        return false;
    }

    QTextStream out(&file);
    out << xml;
    out.flush();

    if (file.error() != QFile::NoError) {
        qCWarning(KCM_KEYBOARD) << "Failed to store keyboard layout memory, error" << file.error();
        file.close();
        file.remove();
        return false;
    } else {
        qCDebug(KCM_KEYBOARD) << "Keyboard layout memory stored into" << file.fileName() << "written" << file.pos();
        return true;
    }
}

class MapHandler : public QXmlDefaultHandler
{
public:
    MapHandler(const KeyboardConfig::SwitchingPolicy &switchingPolicy_)
        : verified(false)
        , switchingPolicy(switchingPolicy_)
    {
    }

    bool startElement(const QString & /*namespaceURI*/, const QString & /*localName*/, const QString &qName, const QXmlAttributes &attributes) override
    {
        if (qName == ROOT_NODE) {
            if (attributes.value(VERSION_ATTRIBUTE) != VERSION)
                return false;
            if (attributes.value(SWITCH_MODE_ATTRIBUTE) != KeyboardConfig::getSwitchingPolicyString(switchingPolicy))
                return false;

            verified = true;
        }
        if (qName == ITEM_NODE) {
            if (!verified)
                return false;

            if (switchingPolicy == KeyboardConfig::SWITCH_POLICY_GLOBAL) {
                globalLayout = LayoutUnit(attributes.value(CURRENT_LAYOUT_ATTRIBUTE));
            } else {
                QStringList layoutStrings = attributes.value(LAYOUTS_ATTRIBUTE).split(LIST_SEPARATOR_LM);
                LayoutSet layoutSet;
                foreach (const QString &layoutString, layoutStrings) {
                    layoutSet.layouts.append(LayoutUnit(layoutString));
                }
                layoutSet.currentLayout = LayoutUnit(attributes.value(CURRENT_LAYOUT_ATTRIBUTE));
                QString ownerKey = attributes.value(OWNER_KEY_ATTRIBUTE);

                if (ownerKey.trimmed().isEmpty() || !layoutSet.isValid())
                    return false;

                layoutMap[ownerKey] = layoutSet;
            }
        }
        return verified;
    }

    bool verified;
    QMap<QString, LayoutSet> layoutMap;
    LayoutUnit globalLayout;

private:
    const KeyboardConfig::SwitchingPolicy &switchingPolicy;
};

template<typename T>
static bool containsAll(QList<T> set1, QList<T> set2)
{
    foreach (const T &t, set2) {
        if (!set1.contains(t))
            return false;
    }
    return true;
}

bool LayoutMemoryPersister::restoreFromFile(const QFile &file_)
{
    globalLayout = LayoutUnit();

    if (!canPersist())
        return false;

    QFile file(file_.fileName()); // so we don't expose the file we open/close to the caller
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qCWarning(KCM_KEYBOARD) << "Failed to open layout memory xml file for reading" << file.fileName() << "error:" << file.error();
        return false;
    }

    MapHandler mapHandler(layoutMemory.keyboardConfig.switchingPolicy);

    QXmlSimpleReader reader;
    reader.setContentHandler(&mapHandler);
    reader.setErrorHandler(&mapHandler);

    QXmlInputSource xmlInputSource(&file);
    qCDebug(KCM_KEYBOARD) << "Restoring keyboard layout map from" << file.fileName();

    if (!reader.parse(xmlInputSource)) {
        qCWarning(KCM_KEYBOARD) << "Failed to parse the layout memory file" << file.fileName();
        return false;
    }

    if (layoutMemory.keyboardConfig.switchingPolicy == KeyboardConfig::SWITCH_POLICY_GLOBAL) {
        if (mapHandler.globalLayout.isValid() && layoutMemory.keyboardConfig.layouts.contains(mapHandler.globalLayout)) {
            globalLayout = mapHandler.globalLayout;
            qCDebug(KCM_KEYBOARD) << "Restored global layout" << globalLayout.toString();
        }
    } else {
        layoutMemory.layoutMap.clear();
        foreach (const QString &key, mapHandler.layoutMap.keys()) {
            if (containsAll(layoutMemory.keyboardConfig.layouts, mapHandler.layoutMap[key].layouts)) {
                layoutMemory.layoutMap.insert(key, mapHandler.layoutMap[key]);
            }
        }
        qCDebug(KCM_KEYBOARD) << "Restored layouts for" << layoutMemory.layoutMap.size() << "containers";
    }
    return true;
}

bool LayoutMemoryPersister::canPersist()
{
    // we can't persist per window - as we're using window id which is not preserved between sessions
    bool windowMode = layoutMemory.keyboardConfig.switchingPolicy == KeyboardConfig::SWITCH_POLICY_WINDOW;
    if (windowMode) {
        qCDebug(KCM_KEYBOARD) << "Not saving session for window mode";
    }
    return !windowMode;
}
