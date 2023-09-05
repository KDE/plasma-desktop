/*
    SPDX-FileCopyrightText: 2023 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

/**
    The clipboard widget now uses a string to store the last used barcode type to adapt to
    the enum change in Prison library.

    Before:
    Null = 0,
    QRCode,
    DataMatrix,
    Aztec,
    Code39,
    Code93,
    Code128,
    PDF417,
    EAN13,

    After:
    QRCode = 0,
    DataMatrix,
    Aztec,
    Code39,
    Code93,
    Code128,
    PDF417,
    EAN13,

    @see https://invent.kde.org/plasma/plasma-workspace/-/merge_requests/3193
    @see https://invent.kde.org/frameworks/prison/-/commit/7f511c9db2e88f7e15a2ed45c4763aee4822a6cc
    @since 6.0
*/

const barcodeTypeList = [
    "QRCode", // Old "Null"
    "QRCode",
    "DataMatrix",
    "Aztec",
    "Code39",
    "Code93",
    "Code128"
]

const containments = desktops().concat(panels());
containments.forEach(containment => containment.widgets("org.kde.plasma.clipboard").forEach(widget => {
    widget.currentConfigGroup = ["General"];
    let barcodeTypeIndex = widget.readConfig("barcodeType", 1 /* Default */);
    if (barcodeTypeIndex < 0 || barcodeTypeIndex >= barcodeTypeList.length) {
        barcodeTypeList = 1; // Reset abnormal index
    }
    if (barcodeTypeIndex !== 1 /* Changed by the user */) {
        widget.writeConfig("barcodeType", barcodeTypeList[barcodeTypeIndex]);
    }
}));
