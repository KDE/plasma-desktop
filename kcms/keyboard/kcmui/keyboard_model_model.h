#ifndef KCM_KEYBOARD_KEYBOARDMODELMODEL_H
#define KCM_KEYBOARD_KEYBOARDMODELMODEL_H

#include <QAbstractListModel>
#include <QVector>

#include "../xkb_rules.h"

class KeyboardModelModel : public QAbstractListModel {
    Q_OBJECT

public:
    explicit KeyboardModelModel(XkbRules const& rules, QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;

    QVariant data(const QModelIndex& index, int role) const override;

    Qt::ItemFlags flags(const QModelIndex& index) const override;

    struct Roles {
        enum {
            DescriptionRole = Qt::UserRole + 1,
            NameRole,
        };
    };

    QHash<int, QByteArray> roleNames() const override;

public:
    int findIndexByName(QString const& name) const;

private:
    struct KeyboardModel {
        QString vendor;
        QString name;
        QString description;

        QString display_text() const;
    };
    QVector<KeyboardModel> m_list;
};

#endif // KCM_KEYBOARD_KEYBOARDMODELMODEL_H
