#include "keyboard_model_model.h"

#include <KLocalizedString>
#include <QErrorMessage>
#include <QDebug>

#include <algorithm>

KeyboardModelModel::KeyboardModelModel(XkbRules const& rules, QObject* parent)
    : QAbstractListModel(parent)
{
    m_list.reserve(rules.modelInfos.count());
    for (auto model_info : rules.modelInfos) {
        m_list.push_back({model_info->vendor, model_info->name, model_info->description});
    }
    std::sort(m_list.begin(), m_list.end(), [](KeyboardModel const& a, KeyboardModel const& b) {
        return a.display_text() < b.display_text();
    });
}

int KeyboardModelModel::rowCount(const QModelIndex& /* parent */) const
{
    return m_list.size();
}

QVariant KeyboardModelModel::data(const QModelIndex& index, int role) const
{
    auto model = m_list.at(index.row());
    if (role == Roles::DescriptionRole) {
        return model.display_text();
    }
    if (role == Roles::NameRole) {
        return model.name;
    }
    return QVariant();
}

Qt::ItemFlags KeyboardModelModel::flags(const QModelIndex& /* index */) const
{
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QHash<int, QByteArray> KeyboardModelModel::roleNames() const
{
    return { { Roles::DescriptionRole, "description" }, { Roles::NameRole, "name" } };
}

int KeyboardModelModel::findIndexByName(const QString &name) const
{
    int index;
    for (index = 0; index < m_list.size(); ++index) {
        if (m_list[index].name == name)
            break;
    }
    if (index == m_list.size()) {
        qWarning() << i18nc("keyboard model", "Keyboard model \"%1\" not detected", name);
        return -1;
    }
    return index;
}

QString KeyboardModelModel::KeyboardModel::display_text() const
{
    return i18nc("vendor | keyboard model", "%1 | %2", vendor, description);
}
