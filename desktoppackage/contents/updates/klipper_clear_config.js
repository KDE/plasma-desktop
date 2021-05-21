/*
    Previously, Klipper "clear history" dialog used to not ask again even if user answered No.
    This was changed so that only a Yes answer will lead to no more asking, by using warningContinueCancel instead of questionYesNo.
    Now the behaviour of previous config value really_clear_history has inverted: true/undefined => ask again; false => don't ask.
    This update script migrates old configs to use the new config value, renamed to klipperClearHistoryAskAgain.
*/

config = ConfigFile("plasmashellrc", "Notification Messages");
oldVal = config.readEntry("really_clear_history");
if (oldVal === "true") {
    // Clear and don't ask again -- preserve this choice
    config.writeEntry("klipperClearHistoryAskAgain", false)
}
config.deleteEntry("really_clear_history");
