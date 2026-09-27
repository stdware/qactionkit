#ifndef ACTIONREGISTRY_H
#define ACTIONREGISTRY_H

#include <optional>

#include <QtCore/QMap>
#include <QtCore/QSharedData>

#include <QAKCore/actionextension.h>
#include <QAKCore/actionfamily.h>

namespace QAK {

    class ActionContext;

    class ActionRegistryPrivate;

    /// The logical hierarchy of the action items, from which a settings page presents them to the
    /// user. The hierarchy is a forest whose root is the empty id, and it is independent of the
    /// layout of the items in the menus.
    class ActionCatalog {
    public:
        /// Default constructor.
        inline ActionCatalog() = default;
        /// Constructs from a list of child-parent pairs.
        inline explicit ActionCatalog(const QVector<QPair<QString, QString>> &input) {
            setParentMap(input);
        }
        /// Constructs from an adjacency table.
        inline explicit ActionCatalog(const QMap<QString, QStringList> &input) {
            setAdjacencyTable(input);
        }

    public:
        inline QMap<QString, QStringList> adjacencyTable() const {
            return m_adjacencyMap;
        }
        /// Returns the id of the parent of \a id, which is empty for a top-level node, or
        /// \c std::nullopt if the catalog does not contain \a id.
        inline std::optional<QString> parent(const QString &id) const {
            if (auto it = m_parentMap.constFind(id); it != m_parentMap.constEnd()) {
                return it.value();
            }
            return std::nullopt;
        }
        inline QStringList children(const QString &id) const {
            return m_adjacencyMap.value(id);
        }
        inline void clear() {
            m_adjacencyMap.clear();
            m_parentMap.clear();
        }
        QAK_CORE_EXPORT void setAdjacencyTable(const QMap<QString, QStringList> &input);
        QAK_CORE_EXPORT void setParentMap(const QVector<QPair<QString, QString>> &input);

    protected:
        QMap<QString, QStringList> m_adjacencyMap;
        QMap<QString, QString> m_parentMap;
    };

    /// The composition of the menus, tool bars and groups of a view, stored as the adjacency map
    /// of a directed acyclic graph. What the user changes is stored as \c ActionLayoutChange
    /// records rather than as a whole layout.
    class ActionLayouts {
        Q_GADGET
    public:
        /// Default constructor.
        inline ActionLayouts() = default;
        /// Constructs from an adjacency map.
        inline explicit ActionLayouts(const QMap<QString, QVector<ActionLayoutEntry>> &input)
            : m_adjacencyMap(input) {
        }

    public:
        inline QMap<QString, QVector<ActionLayoutEntry>> adjacencyMap() const {
            return m_adjacencyMap;
        }
        QAK_CORE_EXPORT QJsonObject toJsonObject() const;
        QAK_CORE_EXPORT static ActionLayouts fromJsonObject(const QJsonObject &obj);

    protected:
        QMap<QString, QVector<ActionLayoutEntry>> m_adjacencyMap;
    };

    /// A change that the user made to the default layouts, which the registry replays on the
    /// layouts computed from the extensions. A change adds an entry to a container or removes one
    /// from it, and a move is a removal followed by an addition, both marked as moved.
    ///
    /// A position is an anchor as in \c ActionInsertion, together with a number of separators and
    /// stretches passed beyond the anchor, since those have no id to anchor to. With \c First, the
    /// position follows that many leading ones, with \c Last it precedes that many trailing ones,
    /// and with \c After and \c Before it lies that many further from \c relativeTo.
    struct QAK_CORE_EXPORT ActionLayoutChange {
        /// The kinds of change.
        enum Kind {
            Add,    ///< Adds the entry at the position
            Remove, ///< Removes the entry with the id, or the separator or stretch at the position
        };

