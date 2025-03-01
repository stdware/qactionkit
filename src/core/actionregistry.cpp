#include "actionregistry.h"
#include "actionregistry_p.h"

namespace QAK {

    ActionCatalog::ActionCatalog() : d(new ActionCatalogData()) {
    }

    ActionCatalog::ActionCatalog(const QString &id) : ActionCatalog() {
        d->id = id;
    }

    ActionCatalog::ActionCatalog(const ActionCatalog &other) = default;

    ActionCatalog &ActionCatalog::operator=(const ActionCatalog &other) = default;

    ActionCatalog::~ActionCatalog() = default;

    QString ActionCatalog::id() const {
        return d->id;
    }

    void ActionCatalog::setId(const QString &id) {
        d->id = id;
    }

    QList<ActionCatalog> ActionCatalog::children() const {
        return d->children;
    }

    void ActionCatalog::addChild(const ActionCatalog &child) {
        d->children.append(child);
    }

    void ActionCatalog::setChildren(const QList<ActionCatalog> &children) {
        d->children = children;
    }



    ActionLayout::ActionLayout() : d(new ActionLayoutData()) {
    }

    ActionLayout::ActionLayout(const QString &id) : ActionLayout() {
        d->id = id;
    }

    ActionLayout::ActionLayout(const ActionLayout &other) = default;

    ActionLayout &ActionLayout::operator=(const ActionLayout &other) = default;

    ActionLayout::~ActionLayout() = default;

    QString ActionLayout::id() const {
        return d->id;
    }

    void ActionLayout::setId(const QString &id) {
        d->id = id;
    }

    ActionLayoutInfo::Type ActionLayout::type() const {
        return d->type;
    }

    void ActionLayout::setType(ActionLayoutInfo::Type type) {
        d->type = type;
    }

    QList<ActionLayout> ActionLayout::children() const {
        return d->children;
    }

    void ActionLayout::addChild(const ActionLayout &child) {
        d->children.append(child);
    }

    void ActionLayout::setChildren(const QList<ActionLayout> &children) {
        d->children = children;
    }



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

    ActionCatalog ActionRegistry::catalog() const {
        return {};
    }

    QList<ActionLayout> ActionRegistry::layouts() const {
        return {};
    }

    void ActionRegistry::setLayouts(const QList<ActionLayout> &layouts) {
    }

    void ActionRegistry::resetLayouts() {
    }

    QJsonObject ActionRegistry::layoutsToJson(const ShortcutsFamily &shortcutsFamily) {
        return {};
    }

    ActionFamily::ShortcutsFamily ActionRegistry::layoutsFromJson(const QJsonObject &obj) {
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