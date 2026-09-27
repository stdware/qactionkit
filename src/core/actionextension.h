#ifndef ACTIONEXTENSION_H
#define ACTIONEXTENSION_H

#include <optional>

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
        Q_PROPERTY(Type type READ type CONSTANT)
    public:
        /// The kinds of entry a layout node can hold.
        enum Type {
            Action,    ///< A reference to an action item
            Group,     ///< A reference to a group, whose children are placed here directly
            Menu,      ///< A reference to a sub-menu
            Separator, ///< A separator, which has no id
            Stretch,   ///< An expanding space, which has no id
        };
        Q_ENUM(Type)

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

    /// A translatable string of an item: the text written in the manifest and its translation.
    struct QAK_CORE_EXPORT ActionText {
        QString source;                     ///< The text written in the manifest
        std::optional<QString> translation; ///< The installed translation, or std::nullopt if none

        /// Returns the translation if one exists, and the source text otherwise.
        inline QString toString() const {
            return translation.value_or(source);
        }

        /// Returns the result of \c toString() without mnemonic markers, for display outside
        /// menus, such as in a command palette or a tool tip. An \c & before a character is
        /// removed and \c && becomes \c &, as \c QPlatformTheme::removeMnemonics() does. A marker
        /// in parentheses, such as \c (&O) in a Chinese text, is removed with the spaces before it.
        /// An ellipsis is kept.
        QString withoutMnemonic() const;
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

        /// Returns the text of the item and its translation. The translation context is the first
        /// specified among the \c textTr attribute of the item, the \c text attribute of
        /// \c translationContext in the configuration of the manifest, and
        /// \c QActionKit::ActionText. An empty text has no translation.
        ActionText text() const;
        /// Returns the category of the action, a label that a command palette shows before the
        /// text, such as File in File: Open, and its translation, in a context chosen as that of
        /// \c text() is, from \c categoryTr, \c category and \c QActionKit::ActionCategory. Only
        /// actions carry a category, and the category of other items is empty. The category is
        /// unrelated to the catalog, which places the item in the hierarchy of a settings page.
        ActionText category() const;
        /// Returns the description of the item and its translation, in a context chosen as that of
        /// \c text() is, from \c descriptionTr, \c description and
        /// \c QActionKit::ActionDescription.
        ActionText description() const;

        /// Returns the icon id, which defaults to the item id.
        QString icon() const;

        /// Returns the shortcuts declared by the extension. Only actions carry shortcuts, and the
        /// list of other items is empty.
        /// \note These are the defaults. \c ActionRegistry::actionShortcuts() returns the shortcuts
        ///       with the keymap of the user applied.
        QList<QKeySequence> shortcuts() const;

        /// Returns the id of the catalog node this item belongs to.
        QString catalog() const;

        /// Returns whether the item is a top-level menu-like item, such as a pop-up menu, a menu
        /// bar or a tool bar. Only menus and groups can be top-level, and the value of other items
        /// is \c false.
        bool topLevel() const;

        /// Returns the custom attributes of the item, keyed by name and namespace URI. A custom
        /// attribute always has a namespace, since AEC rejects any other attribute without one
        /// that the manifest format does not define.
        QMap<ActionAttributeKey, QString> attributes() const;

        /// Returns the children that the extension declares for the item, each of which refers to
        /// another item of the same extension. Only menus and groups have children, and the list of
        /// other items is empty.
        ///
        /// \note These are the defaults. \c ActionRegistry::layouts() returns the layout in effect,
        ///       which includes the insertions of other extensions and the changes made by the
        ///       user.
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
    /// The Action Extension Compiler emits it as static data in a generated C++ source file. The
    /// function named by the \c FUNCTION option of \c qak_add_action_extension() returns it, and
    /// the generated header declares that function.
    class QAK_CORE_EXPORT ActionExtension {
    public:
        QString version() const;

        QString id() const;
        QString hash() const;

        int itemCount() const;
        /// Returns the item at \a index, which must be in the range [0, itemCount()).
        ActionItemInfo item(int index) const;

        int insertionCount() const;
        /// Returns the insertion at \a index, which must be in the range [0, insertionCount()).
        ActionInsertion insertion(int index) const;

        /// The data of the extension, initialized by the generated code in the way moc initializes
        /// QMetaObject::d. Applications do not access it.
        struct Data {
            const ActionExtensionData *data;
        };
        Data d;
    };

}

#endif // ACTIONEXTENSION_H