        Kind kind = Add;
        QString container;       ///< The id of the menu or group that the change applies to
        ActionLayoutEntry entry; ///< The entry added or removed
        ActionInsertion::Anchor anchor = ActionInsertion::Last; ///< The anchor of the position
        QString relativeTo; ///< The id that an \c After or \c Before anchor refers to
        int offset = 0;     ///< The separators and stretches passed beyond the anchor
        /// Whether the change is half of a move. An addition that is half of a move applies only if
        /// the removal of the same id before it has applied.
        bool moved = false;

        inline bool operator==(const ActionLayoutChange &RHS) const {
            return kind == RHS.kind && container == RHS.container && entry == RHS.entry &&
                   anchor == RHS.anchor && relativeTo == RHS.relativeTo && offset == RHS.offset &&
                   moved == RHS.moved;
        }
        inline bool operator!=(const ActionLayoutChange &RHS) const {
            return !(*this == RHS);
        }

        QJsonObject toJsonObject() const;
        /// Returns the change that \a obj stores, or \c std::nullopt if a field is missing or
        /// invalid.
        static std::optional<ActionLayoutChange> fromJsonObject(const QJsonObject &obj);
    };

    /// The central repository of the action extensions of an application, holding the catalog, the
    /// layouts and the customizations made by the user. An application normally has one registry,
    /// with one \c ActionContext per window registered with it.
    class QAK_CORE_EXPORT ActionRegistry : public ActionFamily {
        Q_OBJECT
        Q_DECLARE_PRIVATE(ActionRegistry)
    public:
        explicit ActionRegistry(QObject *parent = nullptr);
        ~ActionRegistry();

    public:
        QList<const ActionExtension *> extensions() const;
        void setExtensions(const QList<const ActionExtension *> &extensions);
        void addExtension(const ActionExtension *extension);

        QStringList actionIds() const;
        /// Returns the item \a id, or \c std::nullopt if no registered extension declares it.
        std::optional<ActionItemInfo> actionInfo(const QString &id) const;
        ActionCatalog catalog() const;

    public:
        /// Returns the layouts computed from the registered extensions, which the changes of the
        /// user do not affect.
        ActionLayouts defaultLayouts() const;
        /// Returns the layouts in effect: the default layouts with the changes of the user
        /// replayed on them.
        ActionLayouts layouts() const;

        /// Returns the changes that the user has made to the default layouts, which are what an
        /// application saves. Setting an empty list restores the default layouts.
        QVector<ActionLayoutChange> layoutChanges() const;
        /// Sets the changes of the user, which are replayed in order on the default layouts. A
        /// change that cannot apply, for example because an extension no longer declares its
        /// entry, is skipped with a warning.
        void setLayoutChanges(const QVector<ActionLayoutChange> &changes);
        /// Appends \a change to the changes of the user and replays it.
        void addLayoutChange(const ActionLayoutChange &change);
        /// Returns the changes that turn the default layouts into \a edited, for a settings page
        /// that edits the layouts as a whole. Set as the changes of the user, they reproduce
        /// \a edited as long as the default layouts stay the same. A container that \a edited
        /// omits keeps its default children.
        QVector<ActionLayoutChange> computeLayoutChanges(const ActionLayouts &edited) const;

        inline QList<QKeySequence> actionShortcuts(const QString &id) const;

    public:
        /// Registers \a ctx with the registry, and removes it from the registry it was registered
        /// with before. A destroyed context is removed from its registry.
        void addContext(ActionContext *ctx);
        /// Unregisters \a ctx from the registry.
        void removeContext(ActionContext *ctx);
        /// Rebuilds \a element of every registered context from the current state of the
        /// registry.
        void updateContext(ActionElement element);

    protected:
        explicit ActionRegistry(ActionRegistryPrivate &d, QObject *parent = nullptr);
    };

    inline QList<QKeySequence> ActionRegistry::actionShortcuts(const QString &id) const {
        if (const auto o = shortcuts(id); o) {
            return o.value();
        }
        if (const auto info = actionInfo(id)) {
            return info->shortcuts();
        }
        return {};
    }

}

#endif // ACTIONREGISTRY_H
