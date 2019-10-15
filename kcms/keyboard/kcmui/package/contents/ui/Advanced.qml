import QtQuick 2.9
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.3 as Controls
import org.kde.kirigami 2.5 as Kirigami
import org.kde.plasma.core 2.1 as PlasmaCore
import org.kde.kcm 1.2 as KCM
import org.kde.kquickcontrols 2.0 as KQuickControls

KCM.ScrollViewKCM {
    id: root

    property var dataModel;
    signal changed();

    property var openedGroups: [];

    view: ListView {
        model: PlasmaCore.SortFilterModel {
            id: sortFilterModel
            sourceModel: dataModel
            filterRole: "section_name_plus_is_group"
            filterCallback: function(idx, item) {
                var sectionName = item.split('+')[0];
                var isGroup = item.split('+')[1] === "true";
                return isGroup || root.openedGroups.indexOf(sectionName) >= 0;
            }
        }

        delegate: Kirigami.BasicListItem {
            id: item
            reserveSpaceForIcon: !model.is_group
            separatorVisible: true

            Controls.CheckBox {
                id: checkbox

                Binding on checked {
                    when: !model.is_group
                    value: model.selected
                }

                checked: model.is_group && root.openedGroups.indexOf(model.section_name) >= 0

                onClicked: {
                    if (model.is_group) {
                        if (checked) {
                            root.openedGroups.push(model.section_name);
                            sortFilterModel.invalidate();
                        }
                        else {
                            root.openedGroups.splice(openedGroups.indexOf(model.section_name), 1);
                            sortFilterModel.invalidate();
                        }
                    }
                    else {
                        model.selected = checked;
                        root.changed();
                    }
                }
            }

            Text {
                text: model.description
            }
        }
    }
}
