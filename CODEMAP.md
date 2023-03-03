## Code Map

### `applets/`

Various desktop-specific applets. Look inside the specific applet directories for more info.

### `emojier/`

The Emoji Selector, bound to Meta+dot by default.

### `kcms/`

KCMs are KDE Config Modules, which make up System Settings. Desktop-specific ones can be found here.

### `runners/`

Runners are plugins for KRunner, the launcher system used throughout Plasma. Only two runners are provided here:

- `plasma-desktop`, which provides access to the Plasma and KWin scripting consoles.
- `kwin`, which provides access to the KWin debug console.

Most of the interesting runners are in plasma-workspace and other component repos.

### `toolboxes/`

UI for the "Edit Mode" in Plasma. See also the `ConfigOverlay.qml` in `containments/` for the UI elements for editing individual applets.

### `containments/`

The panel, desktop, and folder view containments.

### `desktoppackage/contents/updates/`

Update scripts used to ensure that Plasma updates go smoothly, such as migrating settings whenever the configuration format changes.

### `desktoppackage/contents/configuration/`

UI for the config and about dialogs and popups of applets and containments.

### `desktoppackage/contents/applet/`

UI shells for the applet, like the applet loading error page and the CompactRepresentation.

### `desktoppackage/contents/explorer/`

UI related to adding and changing applets — the add widgets sidebar and the applet alternatives menu.

### `desktoppackage/contents/views`

UI for the panel and desktop.

### `layouts-templates/`

Templates for the items in the "Add Panel…" menu on the desktop: default panel, empty panel, and global menubar.

### `solid-device-automounter/`

Provides the KCM, library, and KDE Daemon (kded) plugin for handling and configuring the automatic mounting of any removable devices plugged in.

### `kaccess/`

Accessibility daemon.

### `knetattach/`

Wizard to add a network folder.

### `imports/activitymanager/`

Provides the `org.kde.plasma.activityswitcher` QML import. Used in various applets to interact with and handle Activities.

### `attica-kde/`

Attica is the KDE Framework for interacting with Open Collaboration Services, used in KDE for the Get New Stuff system. The part here is the plugin for integrating the OCS accounts system into Plasma.