#include "actionregistry.h"
#include "actionregistry_p.h"

namespace QAK {

    ActionRegistryPrivate::ActionRegistryPrivate() {
    }

    ActionRegistryPrivate::~ActionRegistryPrivate() {
    }

    ActionRegistry::ActionRegistry(QObject *parent)
        : ActionFamily(*new ActionRegistryPrivate(), parent) {
    }

    ActionRegistry::~ActionRegistry() = default;

    void ActionRegistry::addExtension(const ActionExtension *extension) {
    }

    void ActionRegistry::removeExtension(const ActionExtension *extension) {
    }

    QStringList ActionRegistry::actionIds() const {
        return {};
    }

    ActionItemInfo ActionRegistry::actionInfo(const QString &id) const {
        return {};
    }

    ActionRegistry::Catalog ActionRegistry::catalog() const {
        return {};
    }

    ActionRegistry::Layouts ActionRegistry::layouts() const {
        return {};
    }

    void ActionRegistry::setLayouts(const Layouts &layouts) {
    }

    void ActionRegistry::resetLayouts() {
    }

    QJsonObject ActionRegistry::layoutsToJson(const Layouts &shortcutsFamily) {
        return {};
    }

    ActionRegistry::Layouts ActionRegistry::layoutsFromJson(const QJsonObject &obj) {
        return {};
    }

    ActionRegistry::ActionRegistry(ActionRegistryPrivate &d, QObject *parent)
        : ActionFamily(d, parent) {
    }

    void ActionRegistry::addContext(ActionContext *builder) {
    }

    void ActionRegistry::removeContext(ActionContext *builder) {
    }

    void ActionRegistry::updateContextLayouts() {
    }

    void ActionRegistry::updateContextTexts() {
    }

    void ActionRegistry::updateContextKeymap() {
    }

    void ActionRegistry::updateContextIcons() {
    }
}