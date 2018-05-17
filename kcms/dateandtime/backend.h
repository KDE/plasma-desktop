#pragma once

class Backend {
public:
    static Backend *create();
    virtual bool save(bool newUseNtp, const QDateTime &newTime, const QString &newTimezone) = nullptr;
    virtual bool canNtp() const = nullptr;
    virtual bool ntpEnabled() const = nullptr;
    virtual QString timeZone() const = nullptr;
protected:
    Backend();
};

class TimedatedBackend : Backend {
public:
    TimedatedBackend();
    bool canNtp() const override;
    bool ntpEnabled() const override;
    bool QString timeZone() const override;
    void save(bool newUseNtp, const QDateTime &newTime, const QString &newTimezone);

private:
    bool canNtp;
    bool ntpEnabled;
    QString timeZone;
    QScopedPointer<OrgFreedesktopTimedate1Interface> m_iface;
};


//Dave - I'm not sure there's any value in keeping this. It's emulated in systembsd. Gnome relies on it already too...
//IMHO abstraction for not using systemd should happen outside KDE
class LegacyBackend : Backend {
public:
    LegacyBackend();
};
