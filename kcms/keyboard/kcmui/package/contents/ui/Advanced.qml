import QtQuick 2.9
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.3 as Controls
import org.kde.kirigami 2.13 as Kirigami
import org.kde.plasma.core 2.1 as PlasmaCore
import org.kde.kcm 1.2 as KCM
import org.kde.kquickcontrols 2.0 as KQuickControls

KCM.ScrollViewKCM {
    id: root

    property var dataModel;
    signal changed();
    
    header: Kirigami.SearchField {
        id: filterText
    }

    view: ListView {
        id: advancedRuleList
        property var _buttonGroups: []
        
        model: PlasmaCore.SortFilterModel {
            id: nameFilterModel
            sourceModel: dataModel
            filterRole: "description"
            filterCaseSensitivity: Qt.CaseInsensitive
            filterString: filterText.text
        }
        
        section.property: "sectionDescription"
        section.delegate: Kirigami.ListSectionHeader {
            label: section
        }

        delegate: Kirigami.BasicListItem {
            id: item
            separatorVisible: true

            Controls.CheckBox {
                id: checkbox
                visible: !model.exclusive
                checked: model.selected
                onToggled: model.selected = !model.selected
            }
            
            Controls.RadioButton {
                visible: model.exclusive
                checked: model.selected
                Controls.ButtonGroup.group : model.exclusive ? advancedRuleList.findButtonGroup(model.sectionName) : null
                onToggled: model.selected = !model.selected
            }

            label: model.description
        }
        
        function findButtonGroup(name) {
            for (let item of advancedRuleList._buttonGroups) {
                if (item.name == name) {
                    return item.group;
                }
            }

            let group = Qt.createQmlObject(
                'import QtQuick 2.5;' +
                'import QtQuick.Controls 2.5;' +
                'ButtonGroup {}',
                advancedRuleList,
                "dynamicButtonGroup" + advancedRuleList._buttonGroups.length
            );

            advancedRuleList._buttonGroups.push({ name, group });

            return group;
        }
    }
}
