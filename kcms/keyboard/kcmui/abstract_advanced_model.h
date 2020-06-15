#ifndef ABSTRACT_ADVANCED_MODEL_H
#define ABSTRACT_ADVANCED_MODEL_H

#include <QHash>

class AbstractAdvancedModel
{
public:
    virtual ~AbstractAdvancedModel() = default;

    QHash<int, QByteArray> roleNames() const;

    struct Roles {
        enum {
            DescriptionRole = Qt::DisplayRole,
            NameRole = Qt::UserRole + 1,
            SectionNameRole,
            SectionDescriptionRole,
            ExclusiveRole,
            SelectedRole,
            IsGroupRole,
        };
    };
};

#endif // ABSTRACT_ADVANCED_MODEL_H
