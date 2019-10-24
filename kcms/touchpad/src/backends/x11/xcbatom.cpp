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

#include "xcbatom.h"

#include <cstdlib>
#include <cstring>

XcbAtom::XcbAtom() : m_connection(nullptr), m_reply(nullptr), m_fetched(false)
{
}

XcbAtom::XcbAtom(xcb_connection_t *c, const char *name, bool onlyIfExists)
    : m_reply(nullptr), m_fetched(false)
{
    intern(c, name, onlyIfExists);
}

void XcbAtom::intern(xcb_connection_t *c, const char *name, bool onlyIfExists)
{
    m_connection = c;
    m_cookie = xcb_intern_atom(c, onlyIfExists, std::strlen(name), name);
}

XcbAtom::~XcbAtom()
{
    std::free(m_reply);
}

xcb_atom_t XcbAtom::atom()
{
    if (!m_fetched) {
        m_fetched = true;
        m_reply = xcb_intern_atom_reply(m_connection, m_cookie, nullptr);
    }
    if (m_reply) {
        return m_reply->atom;
    } else {
        return 0;
    }
}
