#ifndef NEW_KEYBOARD_MAIN_H
#define NEW_KEYBOARD_MAIN_H

#include <KQuickAddons/ConfigModule>

class KcmNewKeyboard : public KQuickAddons::ConfigModule {
    Q_OBJECT
    Q_PROPERTY(QString keyboardModel READ keyboardModel WRITE setKeyboarModel NOTIFY keyboardModelChanged)

public:
    KcmNewKeyboard(QObject* parent, const QVariantList& args);
    virtual ~KcmNewKeyboard() override;

public Q_SLOTS:
    virtual void load() override;
    virtual void save() override;
    virtual void defaults() override;

public:
    void setKeyboarModel(const QString& model);
    QString keyboardModel() const;

Q_SIGNALS:
    void keyboardModelChanged();

private:
    QString m_keyboard_model;
};

#endif // NEW_KEYBOARD_MAIN_H
