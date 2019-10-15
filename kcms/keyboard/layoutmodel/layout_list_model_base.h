#ifndef LAYOUTLISTMODELBASE_H
#define LAYOUTLISTMODELBASE_H

#include <QByteArray>
#include <QHash>
#include <QObject>

class LayoutListModelBase {
public:
    virtual ~LayoutListModelBase() = default;

    struct Roles {
        enum {
            NameRole = Qt::UserRole + 1,
            DescriptionRole,
            LanguagesRole,
            EnabledRole,
            IsConfigurableRole,
            SourceRole,
            ConfigModelRole,
            PriorityRole,
            SaveNameRole,
            IsLatinModeEnabledRole,
            LatinModeLayoutRole,
            ShortNameRole,
        };
    };

    QHash<int, QByteArray> roleNames() const;
    int role(QString const& roleName) const;
};

#endif // LAYOUTLISTMODELBASE_H
