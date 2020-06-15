#include "abstract_advanced_model.h"
#include <QDebug>

QHash<int, QByteArray> AbstractAdvancedModel::roleNames() const
{
    return {
        { Roles::DescriptionRole, "description" },
        { Roles::NameRole, "name" },
        { Roles::SectionNameRole, "sectionName" },
        { Roles::SectionDescriptionRole, "sectionDescription" },
        { Roles::SelectedRole, "selected" },
        { Roles::ExclusiveRole, "exclusive" },
        { Roles::IsGroupRole, "isGroup" },
    };
}
