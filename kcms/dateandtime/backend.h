#pragma once

#include <QDateTime>
#include <QString>
#include <QScopedPointer>

class OrgFreedesktopTimedate1Interface;

class Backend {
public:
    static Backend *create();
    virtual ~Backend();
    virtual bool save(bool newUseNtp, const QDateTime &newTime, const QString &newTimezone) = 0;
    virtual bool canNtp() const = 0;
    virtual bool ntpEnabled() const = 0;
    virtual QString timeZone() const = 0;
protected:
    Backend();
};

class TimedatedBackend : public Backend {
public:
    TimedatedBackend();
    bool canNtp() const override;
    bool ntpEnabled() const override;
    QString timeZone() const override;
    bool save(bool newUseNtp, const QDateTime &newTime, const QString &newTimezone);

private:
    bool m_canNtp;
    bool m_ntpEnabled;
    QString m_timeZone;
    QScopedPointer<OrgFreedesktopTimedate1Interface> m_iface;
};


//Dave - I'm not sure there's any value in keeping this. It's emulated in systembsd. Gnome relies on it already too...
//IMHO abstraction for not using systemd should happen outside KDE
class LegacyBackend : Backend {
public:
    LegacyBackend();
};
