/*
    SPDX-FileCopyrightText: 2010 Andriy Rysin <rysin@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef ISO_CODES_H_
#define ISO_CODES_H_

#include <QList>
#include <QMap>
#include <QString>

/**
 * Represents an item for iso-* standards which consists of attributes and their values
 */
struct IsoCodeEntry : public QMap<QString, QString> {
};

class IsoCodesPrivate;

/**
 * Represents a set of codes for iso-* standards.
 * Uses iso-codes project to read and localize the values.
 */
class IsoCodes
{
public:
    //	static const char* iso_639;
    static const char iso_639_3[];

    static const char attr_name[];
    //	static const char* attr_iso_639_2B_code;
    //	static const char* attr_iso_639_2T_code;
    //	static const char* attr_iso_639_1_code;
    static const char attr_iso_639_3_id[];

    /**
     * @param isoCode Code for iso standard, i.e. "639", for convenience there's iso_* constants defined
     * @param isoCodesXmlDir the directory where ISO data are stored
     */
    explicit IsoCodes(const QString &isoCode, const QString &isoCodesXmlDir = QStringLiteral("/usr/share/xml/iso-codes"));
    ~IsoCodes();

    /**
     * @return Returns the list of items for this iso-* standard
     */
    QList<IsoCodeEntry> getEntryList();
    /**
     * @return Returns the item for which given attribute has specified value
     */
    const IsoCodeEntry *getEntry(const QString &attributeName, const QString &attributeValue);

private:
    IsoCodesPrivate *const d;
};

#endif /* ISO_CODES_H_ */
