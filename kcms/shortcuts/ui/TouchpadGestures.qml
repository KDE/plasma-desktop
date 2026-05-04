
import QtQml
import QtQml.Models


KCMUtils.SimpleKCM {

    ListView {
        id: components
        clip: true
        model: kcm.baseModel


        delegate: QQC2.ItemDelegate {
            id: componentDelegate

            text: model.display
            icon.name: model.decoration
        }
    }

}
