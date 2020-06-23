#include "fcitx_im_config_model.h"

#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>
#include <QDBusInterface>
#include <QDebug>
#include <QFile>
#include <QMetaEnum>
#include <QSettings>

#include "../fcitx/fcitxqtinputmethodproxy.h"
#include "layout_list_model_fcitx.h"
#include "../keyboard_dbus.h"

FcitxIMListModel::FcitxIMListModel(QObject* parent)
    : LayoutListSortFilterProxyModel(parent)
{
    setSourceModel(new LayoutListModelFcitx(this));
    setSortRole(Roles::DescriptionRole);
    sort(0);
}

QHash<int, QByteArray> FcitxIMListModel::roleNames() const
{
    return LayoutListModelBase::roleNames();
}

bool FcitxIMListModel::filterAcceptsRow(int source_row, const QModelIndex &) const
{
    QString name = sourceModel()->data(sourceModel()->index(source_row, 0), Roles::NameRole).toString();
    return name.startsWith("fcitx-keyboard-");
}

FcitxIMConfigModel::FcitxIMConfigModel(QString imName, QObject* parent)
    : QAbstractListModel(parent)
    , m_imName(imName)
{
    QDBusInterface interface("org.fcitx.Fcitx", "/inputmethod", "org.fcitx.Fcitx.InputMethod");
    QDBusMessage reply = interface.call("GetIMAddon", imName);
    m_addonName = reply.arguments().first().toString();

    QString config_desc_path = QString("/usr/share/fcitx/configdesc/%1.desc").arg(m_addonName);

    // determine order of config items
    QFile config_desc_file(config_desc_path);
    config_desc_file.open(QIODevice::ReadOnly);
    if (!config_desc_file.isOpen()) {
        qWarning() << "Cannot open config description file";
        return;
    }
    QTextStream stream(&config_desc_file);
    QString line;
    QStringList order;
    while (stream.readLineInto(&line)) {
        if (line.startsWith('[')) {
            order << line;
        }
    }
    config_desc_file.close();

    // read description
    QSettings config_desc(config_desc_path, QSettings::IniFormat);
    for (QString group : config_desc.childGroups()) {
        if (group == "DescriptionFile") {
            continue;
        }
        config_desc.beginGroup(group);
        for (QString name : config_desc.childGroups()) {
            config_desc.beginGroup(name);

            ConfigItem item;
            item.group = group;
            item.name = name;
            item.description = config_desc.value("Description").toString();
            item.default_value = config_desc.value("DefaultValue").toString();

            QString type_string = config_desc.value("Type").toString();
            if (type_string == "Integer") {
                item.type = ConfigType::IntegerType;
            } else if (type_string == "Char") {
                item.type = ConfigType::CharType;
            } else if (type_string == "String") {
                item.type = ConfigType::StringType;
            } else if (type_string == "I18NString") {
                item.type = ConfigType::I18NStringType;
            } else if (type_string == "Boolean") {
                item.type = ConfigType::BooleanType;
            } else if (type_string == "File") {
                item.type = ConfigType::FileType;
            } else if (type_string == "Font") {
                item.type = ConfigType::FontType;
            } else if (type_string == "Hotkey") {
                item.type = ConfigType::HotkeyType;
            } else if (type_string == "Enum") {
                item.type = ConfigType::EnumType;
                int num = config_desc.value("EnumCount").toInt();
                QStringList list;
                for (int i = 0; i < num; ++i) {
                    list << config_desc.value(QString("Enum%1").arg(i)).toString();
                }
                item.data = list;
            }

            m_configs << item;

            config_desc.endGroup();
        }
        config_desc.endGroup();
    }

    std::sort(m_configs.begin(), m_configs.end(), [&](ConfigItem const& l, ConfigItem const& r) {
        int left = order.indexOf(QString("[%1/%2]").arg(l.group, l.name));
        int right = order.indexOf(QString("[%1/%2]").arg(r.group, r.name));
        Q_ASSERT(left != -1 && right != -1);
        return left < right;
    });

    // read config values
    auto config_path = QString("fcitx/conf/%1.config").arg(m_addonName);
    KSharedConfigPtr config = KSharedConfig::openConfig(config_path, KConfig::SimpleConfig);

    for (auto& item : m_configs) {
        KConfigGroup group(config, item.group);

        switch (item.type) {
        case ConfigType::IntegerType:
            item.current_value = group.readEntry<int>(item.name, item.default_value.toInt());
            break;
        case ConfigType::BooleanType:
            item.current_value = group.readEntry<bool>(item.name, item.default_value.toBool());
            break;
        case ConfigType::EnumType:
            item.current_value = group.readEntry<QString>(item.name, item.default_value.toString());
            break;
        case ConfigType::CharType:
        case ConfigType::StringType:
        case ConfigType::HotkeyType:
            item.current_value = group.readEntry<QString>(item.name, item.default_value.toString());
            break;
        }
    }

    if (isRealIM()) {
        m_latinModeLayoutList = new FcitxIMListModel(this);

        KConfigGroup kxkbrc(
            KSharedConfig::openConfig(QStringLiteral("kxkbrc"), KConfig::NoGlobals),
            QStringLiteral("Layout"));

        setIsLatinSwitchingEnabled(kxkbrc.readEntry<bool>(QString("Fcitx%1LatinFallBackEnabled").arg(m_imName), false));
        setSelectedLatinLayoutId(kxkbrc.readEntry<QString>(QString("Fcitx%1LatinFallBack").arg(m_imName), QString("us")));
    }
}

