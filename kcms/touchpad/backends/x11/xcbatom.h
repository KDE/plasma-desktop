/*
    SPDX-FileCopyrightText: 2013 Alexander Mezin <mezin.alexander@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef XCBATOM_H
#define XCBATOM_H

#include <xcb/xcb.h>

class XcbAtom
{
public:
    XcbAtom();
    XcbAtom(xcb_connection_t *, const char *name, bool onlyIfExists = true);
    ~XcbAtom();

    void intern(xcb_connection_t *, const char *name, bool onlyIfExists = true);
    xcb_atom_t atom();
    operator xcb_atom_t()
    {
        return atom();
    }

private:
    XcbAtom(const XcbAtom &);
    XcbAtom &operator=(const XcbAtom &);

    xcb_connection_t *m_connection;
    xcb_intern_atom_cookie_t m_cookie;
    xcb_intern_atom_reply_t *m_reply;
    bool m_fetched;
};

#endif // XCBATOM_H
