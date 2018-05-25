import QtQuick 2.0
import QtQuick.Controls 1.2 as QtControls
import QtQuick.Layouts 1.0
import QtQuick.Dialogs 1.1

import org.kde.plasma.private.digitalclock 1.0

ColumnLayout {
    QtControls.TextField {
            id: filter
            Layout.fillWidth: true
            placeholderText: i18n("Search Time Zones")
        }

        QtControls.TableView {
            id: timeZoneView

            signal toggleCurrent

            Layout.fillWidth: true
            Layout.fillHeight: true

            Keys.onSpacePressed: toggleCurrent()

            TimeZoneModel {
                id: timeZones

                onSelectedTimeZonesChanged: {
                    if (selectedTimeZones.length == 0) {
                        messageWidget.visible = true;

                        timeZones.selectLocalTimeZone();
                    }
                }
            }

            model: TimeZoneFilterProxy {
                sourceModel: timeZones
                filterString: filter.text
            }

            QtControls.TableViewColumn {
                role: "city"
                title: i18n("City")
            }
            QtControls.TableViewColumn {
                role: "region"
                title: i18n("Region")
            }
            QtControls.TableViewColumn {
                role: "comment"
                title: i18n("Comment")
            }
        }
}
