/*
    SPDX-FileCopyrightText: 2025 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "keysrunner.h"

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusMetaType>

#include <KLocalizedString>

#include "filteredmodel.h"
#include "globalaccelmodel.h"

using namespace Qt::StringLiterals;

QDBusArgument &operator<<(QDBusArgument &argument, const QKeySequence &sequence)
{
    argument.beginStructure();
    argument.beginArray(qMetaTypeId<int>());
    for (int i = 0; i < 4; i++) {
        argument << (i < sequence.count() ? sequence[i].toCombined() : 0);
    }
    argument.endArray();
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, QKeySequence &sequence)
{
    int s1;
    int s2;
    int s3;
    int s4;
    argument.beginStructure();
    argument.beginArray();
    argument >> s1 >> s2 >> s3 >> s4;
    sequence = QKeySequence(s1, s2, s3, s4);
    argument.endArray();
    argument.endStructure();
    return argument;
}

KeysRunner::KeysRunner(QObject *parent, const KPluginMetaData &metaData)
    : KRunner::AbstractRunner(parent, metaData)
    , m_isKRunner(KLocalizedString::applicationDomain() == QByteArrayView("krunner"))
    , m_isPlasmashell(KLocalizedString::applicationDomain() == QByteArrayView("plasmashell"))
{
    addSyntax(u":q:"_s, i18nc("@info:usagetip", "Finds shortcuts with names or descriptions matching :q: or gets the name of a shortcut"));
}

void KeysRunner::init()
{
    m_globalAccelModel = new GlobalAccelModel(this);
    m_filteredModel = new FilteredShortcutsModel(this);
    m_filteredModel->setShowsLaunchAction(false);
    m_filteredModel->setSourceModel(m_globalAccelModel);
    if (m_globalAccelModel->isValid()) {
        m_globalAccelModel->load();
        connect(
            m_globalAccelModel,
            &QAbstractItemModel::modelReset,
            this,
            [this] {
                qDBusRegisterMetaType<QKeySequence>();
                qDBusRegisterMetaType<QList<QKeySequence>>();
                QDBusConnection::sessionBus().connect(u"org.kde.kglobalaccel"_s,
                                                      u"/kglobalaccel"_s,
                                                      u"org.kde.KGlobalAccel"_s,
                                                      u"yourShortcutsChanged"_s,
                                                      this,
                                                      SLOT(onShortcutsChanged(QStringList, QList<QKeySequence>)));
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
        if (m_isKRunner && componentUniqueName == u"org.kde.krunner.desktop") {
            continue;
        } else if (m_isPlasmashell && componentUniqueName == u"plasmashell") {
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
            match.setText(i18nc("@label action - app", "%1 - %2", actionName, componentFriendlyName));

            auto shortcuts = actionIdx.data(BaseModel::ActiveShortcutsRole).value<QSet<QKeySequence>>();
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
    QDBusConnection::sessionBus().call(msg, QDBus::NoBlock);
}

void KeysRunner::onShortcutsChanged(const QStringList &, const QList<QKeySequence> &)
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
