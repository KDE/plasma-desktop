/*
    SPDX-FileCopyrightText: 2019 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "key.h"

#include <QDebug>
#include <QKeyEvent>
#include <QMetaEnum>

#include <X11/keysym.h>
#include <xkbcommon/xkbcommon.h>

#include "application.h"
#include "shape.h"

static QString xkbKeysymToName(xkb_keysym_t keysym)
{
    QVarLengthArray<char, 32> chars(32);
    Q_ASSERT(chars.size() >= 0); // ensure cast to size_t

    const int size = xkb_keysym_get_name(keysym, chars.data(), static_cast<size_t>(chars.size()));
    if (Q_UNLIKELY(size > chars.size())) {
        chars.resize(size);
        xkb_keysym_get_name(keysym, chars.data(), static_cast<size_t>(chars.size()));
    }

    return QString::fromUtf8(chars.constData(), size);
}

static QString xkbKeysymToUtf8(xkb_keysym_t keysym)
{
    QVarLengthArray<char, 32> chars(32);
    Q_ASSERT(chars.size() >= 0); // ensure cast to size_t

    const int size = xkb_keysym_to_utf8(keysym, chars.data(), static_cast<size_t>(chars.size()));
    if (Q_UNLIKELY(size > chars.size())) {
        chars.resize(size);
        xkb_keysym_to_utf8(keysym, chars.data(), static_cast<size_t>(chars.size()));
    }

    return QString::fromUtf8(chars.constData(), size);
}

static QString keySymToString(KeySym keysym)
{
    // Strangely enough xkbcommons's UTF map is incomplete with regards to
    // dead keys. Extend it a bit.
    static QHash<unsigned long, char16_t> deadMap{
        {XK_dead_grave, 0x0060},
        {XK_dead_acute, 0x00b4},
        {XK_dead_circumflex, 0x02c6},
        {XK_dead_tilde, 0x02dc},
        {XK_dead_macron, 0x00af},
        {XK_dead_breve, 0x02D8},
        {XK_dead_abovedot, 0x02D9},
        {XK_dead_diaeresis, 0x00A8},
        {XK_dead_abovering, 0x02DA},
        {XK_dead_doubleacute, 0x02DD},
        {XK_dead_caron, 0x02C7},
        {XK_dead_cedilla, 0x00B8},
        {XK_dead_ogonek, 0x02DB},
        {XK_dead_iota, 0x0269},
        {XK_dead_voiced_sound, 0x309B},
        {XK_dead_semivoiced_sound, 0x309C},
        {XK_dead_belowdot, 0x0323},
        {XK_dead_hook, 0x0309},
        {XK_dead_horn, 0x031b},
        {XK_dead_stroke, 0x0335},
        {XK_dead_abovecomma, 0x0312},
        {XK_dead_abovereversedcomma, 0x0314},
        {XK_dead_doublegrave, 0x030f},
        {XK_dead_belowring, 0x0325},
        {XK_dead_belowmacron, 0x0331},
        {XK_dead_belowcircumflex, 0x032D},
        {XK_dead_belowtilde, 0x0330},
        {XK_dead_belowbreve, 0x032e},
        {XK_dead_belowdiaeresis, 0x0324},
        {XK_dead_invertedbreve, 0x0311},
        {XK_dead_belowcomma, 0x0326},
        {XK_dead_currency, 0x00A4},
        {XK_dead_a, 0x0061},
        {XK_dead_A, 0x0041},
        {XK_dead_e, 0x0065},
        {XK_dead_E, 0x0045},
        {XK_dead_i, 0x0069},
        {XK_dead_I, 0x0049},
        {XK_dead_o, 0x006f},
        {XK_dead_O, 0x004f},
        {XK_dead_u, 0x0075},
        {XK_dead_U, 0x0055},
        {XK_dead_small_schwa, 0x0259},
        {XK_dead_capital_schwa, 0x018F},
    };

    // XKB has fairly OK unicode maps, unfortunately it is
    // overzaelous and will for example return "U+001B" for
    // Esc which is a non-printable control character and
    // also not present in most fonts. As such it is
    // worthless to use and we'll discard unicode strings that
    // contain non-printable characters (ignore null).
    // This will lead to one of the stringy name fallbacks to handle
    // these cases and produce for example 'Escape'

    if (keysym == 0 /* NoSymbol */ || keysym == XK_VoidSymbol) {
        return QString();
    }

    QString str;

    // Smartly xlib uses ulong and xkbcommon uses uint32 for syms,
    // so we'd best make sure that we can even cast the symbol before
    // tryint to do xkb mappings. Otherwise skip to fallbacks right away.
    const xkb_keysym_t xkbKeysym = static_cast<xkb_keysym_t>(keysym);
    if (static_cast<KeySym>(xkbKeysym) == keysym) {
        str = xkbKeysymToUtf8(xkbKeysym);

        const auto ucs4Str = str.toUcs4();

        if (!str.isEmpty()) {
            const QChar::Category category = QChar::category(ucs4Str[0]);
            if (category == QChar::Mark_NonSpacing || category == QChar::Mark_Enclosing) {
                str.prepend(" ");
            }
        }

        for (const auto &c : qAsConst(ucs4Str)) {
            if (!QChar::isPrint(c) && !(c == 0) && !(QChar::category(c) == QChar::Other_PrivateUse)) {
                str = "";
                break;
            }
        }

        if (str.isEmpty()) {
            str = xkbKeysymToName(xkbKeysym);
        }
    }

    if (str.isEmpty()) {
        str = XKeysymToString(keysym);
        // X11 keys can be of the form "Control_L".
        // Split them so they are easier on the eyes.
        str = str.replace('_', ' ');
    }

    if (deadMap.contains(keysym)) {
        const QChar c(deadMap[keysym]);

        // Make sure non-spacing diacritics are rendered
        if (c.category() == QChar::Mark_NonSpacing || c.category() == QChar::Mark_Enclosing) {
            // FIXME: should really be NBSP, but it doesn't seem to
            // render properly, so use SPACE which is not recommended
            // since Unicode 4.1
            str = QStringLiteral(" ") + c;
        } else {
            str = c;
        }
    }

    // X11 keys can be of the form "Control_L".
    // Split them so they are easier on the eyes.
    // But only do that on strings of 3 chars or more to not lose "_"
    if (str.size() > 2) {
        str.replace('_', ' ');
    }

    return str;
}

