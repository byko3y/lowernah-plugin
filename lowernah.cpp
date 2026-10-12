#include <QObject>
#include <QtPlugin>
#include <QWidget>
#include "psiplugin.h"

class LowercasePlugin : public QObject, public PsiPlugin {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "me.bykozy.lowernah")
    Q_INTERFACES(PsiPlugin)

public:
    QString name() const override      { return "Lowercase Plugin"; }
    QString shortName() const override { return "lowercase"; }
    QString version() const override   { return "0.1"; }
    QWidget *options() override        { return nullptr; }
    bool enable() override             { return true; }
    bool disable() override            { return true; }
    void applyOptions() override       {}
    void restoreOptions() override     {}
};

#include "lowernah.moc"
