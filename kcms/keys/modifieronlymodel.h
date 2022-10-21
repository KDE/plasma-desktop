#ifndef MODIFIERONLYMODEL_H
#define MODIFIERONLYMODEL_H

#include <QIdentityProxyModel>

#include "basemodel.h"
#include "modifieronlyshortcuts.h"

#include <KModelIndexProxyMapper>

#include <memory>

class ModifierOnlyModel : public QIdentityProxyModel
{
    Q_OBJECT
public:
    enum Roles { SupportsModifierOnlyShortcuts = Qt::UserRole + 100, ModifierOnlyShortcuts };

    ModifierOnlyModel(QAbstractItemModel *toDecorate);

    QHash<int, QByteArray> roleNames() const override;
    void setSourceModel(QAbstractItemModel *sourceModel) override;
    QVariant data(const QModelIndex &proxyIndex, int role = Qt::DisplayRole) const override;

private:
    QAbstractItemModel *m_model;
    KModelIndexProxyMapper m_mapper;
    ModifierOnlyShortcutsSettings m_modifieronlyShortcuts;
    QMap<std::pair<QString, QString>, Qt::KeyboardModifiers> m_modiferOnlyShortcutsMap;
}

#endif
