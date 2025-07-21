#ifndef ACTIONCONTEXT_H
#define ACTIONCONTEXT_H

#include <QtGui/QAction>

#include <QAKCore/qakglobal.h>

namespace QAK {

    class ActionRegistry;

    class ActionContextPrivate;

    class QAK_CORE_EXPORT ActionContext : public QObject {
        Q_OBJECT
        Q_DECLARE_PRIVATE(ActionContext)
    public:
        explicit ActionContext(QObject *parent = nullptr);
        ~ActionContext();

        ActionRegistry *registry() const;

    protected:
        virtual void updateElement(ActionElement element) = 0;

    protected:
        ActionContext(ActionContextPrivate &d, QObject *parent = nullptr);

        std::unique_ptr<ActionContextPrivate> d_ptr;

        friend class ActionRegistry;
    };

}

#endif // ACTIONCONTEXT_H
