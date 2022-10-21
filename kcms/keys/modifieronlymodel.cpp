#include "modifieronlymodel.h"

ModifierOnlyShortcutsSettings m_modifieronlyShortcuts;
QMap<std::pair<QString, QString>, Qt::KeyboardModifiers> m_modiferOnlyShortcutsMap;

ModifierOnlyModel::ModifierOnlyModel(QAbstractItemModel *toDecorate)
    : m_model(toDecorate)
    , m_mapper(this, m_model)
{
}

QHash<int, QByteArray> ModifierOnlyModel::roleNames() const
{
    auto roles = QIdentityProxyModel::roleNames();
    roles.insert(SupportsModifierOnlyShortcuts, "supportsModifierOnlyShortcuts");
    roles.insert(ModifierOnlyShortcuts, "modifierOnlyShortcuts");
    return roles;
}

void ModifierOnlyModel::setSourceModel(QAbstractItemModel *sourceModel)
{
}

QVariant ModifierOnlyModel::data(const QModelIndex &proxyIndex, int role) const
{
    if (m_mapper.isConnected() && (role == SupportsModifierOnlyShortcuts || role == ModifierOnlyShortcuts)) {
        auto index = m_mapper.mapLeftToRight(proxyIndex);
        if (index.model() == m_model) {
            if (role == SupportsModifierOnlyShortcuts) {
                return true;
            } else if (role == ModifierOnlyShortcuts) {
            }
        }
    }
    return QIdentityProxyModel::data(proxyIndex, role);
}
