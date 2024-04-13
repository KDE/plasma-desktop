/*
    SPDX-FileCopyrightText: 2011 Andriy Rysin <rysin@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "layout_memory_persister.h"
#include "debug.h"

#include <KConfigGroup>
#include <KSharedConfig>

#include <QDir>
#include <QDomDocument> // TODO port to QXmlStreamWriter to save memory, we don't actually need DOM
#include <QFile>
#include <QStandardPaths>
#include <QXmlStreamReader>

#include "keyboard_config.h"
#include "keyboardsettings.h"
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

static const QChar LIST_SEPARATOR_LM = ',';

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
        return QString();

    QDomDocument doc(DOC_NAME);
    QDomElement root = doc.createElement(ROOT_NODE);
    root.setAttribute(VERSION_ATTRIBUTE, VERSION);
    root.setAttribute(SWITCH_MODE_ATTRIBUTE, layoutMemory.keyboardConfig.keyboardSettings()->switchMode());
    doc.appendChild(root);

    if (layoutMemory.keyboardConfig.switchingPolicy() == KeyboardConfig::SWITCH_POLICY_GLOBAL) {
        if (!globalLayout.isValid())
            return QString();

        QDomElement item = doc.createElement(ITEM_NODE);
        item.setAttribute(CURRENT_LAYOUT_ATTRIBUTE, globalLayout.toString());
        root.appendChild(item);
    } else {
        const QStringList keys = layoutMemory.layoutMap.keys();
        for (const QString &key : keys) {
            if (isDefaultLayoutConfig(layoutMemory.layoutMap[key], layoutMemory.keyboardConfig.getDefaultLayouts())) {
                continue;
            }

            QDomElement item = doc.createElement(ITEM_NODE);
            item.setAttribute(OWNER_KEY_ATTRIBUTE, key);
            item.setAttribute(CURRENT_LAYOUT_ATTRIBUTE, layoutMemory.layoutMap[key].currentLayout.toString());

            QString layoutSetString;
            const QList<LayoutUnit> layouts = layoutMemory.layoutMap[key].layouts;
            for (const LayoutUnit &layoutUnit : layouts) {
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

class MapHandler
{
public:
    MapHandler(const KeyboardConfig::SwitchingPolicy &switchingPolicy_)
        : verified(false)
        , switchingPolicy(switchingPolicy_)
    {
    }

    bool startElement(QXmlStreamReader &xml)
    {
        if (!verified && xml.name() == QLatin1String(ROOT_NODE)) {
            if (xml.attributes().value(VERSION_ATTRIBUTE) != QLatin1String(VERSION)) {
                xml.raiseError("Unexpected version!");
                return false;
            }
            if (xml.attributes().value(SWITCH_MODE_ATTRIBUTE) != KeyboardConfig::getSwitchingPolicyString(switchingPolicy)) {
                xml.raiseError("Unexpected switching mode!");
                return false;
            }
            verified = true;
        } else if (xml.name() == QLatin1String(ITEM_NODE)) {
            if (!verified) {
                xml.raiseError("Malformed xml structure!");
                return false;
            }

            if (switchingPolicy == KeyboardConfig::SWITCH_POLICY_GLOBAL) {
                globalLayout = LayoutUnit(xml.attributes().value(CURRENT_LAYOUT_ATTRIBUTE).toString());
            } else {
                const auto layoutStrings = xml.attributes().value(LAYOUTS_ATTRIBUTE).split(LIST_SEPARATOR_LM);
                LayoutSet layoutSet;
                for (const auto &layoutString : layoutStrings) {
                    layoutSet.layouts.append(LayoutUnit(layoutString.toString()));
                }
                layoutSet.currentLayout = LayoutUnit(xml.attributes().value(CURRENT_LAYOUT_ATTRIBUTE).toString());
                QString ownerKey = xml.attributes().value(OWNER_KEY_ATTRIBUTE).toString();

                if (ownerKey.trimmed().isEmpty() || !layoutSet.isValid()) {
                    xml.raiseError("Invalid layout data!");
                    return false;
                }

                layoutMap[ownerKey] = layoutSet;
            }
            xml.skipCurrentElement();
        } else {
            verified = false;
            xml.raiseError("Malformed xml structure! Unexpected element!");
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
static bool containsAll(const QList<T> &set1, const QList<T> &set2)
{
    for (const T &t : set2) {
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

    auto switchingPolicy = layoutMemory.keyboardConfig.switchingPolicy();
    MapHandler mapHandler(switchingPolicy);

    QXmlStreamReader xml(&file);
    qCDebug(KCM_KEYBOARD) << "Restoring keyboard layout map from" << file.fileName();

    auto firstElement = xml.readNextStartElement();
    do {
        if (!firstElement || !mapHandler.startElement(xml)) {
            qCWarning(KCM_KEYBOARD) << "Failed to parse the layout memory file" << file.fileName();
            qCWarning(KCM_KEYBOARD) << xml.errorString();
            return false;
        }
    } while (xml.readNextStartElement());

    if (xml.hasError()) {
        qCWarning(KCM_KEYBOARD) << "XML error:" << xml.errorString();
        return false;
    }

    if (layoutMemory.keyboardConfig.switchingPolicy() == KeyboardConfig::SWITCH_POLICY_GLOBAL) {
        if (mapHandler.globalLayout.isValid() && layoutMemory.keyboardConfig.layouts().contains(mapHandler.globalLayout)) {
            globalLayout = mapHandler.globalLayout;
            qCDebug(KCM_KEYBOARD) << "Restored global layout" << globalLayout.toString();
        }
    } else {
        layoutMemory.layoutMap.clear();
        for (const QString &key : mapHandler.layoutMap.keys()) {
            if (containsAll(layoutMemory.keyboardConfig.layouts(), mapHandler.layoutMap[key].layouts)) {
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
    bool windowMode = layoutMemory.keyboardConfig.switchingPolicy() == KeyboardConfig::SWITCH_POLICY_WINDOW;
    if (windowMode) {
        qCDebug(KCM_KEYBOARD) << "Not saving session for window mode";
    }
    return !windowMode;
}
