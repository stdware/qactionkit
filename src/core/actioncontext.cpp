#include "actioncontext.h"
#include "actioncontext_p.h"

namespace QAK {

    ActionContextPrivate::ActionContextPrivate() {
    }

    ActionContextPrivate::~ActionContextPrivate() = default;

    void ActionContextPrivate::init() {
    }

    ActionContext::ActionContext(QObject *parent)
        : ActionContext(*new ActionContextPrivate(), parent) {
    }

    ActionContext::~ActionContext() = default;

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