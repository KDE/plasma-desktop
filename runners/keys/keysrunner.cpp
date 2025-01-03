/*
    SPDX-FileCopyrightText: 2025 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "keysrunner.h"

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingCall>

#include <KLocalizedString>

#include "filteredmodel.h"
#include "globalaccelmodel.h"

using namespace Qt::StringLiterals;

KeysRunner::KeysRunner(QObject *parent, const KPluginMetaData &metaData)
    : KRunner::AbstractRunner(parent, metaData)
{
    addSyntax(u":q:"_s, i18nc("@info:usagetip", "Finds shortcuts whose name or description match :q:"));
}

void KeysRunner::init()
{
    m_globalAccelModel = new GlobalAccelModel(this);
    m_filteredModel = new FilteredShortcutsModel(this);
    m_filteredModel->setSourceModel(m_globalAccelModel);
    m_filteredModel->setShowsLaunchAction(false);
    if (m_globalAccelModel->isValid()) {
        m_globalAccelModel->load();
        connect(
            m_globalAccelModel,
            &QAbstractItemModel::modelReset,
            this,
            [this] {
                QDBusConnection::sessionBus().connect(u"org.kde.kglobalaccel"_s,
                                                      u"/kglobalaccel"_s,
                                                      u"org.kde.KGlobalAccel"_s,
                                                      u"yourShortcutsChanged"_s,
                                                      this,
                                                      SLOT(onShortcutsChanged(QStringList, QList<int>)));
                m_needsUpdate = false;
            },
            Qt::SingleShotConnection);
    }

    connect(this, &KRunner::AbstractRunner::prepare, this, &KeysRunner::onPrepare);
}

void KeysRunner::match(KRunner::RunnerContext &context)
{
    m_filteredModel->setFilter(context.query());
    for (int i = 0, count = m_filteredModel->rowCount(); i < count; ++i) {
        const QModelIndex componentIdx = m_filteredModel->index(i, 0);
        Q_ASSERT(componentIdx.isValid() && !componentIdx.parent().isValid());
        const QString componentUniqueName = componentIdx.data(BaseModel::ComponentRole).toString();
        if (componentUniqueName == u"org.kde.krunner.desktop") {
            continue;
        }

        const QString componentFriendlyName = componentIdx.data(Qt::DisplayRole).toString();
        const QString iconName = componentIdx.data(Qt::DecorationRole).toString();

        for (int j = 0, childCount = m_filteredModel->rowCount(componentIdx); j < childCount; ++j) {
            const QModelIndex actionIdx = m_filteredModel->index(j, 0, componentIdx);
            const QString actionName = actionIdx.data(Qt::DisplayRole).toString();
            if (actionName.isEmpty()) {
                continue;
            }

            KRunner::QueryMatch match(this);
            match.setText(actionName + u" - " + componentFriendlyName);

            QSet<QKeySequence> shortcuts = actionIdx.data(BaseModel::ActiveShortcutsRole).value<QSet<QKeySequence>>();
            if (shortcuts.empty()) {
                shortcuts = actionIdx.data(BaseModel::DefaultShortcutsRole).value<QSet<QKeySequence>>();
            }
            if (shortcuts.empty()) {
                shortcuts = actionIdx.data(BaseModel::CustomShortcutsRole).value<QSet<QKeySequence>>();
            }
            if (!shortcuts.empty()) {
                QString subText = shortcuts.cbegin()->toString(QKeySequence::NativeText);
                subText.reserve(shortcuts.size() * 5);
                for (auto it = std::next(shortcuts.cbegin()); it != shortcuts.cend(); it = std::next(it)) {
                    subText += u", " + it->toString(QKeySequence::NativeText);
                }
                match.setSubtext(subText);
            }

            match.setIconName(iconName);
            match.setData(QStringList{componentUniqueName, actionIdx.data(BaseModel::ActionRole).toString()});
            context.addMatch(match);
        }
    }
}

void KeysRunner::run(const KRunner::RunnerContext & /*context*/, const KRunner::QueryMatch &match)
{
    const QStringList data = match.data().toStringList();
    Q_ASSERT(data.length() == 2);
    QString dbusPath = data[0];
    // DBus path can only contain ASCII characters, any non-alphanumeric char should
    // be turned into '_'
    dbusPath.replace(u'.', u'_');
    QDBusMessage msg =
        QDBusMessage::createMethodCall(u"org.kde.kglobalaccel"_s, u"/component/" + dbusPath, u"org.kde.kglobalaccel.Component"_s, u"invokeShortcut"_s);
    msg << data[1];
    QDBusConnection::sessionBus().asyncCall(msg);
}

void KeysRunner::onShortcutsChanged(const QStringList &, const QList<int> &)
{
    m_needsUpdate = true;
}

void KeysRunner::onPrepare()
{
    if (!m_needsUpdate) {
        return;
    }
    connect(
        m_globalAccelModel,
        &QAbstractItemModel::modelReset,
        this,
        [this] {
            m_needsUpdate = false;
        },
        Qt::SingleShotConnection);
    m_globalAccelModel->load();
}

K_PLUGIN_CLASS_WITH_JSON(KeysRunner, "plasma-runner-keys.json")

#include "keysrunner.moc"
#include "moc_keysrunner.cpp"
