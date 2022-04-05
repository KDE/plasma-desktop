/*
    SPDX-FileCopyrightText: 2010 Andriy Rysin <rysin@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "iso_codes.h"
#include "debug.h"

#include <KLocalizedString>

#include <QFile>
#include <QXmlStreamReader>

class IsoCodesPrivate
{
public:
    IsoCodesPrivate(const QString &isoCode_, const QString &isoCodesXmlDir_)
        : isoCode(isoCode_)
        , isoCodesXmlDir(isoCodesXmlDir_)
        , loaded(false)
    {
    }
    void buildIsoEntryList();

    const QString isoCode;
    const QString isoCodesXmlDir;
    QList<IsoCodeEntry> isoEntryList;
    bool loaded;
};

IsoCodes::IsoCodes(const QString &isoCode, const QString &isoCodesXmlDir)
    : d(new IsoCodesPrivate(isoCode, isoCodesXmlDir))
{
}

IsoCodes::~IsoCodes()
{
    delete d;
}

QList<IsoCodeEntry> IsoCodes::getEntryList()
{
    if (!d->loaded) {
        d->buildIsoEntryList();
    }
    return d->isoEntryList;
}

// const char* IsoCodes::iso_639="639";
const char IsoCodes::iso_639_3[] = "639_3";
const char IsoCodes::attr_name[] = "name";
// const char* IsoCodes::attr_iso_639_2B_code="iso_639_2B_code";
// const char* IsoCodes::attr_iso_639_2T_code="iso_639_2T_code";
// const char* IsoCodes::attr_iso_639_1_code="iso_639_1_code";
const char IsoCodes::attr_iso_639_3_id[] = "id";

const IsoCodeEntry *IsoCodes::getEntry(const QString &attributeName, const QString &attributeValue)
{
    if (!d->loaded) {
        d->buildIsoEntryList();
    }
    for (QList<IsoCodeEntry>::Iterator it = d->isoEntryList.begin(); it != d->isoEntryList.end(); ++it) {
        const IsoCodeEntry *isoCodeEntry = &(*it);
        if (isoCodeEntry->value(attributeName) == attributeValue)
            return isoCodeEntry;
    }
    return nullptr;
}

void IsoCodesPrivate::buildIsoEntryList()
{
    loaded = true;

    QFile file(QStringLiteral("%1/iso_%2.xml").arg(isoCodesXmlDir, isoCode));
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        qCCritical(KCM_KEYBOARD) << "Can't open the xml file" << file.fileName();
        return;
    }

    QString qName("iso_" + isoCode + "_entry");
    QXmlStreamReader reader(&file);
    while (!reader.atEnd()) {
        if (reader.readNext() == QXmlStreamReader::StartElement && reader.name() == qName) {
            IsoCodeEntry entry;
            const auto attributes = reader.attributes();
            for (const auto &attr : attributes) {
                entry.insert(attr.qualifiedName().toString(), attr.value().toString());
            }
            isoEntryList.append(entry);
        }
    }

    if (reader.hasError()) {
        qCCritical(KCM_KEYBOARD) << "Failed to parse the xml file" << file.fileName() << reader.errorString();
        return;
    }

    qCDebug(KCM_KEYBOARD) << "Loaded" << isoEntryList.count() << ("iso entry definitions for iso" + isoCode) << "from" << file.fileName();
}
