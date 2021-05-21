/*
    SPDX-FileCopyrightText: 2011 Martin Gräßlin <mgraesslin@kde.org>
    SPDX-FileCopyrightText: 2012 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2015 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
import QtQuick 2.0
import org.kde.plasma.private.kicker 0.1 as Kicker

BaseView {
    objectName: "ComputerView"

    model: Kicker.ComputerModel {
        id: computerModel

        appletInterface: plasmoid

        favoritesModel: globalFavorites

        Component.onCompleted: {
            systemApplications = plasmoid.configuration.systemApplications;
        }
    }

    Connections {
        target: computerModel

        function onSystemApplicationsChanged() {
            plasmoid.configuration.systemApplications = target.systemApplications;
        }
    }

    Connections {
        target: plasmoid.configuration

        function onSystemApplicationsChanged() {
            computerModel.systemApplications = plasmoid.configuration.systemApplications;
        }
    }
}
