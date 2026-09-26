#include "actioncontext.h"
#include "actioncontext_p.h"

#include "actionregistry.h"

namespace QAK {

    ActionContextPrivate::ActionContextPrivate() = default;

    ActionContextPrivate::~ActionContextPrivate() = default;

    void ActionContextPrivate::init() {
    }

    ActionContext::ActionContext(QObject *parent)
        : ActionContext(*new ActionContextPrivate(), parent) {
    }

    ActionContext::~ActionContext() {
        Q_D(ActionContext);
        // Unregistered, so that the registry keeps no entry for a destroyed context.
        if (d->registry) {
            d->registry->removeContext(this);
        }
    }

    ActionRegistry *ActionContext::registry() const {
        Q_D(const ActionContext);
        return d->registry;
    }

    ActionContext::ActionContext(ActionContextPrivate &d, QObject *parent)
        : QObject(parent), d_ptr(&d) {
        d.q_ptr = this;
        d.init();
    }

}
