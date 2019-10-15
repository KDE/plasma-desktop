#include "abstract_advanced_model.h"

QHash<int, QByteArray> AbstractAdvancedModel::roleNames() const
{
    return {
        { Roles::DescriptionRole, "description" },
        { Roles::NameRole, "name" },
        { Roles::SectionNameRole, "section_name" },
        { Roles::SelectedRole, "selected" },
        { Roles::ExclusiveRole, "exclusive" },
        { Roles::IsGroupRole, "is_group" },
        { Roles::SectionNamePlusIsGroupRole, "section_name_plus_is_group" }
    };
}
