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
        Q_PROPERTY(QQmlComponent *menuComponent READ menuComponent WRITE setMenuComponent NOTIFY menuComponentChanged)
        Q_PROPERTY(QQmlComponent *separatorComponent READ separatorComponent WRITE setSeparatorComponent NOTIFY separatorComponentChanged)
        Q_PROPERTY(QQmlComponent *stretchComponent READ stretchComponent WRITE setStretchComponent NOTIFY stretchComponentChanged)

    public:
        explicit QuickActionContext(QObject *parent = nullptr);
        ~QuickActionContext() override;

        Q_INVOKABLE void addAction(const QString &id, QQmlComponent *component);

        QQmlComponent *menuComponent() const;
        void setMenuComponent(QQmlComponent *component);

        QQmlComponent *separatorComponent() const;
        void setSeparatorComponent(QQmlComponent *component);

        QQmlComponent *stretchComponent() const;
        void setStretchComponent(QQmlComponent *component);

    signals:
        void actionChanged(const QString &id);
        void menuComponentChanged();
        void separatorComponentChanged();
        void stretchComponentChanged();
        void layoutsAboutToUpdate();
        void textsAboutToUpdate();
        void iconsAboutToUpdate();
        void keymapAboutToUpdate();

    protected:
        void updateLayouts() override;
        void updateTexts() override;
        void updateIcons() override;
        void updateKeymap() override;

    private:
        QScopedPointer<QuickActionContextPrivate> d_ptr;
    };

}

#endif // QUICKACTIONCONTEXT_H