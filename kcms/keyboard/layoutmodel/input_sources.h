#ifndef FCITXDETECTOR_H
#define FCITXDETECTOR_H

#include <QDBusServiceWatcher>

class InputSources : public QObject {
    Q_OBJECT

public:
    static InputSources* self();
    struct Sources {
        enum {
            UnknownSource = 0,
            XkbSource = 1,
            FcitxSource = 2,
        };
    };
    int currentSource() const;

Q_SIGNALS:
    void currentSourceChanged(int newSource);

private Q_SLOTS:
    void serviceRegistered(QString const& name);
    void serviceUnregistered(QString const& name);

private:
    InputSources();
    static InputSources m_self;
    QDBusServiceWatcher *m_fcitxWatcher;
    bool m_isFcitxRegistered;
};

#endif // FCITXDETECTOR_H
