#!/usr/bin/env bash

appletsrcpath=`qtpaths --locate-file GenericConfigLocation plasma-org.kde.plasma.desktop-appletsrc`
kactivitiesrcpath=`qtpaths --locate-file GenericConfigLocation kactivitymanagerd-statsrc`

affectedFiles=( $appletsrcpath $kactivitiesrcpath )
renamedFilesOrig=( "componentchooser.desktop" "clock.desktop" "desktoppath.desktop" "joystick.desktop" "kcmkded.desktop" "mouse.desktop" "qtquicksettings.desktop" "solid-actions.desktop" "device_automounter_kcm.desktop" )
renamedFilesNew=( "kcm_componentchooser.desktop" "kcm_clock.desktop" "kcm_desktoppaths.desktop" "kcm_joystick.desktop" "kcm_kded.desktop" "kcm_mouse.desktop" "kcm_qtquicksettings.desktop" "solid-kcm_solid_actions.desktop" "kcm_device_automounter.desktop" )

for affectedFile in "${affectedFiles[@]}"
do
    if [ -f ${affectedFile} ]; then
        for i in "${!renamedFilesOrig[@]}"; do
            origFile=${renamedFilesOrig[$i]}
            newFile=${renamedFilesNew[$i]}
            # KCMs that are pinned by their file path
            sed -i "s/\/kservices5\/$origFile/\/applications\/$newFile/" $affectedFile
            # KCMs that are pinned by the storage id
            sed -i "s/:$origFile/:$newFile/" $affectedFile
        done
    fi
done
