#ifndef ACTIONEXTENSION_H
#define ACTIONEXTENSION_H

#include <QtCore/QMap>
#include <QtCore/QVector>
#include <QtCore/QStringList>
#include <QtGui/QKeySequence>

#include <QAKCore/qakglobal.h>

namespace QAK {

    class ActionRegistry;

    class ActionExtensionData;

    class ActionExtension;

    /// An attribute key, identified by its name and the namespace URI it was declared in.
    struct QAK_CORE_EXPORT ActionAttributeKey {
        QString name;
        QString namespaceUri;

        ActionAttributeKey() = default;
        ActionAttributeKey(const QString &name, const QString &namespaceUri = QString())
            : name(name), namespaceUri(namespaceUri) {
        }

        bool operator==(const ActionAttributeKey &other) const {
            return name == other.name && namespaceUri == other.namespaceUri;
        }

        bool operator!=(const ActionAttributeKey &other) const {
            return !(*this == other);
        }

        bool operator<(const ActionAttributeKey &other) const {
            if (name != other.name)
                return name < other.name;
            return namespaceUri < other.namespaceUri;
        }
    };

    /// An entry of an \c ActionLayouts node, referring to an action, a group or a menu declared in
    /// the same extension, or standing for a separator or a stretch.
    class QAK_CORE_EXPORT ActionLayoutEntry {
        Q_GADGET
        Q_PROPERTY(QString id READ id CONSTANT)
        Q_PROPERTY(ActionLayoutEntry::Type type READ type CONSTANT)
    public:
        /// The kinds of entry a layout node can hold.
        enum Type {
            Action,    ///< A reference to an action item
            Group,     ///< A reference to a group, whose children are placed here directly
            Menu,      ///< A reference to a sub-menu
            Separator, ///< A separator, which has no id
            Stretch,   ///< An expanding space, which has no id
        };

        inline ActionLayoutEntry(const QString &id = {}, Type type = Action)
            : m_id(id), m_type(type) {
        }
        inline QString id() const {
            return m_id;
        }
        inline Type type() const {
            return m_type;
        }
        /// Returns whether the entry is unusable: an entry that refers to an item and has no id.
        /// Separators and stretches have no id and are never null.
        inline bool isNull() const {
            return m_type != Separator && m_type != Stretch && m_id.isEmpty();
        }

        /// Returns whether both entries have the same type and the same id.
        inline bool operator==(const ActionLayoutEntry &RHS) const {
            return m_type == RHS.m_type && m_id == RHS.m_id;
        }
        inline bool operator!=(const ActionLayoutEntry &RHS) const {
            return !(*this == RHS);
        }

    protected:
        QString m_id;
        Type m_type;
    };

    /// The metadata of a single item declared by an \c ActionExtension.
    ///
    /// A view on the static data of the extension, which is inexpensive to copy and must not
    /// outlive the extension.
    class QAK_CORE_EXPORT ActionItemInfo {
    public:
        ActionItemInfo();
        bool isNull() const;

        /// The kinds of item an extension can declare.
        enum Type {
            Action, ///< A leaf item the user can invoke
            Group,  ///< A named list of items, whose children are placed in its parent directly
            Menu,   ///< A menu, a menu bar or a tool bar
            Phony,  ///< A catalog node only, which no view displays
        };

        QString id() const;
        Type type() const;

        /// Returns the text of the item, translated in the context of the \c textTr attribute if
        /// \a translated is \c true.
        ///
        /// \note The translated text is empty if no translation is installed.
        QString text(bool translated = false) const;
        /// Returns the category of the action, a label that a command palette shows before the
        /// text, such as File in File: Open, translated in the context of the \c categoryTr
        /// attribute if \a translated is \c true. Only actions carry a category. The category is
        /// unrelated to the catalog, which places the item in the hierarchy of a settings page.
        QString category(bool translated = false) const;
        QString description(bool translated = false) const;

        /// Returns the icon id, which defaults to the item id.
        QString icon() const;

        /// Returns the shortcuts declared by the extension. Only actions carry shortcuts.
        /// \note These are the defaults. \c ActionRegistry::actionShortcuts() returns the shortcuts
        ///       with the keymap of the user applied.
        QList<QKeySequence> shortcuts() const;

        /// Returns the id of the catalog node this item belongs to.
        QString catalog() const;

        /// Returns whether the item is a top-level menu-like item, such as a pop-up menu, a menu
        /// bar or a tool bar.
        bool topLevel() const;

        /// Returns the attributes of the item, keyed by name and namespace URI. Three attribute
        /// names are reserved and hold translation contexts:
        /// \li \c textTr for \c text()
        /// \li \c categoryTr for \c category()
        /// \li \c descriptionTr for \c description()
        QMap<ActionAttributeKey, QString> attributes() const;

        /// Returns the children of the item, each of which refers to another item of the same
        /// extension.
        QVector<ActionLayoutEntry> children() const;

    private:
        const ActionExtensionData *e;
        int i;

        friend class ActionExtension;
        friend class ActionRegistry;
    };

    /// An instruction to insert items into a menu declared by another extension, applied while the
    /// action layouts are built. An extension contributes to a menu it does not own through
    /// insertions only.
    class QAK_CORE_EXPORT ActionInsertion {
    public:
        ActionInsertion();
        bool isNull() const;

        /// The position within the target where the items are inserted.
        enum Anchor {
            Last,   ///< At the end of the target, the default
            First,  ///< At the beginning of the target
            After,  ///< Right after \c relativeTo
            Before, ///< Right before \c relativeTo
        };
        Anchor anchor() const;

        /// Returns the id of the item the insertion applies to.
        QString target() const;

        /// Returns the id of the item within \c target() next to which the items are placed. Used
        /// by the \c After and \c Before anchors only.
        QString relativeTo() const;

        /// Returns the items to be inserted, each of which refers to an item of the same
        /// extension.
        QVector<ActionLayoutEntry> items() const;

    private:
        const ActionExtensionData *e;
        int i;

        friend class ActionExtension;
        friend class ActionRegistry;
    };

    /// The compiled form of an action extension manifest, holding every item and insertion from
    /// which the registry builds the catalog and the layouts.
    ///
    /// The Action Extension Compiler emits it as static data in a generated C++ source file, and
    /// \c QAK_STATIC_ACTION_EXTENSION returns it.
    class QAK_CORE_EXPORT ActionExtension {
    public:
        QString version() const;

        QString id() const;
        QString hash() const;

        int itemCount() const;
        ActionItemInfo item(int index) const;

        int insertionCount() const;
        ActionInsertion insertion(int index) const;

        struct Data {
            const ActionExtensionData *data;
        };
        Data d;
    };

}

/// Returns the static action extension \a name , the identifier passed to the Action Extension
/// Compiler for the manifest.
/// \warning The macro declares an extern function, so it cannot be used inside a namespace.
///
/// \code
///     static auto getActionExtension() {
///         return QAK_STATIC_ACTION_EXTENSION(core_actions);
///     }
/// \endcode
#define QAK_STATIC_ACTION_EXTENSION(name)                                                          \
    []() {                                                                                         \
        extern const QAK::ActionExtension *QT_MANGLE_NAMESPACE(                                    \
            qakGetStaticActionExtension_##name)();                                                 \
        return QT_MANGLE_NAMESPACE(qakGetStaticActionExtension_##name)();                          \
    }()

#endif // ACTIONEXTENSION_H
