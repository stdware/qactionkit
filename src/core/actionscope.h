#ifndef ACTIONSCOPE_H
#define ACTIONSCOPE_H

#include <QAction>

#include <QAKCore/actionextension.h>

namespace QAK {

    class ActionScopePrivate;

    class QAK_CORE_EXPORT ActionScope {
    public:
        ActionScope();
        virtual ~ActionScope();

    public:
        void addExtension(const ActionExtension *extension);
        void removeExtension(const ActionExtension *extension);

    protected:
        virtual QAction *createAction(const QString &id) const;
    };

}

#endif // ACTIONSCOPE_H
