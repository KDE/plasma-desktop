#include <ddcutil_c_api.h>
#include <ddcutil_status_codes.h>

#include "displaymodel.h"

class DDCDisplay
{
public:
    DDCA_Display_Ref ref;
    QString manufacturer;
    QString name;
};

DisplayModel::DisplayModel(QObject *parent)
    : QAbstractListModel(parent)
{
    DDCA_Display_Info_List *list = nullptr;
    ddca_get_display_info_list2(true, &list);
    for (int i = 0; i < list->ct; i++) {
        auto display = std::make_unique<DDCDisplay>();
        display->ref = list->info[i].dref;
        display->manufacturer = QString::fromUtf8(list->info[i].mfg_id);
        display->name = QString::fromUtf8(list->info[i].model_name);
        m_displays.emplace_back(display);
    }
    ddca_free_display_info_list(list);
}

int DisplayModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_displays.size();
}
QVariant DisplayModel::data(const QModelIndex &index, int role) const
{
    int row = index.row();
    if (row < 0 || row >= int(m_displays.size())) {
        return QVariant();
    }

    switch (role) {
    case Name:
        return m_displays[row]->name;
        break;
    case Manufacturer:
        return m_displays[row]->manufacturer;
        break;
    case DDCRef:
        return QVariant::fromValue(m_displays[row]->ref);
    }
    Q_UNREACHABLE();
    return {};
}
QHash<int, QByteArray> DisplayModel::roleNames() const
{
    return {{Name, "name"}, {Manufacturer, "manufacturer"}, {DDCRef, "ref"}};
}
