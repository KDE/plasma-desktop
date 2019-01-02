/*
 *  Copyright (C) 2010 Andriy Rysin (rysin@kde.org)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */


#ifndef ISO_CODES_H_
#define ISO_CODES_H_

#include <QString>
#include <QList>
#include <QMap>

/**
 * Represents an item for iso-* standards which consists of attributes and their values
 */
struct IsoCodeEntry: public QMap<QString, QString>
{
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
	explicit IsoCodes(const QString& isoCode, const QString& isoCodesXmlDir=QStringLiteral("/usr/share/xml/iso-codes"));
	~IsoCodes();

	/**
	 * @return Returns the list of items for this iso-* standard
	 */
	QList<IsoCodeEntry> getEntryList();
	/**
	 * @return Returns the item for which given attribute has specified value
	 */
	const IsoCodeEntry* getEntry(const QString& attributeName, const QString& attributeValue);

private:
	IsoCodesPrivate* const d;
};

#endif /* ISO_CODES_H_ */