int FcitxIMConfigModel::rowCount(const QModelIndex& parent) const
{
    // For list models only the root node (an invalid parent) should return the list's size. For all
    // other (valid) parents, rowCount() should return 0 so that it does not become a tree model.
    if (parent.isValid())
        return 0;

    return m_configs.size();
}

QVariant FcitxIMConfigModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    auto const& item = m_configs[index.row()];
    switch (role) {
    case Roles::NameRole:
        return item.name;
    case Roles::GroupRole:
        return item.group;
    case Roles::TypeRole:
        return QMetaEnum::fromType<ConfigType>().valueToKey(item.type);
    case Roles::DescriptionRole:
        return item.description;
    case Roles::DataRole:
        return item.data;
    case Roles::CurrentValueRole:
        return item.current_value;
    }

    return QVariant();
}

bool FcitxIMConfigModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    qDebug() << "setData";
    if (role == Roles::CurrentValueRole) {
        m_configs[index.row()].current_value = value;
    }
    return false;
}

QHash<int, QByteArray> FcitxIMConfigModel::roleNames() const
{
    return {
        { Roles::NameRole, "name" },
        { Roles::GroupRole, "group" },
        { Roles::TypeRole, "type" },
        { Roles::DescriptionRole, "description" },
        { Roles::DataRole, "data" },
        { Roles::CurrentValueRole, "current_value" }
    };
}

void FcitxIMConfigModel::save() const
{
    qDebug() << "save";
    if (isRealIM()) {
        KConfigGroup kxkbrc(
            KSharedConfig::openConfig(QStringLiteral("kxkbrc"), KConfig::NoGlobals),
            QStringLiteral("Layout"));
        kxkbrc.writeEntry<bool>(QString("Fcitx%1LatinFallBackEnabled").arg(m_imName), isLatinSwitchingEnabled());
        kxkbrc.writeEntry<QString>(QString("Fcitx%1LatinFallBack").arg(m_imName), m_selectedLatinLayoutId);
        kxkbrc.sync();
    }

    auto config_path = QString("fcitx/conf/%1.config").arg(m_addonName);
    KSharedConfigPtr config = KSharedConfig::openConfig(config_path, KConfig::SimpleConfig);

    for (auto const& item : m_configs) {
        KConfigGroup group(config, item.group);

        switch (item.type) {
        case ConfigType::IntegerType:
            group.writeEntry<int>(item.name, item.current_value.toInt());
            break;
        case ConfigType::BooleanType:
            group.writeEntry<bool>(item.name, item.current_value.toBool());
            break;
        case ConfigType::EnumType:
            group.writeEntry<QString>(item.name, item.current_value.toString());
            break;
        case ConfigType::CharType:
        case ConfigType::StringType:
        case ConfigType::HotkeyType:
            group.writeEntry<QString>(item.name, item.current_value.toString());
            break;
        }

        group.sync();
    }

    // dbus call to the kded (in X11) / kwin (in Wayland) to apply the config changes
    QDBusMessage message = QDBusMessage::createSignal(
        KEYBOARD_DBUS_OBJECT_PATH,
        KEYBOARD_DBUS_SERVICE_NAME,
        KEYBOARD_DBUS_CONFIG_RELOAD_MESSAGE);
    QDBusConnection::sessionBus().send(message);
}

FcitxIMListModel* FcitxIMConfigModel::latinModeLayoutList() const
{
    return m_latinModeLayoutList;
}

bool FcitxIMConfigModel::isLatinSwitchingEnabled() const
{
    return m_isLatinSwitchingEnabled;
}

void FcitxIMConfigModel::setIsLatinSwitchingEnabled(bool isLatinSwitchingEnabled)
{
    if (m_isLatinSwitchingEnabled != isLatinSwitchingEnabled) {
        m_isLatinSwitchingEnabled = isLatinSwitchingEnabled;
    }
}

int FcitxIMConfigModel::selectedLatinLayoutIndex() const
{
    for (int row = 0; row < m_latinModeLayoutList->rowCount(); ++row) {
        if (m_latinModeLayoutList->data(m_latinModeLayoutList->index(row, 0), Roles::NameRole) == m_selectedLatinLayoutId) {
            return row;
        }
    }
    return -1;
}

void FcitxIMConfigModel::setSelectedLatinLayoutIndex(int index)
{
    if (selectedLatinLayoutIndex() != index) {
        m_selectedLatinLayoutId = m_latinModeLayoutList->data(m_latinModeLayoutList->index(index, 0), Roles::NameRole).toString();
        emit selectedLatinLayoutIndexChanged();
    }
}

QString FcitxIMConfigModel::selectedLatinLayoutId() const
{
    return m_selectedLatinLayoutId;
}

void FcitxIMConfigModel::setSelectedLatinLayoutId(const QString &selectedLatinLayoutId)
{
    if (m_selectedLatinLayoutId != selectedLatinLayoutId) {
        m_selectedLatinLayoutId = selectedLatinLayoutId;
        emit selectedLatinLayoutIndexChanged();
    }
}

bool FcitxIMConfigModel::isRealIM() const
{
    return !m_imName.startsWith("fcitx-keyboard-");
}
