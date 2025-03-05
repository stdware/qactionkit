#include "actionregistry.h"
#include "actionregistry_p.h"

#include <utility>

#include "qakglobal_p.h"
#include "actioncontext_p.h"

namespace QAK {

    ActionRegistryPrivate::ActionRegistryPrivate() {
    }

    ActionRegistryPrivate::~ActionRegistryPrivate() {
    }

    void ActionRegistryPrivate::flushActionItems() const {
        if (!actionItemsDirty) {
            return;
        }
        catalog.clear();
        layouts.clear();
        actionItems.clear();
        for (const auto &pair : std::as_const(extensions)) {
            auto &e = pair.second;
            for (int i = 0; i < e->itemCount(); ++i) {
                const auto &item = e->item(i);
                auto id = item.id().toLower();
                if (actionItems.contains(id)) {
                    continue;
                }
                actionItems.append(id, item);
            }
        }
        catalog = defaultCatalog();
        layouts = defaultLayouts();

        actionItemsDirty = false;
    }


    ActionRegistry::Catalog ActionRegistryPrivate::defaultCatalog() const {
        // TODO: implement default catalog
        return {};
    }

    ActionRegistry::Layouts ActionRegistryPrivate::defaultLayouts() const {
        // TODO: implement default layouts
        return {};
    }

    ActionRegistry::ActionRegistry(QObject *parent)
        : ActionFamily(*new ActionRegistryPrivate(), parent) {
    }

    ActionRegistry::~ActionRegistry() = default;

    void ActionRegistry::addExtension(const ActionExtension *extension) {
        Q_D(ActionRegistry);

        const auto &id = extension->id();
        if (d->extensions.contains(id)) {
            qCWarning(qActionKitLog) << "Action extension with id" << id << "already exists";
            return;
        }
        d->extensions.append(id, extension);
        d->actionItemsDirty = true;
    }

    void ActionRegistry::removeExtension(const ActionExtension *extension) {
        Q_D(ActionRegistry);

        auto it = d->extensions.find(extension->id());
        if (it == d->extensions.end()) {
            qCWarning(qActionKitLog)
                << "Action extension with id" << extension->id() << "not found";
            return;
        }
        d->extensions.erase(it);
        d->actionItemsDirty = true;
    }

    QStringList ActionRegistry::actionIds() const {
        Q_D(const ActionRegistry);

        d->flushActionItems();

        QStringList result;
        for (const auto &pair : std::as_const(d->actionItems)) {
            result.append(pair.second.id());
        }
        return result;
    }

    ActionItemInfo ActionRegistry::actionInfo(const QString &id) const {
        Q_D(const ActionRegistry);
        d->flushActionItems();
        return d->actionItems.value(id.toLower());
    }

    ActionRegistry::Catalog ActionRegistry::catalog() const {
        Q_D(const ActionRegistry);
        d->flushActionItems();
        return d->catalog;
    }

    ActionRegistry::Layouts ActionRegistry::layouts() const {
        Q_D(const ActionRegistry);
        d->flushActionItems();
        return d->layouts;
    }

    void ActionRegistry::setLayouts(const Layouts &layouts) {
        Q_D(ActionRegistry);

        // TODO: check if layouts are valid

        d->layouts = layouts;
    }

    void ActionRegistry::resetLayouts() {
        Q_D(ActionRegistry);
        d->flushActionItems();
        d->layouts = d->defaultLayouts();
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

    void ActionRegistry::addContext(ActionContext *ctx) {
        Q_D(ActionRegistry);
        d->contexts.removeAll(nullptr);
        d->contexts.removeAll(ctx);
        d->contexts.append(ctx);

        auto &reg = ctx->d_func()->registry;
        if (reg) {
            reg->removeContext(ctx);
        }
        reg = this;
    }

    void ActionRegistry::removeContext(ActionContext *ctx) {
        Q_D(ActionRegistry);
        for (auto &item : d->contexts) {
            if (item == ctx) {
                item = nullptr;
            }
        }
        ctx->d_func()->registry = nullptr;
    }

    void ActionRegistry::updateContextLayouts() {
        Q_D(ActionRegistry);
        for (auto &ctx : d->contexts) {
            if (ctx) {
                ctx->updateLayouts();
            }
        }
    }

    void ActionRegistry::updateContextTexts() {
        Q_D(ActionRegistry);
        for (auto &ctx : d->contexts) {
            if (ctx) {
                ctx->updateTexts();
            }
        }
    }

    void ActionRegistry::updateContextKeymap() {
        Q_D(ActionRegistry);
        for (auto &ctx : d->contexts) {
            if (ctx) {
                ctx->updateKeymap();
            }
        }
    }

    void ActionRegistry::updateContextIcons() {
        Q_D(ActionRegistry);
        for (auto &ctx : d->contexts) {
            if (ctx) {
                ctx->updateIcons();
            }
        }
    }

}