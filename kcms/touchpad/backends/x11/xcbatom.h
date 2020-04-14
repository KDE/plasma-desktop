/*
 * Copyright (C) 2013 Alexander Mezin <mezin.alexander@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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
    operator xcb_atom_t() { return atom(); }

private:
    XcbAtom(const XcbAtom &);
    XcbAtom &operator =(const XcbAtom &);

    xcb_connection_t *m_connection;
    xcb_intern_atom_cookie_t m_cookie;
    xcb_intern_atom_reply_t *m_reply;
    bool m_fetched;
};

#endif // XCBATOM_H
