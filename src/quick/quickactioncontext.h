#ifndef QUICKACTIONCONTEXT_H
#define QUICKACTIONCONTEXT_H

#include <QAKCore/actioncontext.h>
#include <QAKQuick/qakquickglobal.h>

class QQmlComponent;

namespace QAK {

    class QuickActionContextPrivate;

    class QAK_QUICK_EXPORT QuickActionContext : public ActionContext {
        Q_OBJECT
        Q_DECLARE_PRIVATE(QuickActionContext)
        Q_PROPERTY(QStringList actions READ actions NOTIFY actionsChanged)
        Q_PROPERTY(QQmlComponent *menuComponent READ menuComponent WRITE setMenuComponent NOTIFY
                       menuComponentChanged)
        Q_PROPERTY(QQmlComponent *separatorComponent READ separatorComponent WRITE
                       setSeparatorComponent NOTIFY separatorComponentChanged)
        Q_PROPERTY(QQmlComponent *stretchComponent READ stretchComponent WRITE setStretchComponent
                       NOTIFY stretchComponentChanged)

    public:
        explicit QuickActionContext(QObject *parent = nullptr);
        ~QuickActionContext() override;

        Q_INVOKABLE void addAction(const QString &id, QQmlComponent *component);
        Q_INVOKABLE QQmlComponent *action(const QString &id) const;
        Q_INVOKABLE void attachActionInfo(const QString &id, QObject *object);
        QStringList actions() const;

        QQmlComponent *menuComponent() const;
        void setMenuComponent(QQmlComponent *component);

        QQmlComponent *separatorComponent() const;
        void setSeparatorComponent(QQmlComponent *component);

        QQmlComponent *stretchComponent() const;
        void setStretchComponent(QQmlComponent *component);

        void updateElement(ActionElement element) override;

    signals:
        void actionChanged(const QString &id);
        void actionsChanged();
        void menuComponentChanged();
        void separatorComponentChanged();
        void stretchComponentChanged();
        void layoutsAboutToUpdate();
        void textsAboutToUpdate();
        void iconsAboutToUpdate();
        void keymapAboutToUpdate();

    private:
        QScopedPointer<QuickActionContextPrivate> d_ptr;
    };

}

#endif // QUICKACTIONCONTEXT_H