KeyCap::KeyCap(const QString symbols[], QObject *parent)
    : QObject(parent)
    , topLeft(symbols[Level::TopLeft])
    , topRight(symbols[Level::TopRight])
    , bottomLeft(symbols[Level::BottomLeft])
    , bottomRight(symbols[Level::BottomRight])
{
}

Key::Key(XkbKeyPtr key_, XkbDescPtr xkb_, QObject *parent)
    : XkbObject(xkb_, parent)
    , key(key_)
    , shape(new Shape(xkb->geom->shapes + key->shape_ndx, xkb, this))
    , name(key_->name.name, XkbKeyNameLength)
    , nativeScanCode(nativeScanCodeFromName(name))
    , cap(resolveCap())
    , pressed(false)
{
    qRegisterMetaType<Shape *>();

    connect(Application::instance(), &Application::keyEvent, this, [this](QKeyEvent *event) {
        Q_ASSERT(event);
        if (event->nativeScanCode() == nativeScanCode) {
            pressed = event->type() == QKeyEvent::KeyPress;
            Q_EMIT pressedChanged();
        }
    });
}

uint Key::nativeScanCodeFromName(const QByteArray &needle)
{
    for (uint keycode = xkb->min_key_code; keycode <= xkb->max_key_code; ++keycode) {
        XkbKeyNameRec key = xkb->names->keys[keycode];
        const QByteArray name(key.name, XkbKeyNameLength);
        if (name == needle) {
            return keycode;
        }
    }

    for (int i = 0; i < xkb->names->num_key_aliases; ++i) {
        XkbKeyAliasRec alias = xkb->names->key_aliases[i];
        const QByteArray name(alias.alias, XkbKeyNameLength);
        if (name == needle) {
            return nativeScanCodeFromName(alias.real);
        }
    }

    return INVALID_KEYCODE;
}

KeyCap *Key::resolveCap()
{
    // Documentation TLDR
    // - Levels are accessed by a modifier switching the keyboard to different symbols
    //   such as hitting Shift and getting access to Shift+1=!
    // - Groups are an additional system which considers the entire keyboard switched
    //   to a different symbol set. Such as the entire keyboard being Latin or Cyrillic.
    //   Within each group there may be N Levels. The keyboard therefor has N Groups with
    //   each having M levels.
    // For the purposes of key cap resolution we'll only look at the first group and the
    // first 4 levels within that group (top-left, top-right, bottom-left, bottom-right).

    const quint32 keycode = nativeScanCode;
    QString symbols[KeyCap::levelCount];

    if (keycode == INVALID_KEYCODE) {
        return new KeyCap(symbols, this);
    }

    const int group = 0;
    // We iterate over the enum directly because it also represents
    // preference. TopLeft is the preferred location for unique mapping
    // such as 'F1' that can appear in all levels but we only want it shown
    // once in the TopLeft position.
    const auto levelEnum = QMetaEnum::fromType<KeyCap::Level>();
    QVector<QString> seen;
    for (int i = 0; i < levelEnum.keyCount(); ++i) {
        int level = levelEnum.value(i);
        if (group >= XkbKeyNumGroups(xkb, keycode)) {
            continue; // group doesn't exist, shouldn't happen because we use group0
        }
        if (level >= XkbKeyGroupWidth(xkb, keycode, group)) {
            continue; // level within group doesn't exist, can totally happen!
        }

        KeySym keysym = XkbKeySymEntry(xkb, keycode, level, group);
        const auto str = keySymToString(keysym);
        if (seen.contains(str)) {
            // Don't duplicate. e.g. 'F1' can appear in all levels
            continue;
        }
        seen << str;
        symbols[level] = str;
    }

    return new KeyCap(symbols, this);
}
