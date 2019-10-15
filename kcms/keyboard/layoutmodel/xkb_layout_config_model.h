#ifndef XKB_LAYOUT_CONFIG_MODEL_H
#define XKB_LAYOUT_CONFIG_MODEL_H

#include <QAbstractItemModel>

class XkbRules;

class XkbLayoutConfigModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    XkbLayoutConfigModel(XkbRules* rules, QObject* parent);

    QModelIndex index(int row, int column, const QModelIndex &parent) const override;
    QModelIndex parent(const QModelIndex &child) const override;
    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;

    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void save() const;

private:
    XkbRules* m_rules;

    struct Roles {
        enum {
            DescriptionRole = Qt::UserRole + 1,
        };
    };
};

Q_DECLARE_METATYPE(XkbLayoutConfigModel*)

#endif // XKB_LAYOUT_CONFIG_MODEL_H
