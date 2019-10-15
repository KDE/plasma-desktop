#include "layout_list_model_base.h"

#include <algorithm>
#include <QAbstractProxyModel>
#include <KConcatenateRowsProxyModel>
#include <QDebug>

QHash<int, QByteArray> LayoutListModelBase::roleNames() const
{
    return {
        { Roles::NameRole, "name" },
        { Roles::DescriptionRole, "description" },
        { Roles::LanguagesRole, "languages" },
        { Roles::EnabledRole, "enabled" },
        { Roles::IsConfigurableRole, "is_configurable" },
        { Roles::ConfigModelRole, "config_model" },
        { Roles::SourceRole, "source" },
        { Roles::PriorityRole, "priority" },
        { Roles::SaveNameRole, "save_name" },
        { Roles::IsLatinModeEnabledRole, "is_latin_mode_enabled" },
        { Roles::LatinModeLayoutRole, "latin_mode_layout" },
    };
}

int LayoutListModelBase::role(QString const& roleName) const
{
    auto roles = roleNames();
    for (auto it = roles.keyValueBegin(); it != roles.keyValueEnd(); ++it) {
        if ((*it).second == roleName) {
            return (*it).first;
        }
    }
    return -1;
}
