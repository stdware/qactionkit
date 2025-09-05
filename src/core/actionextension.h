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

    /// \struct ActionAttributeKey
    /// \brief Represents an attribute key with name and namespace URI
    struct QAK_CORE_EXPORT ActionAttributeKey {
        QString name;
        QString namespaceUri;

        ActionAttributeKey() = default;
        ActionAttributeKey(const QString &name, const QString &namespaceUri = QString())
            : name(name), namespaceUri(namespaceUri) {}

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

    /// \class ActionLayoutEntry
    /// \brief An entry in an \c ActionLayout, representing an action, a group, a menu, a separator,
    /// or a stretch.
    class ActionLayoutEntry {
        Q_GADGET
        Q_PROPERTY(QString id READ id CONSTANT)
        Q_PROPERTY(ActionLayoutEntry::Type type READ type CONSTANT)
    public:
        enum Type {
            Action,
            Group,
            Menu,
            Separator,
            Stretch,
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
        inline bool isNull() const {
            return !m_id.isEmpty();
        }

    protected:
        QString m_id;
        Type m_type;
    };

    /// \class ActionItemInfo
    /// \brief Provides metadata about an action item.
    class QAK_CORE_EXPORT ActionItemInfo {
    public:
        ActionItemInfo();
        bool isNull() const;

        enum Type {
            Action,
            Group,
            Menu,
            Phony,
        };

        QString id() const;
        Type type() const;

        QString text(bool translated = false) const;
        QString actionClass(bool translated = false) const;
        QString description(bool translated = false) const;

        /// \brief Returns the icon id.
        QString icon() const;

        /// \brief Returns the shortcuts, only valid for action type.
        QList<QKeySequence> shortcuts() const;

        /// \brief Returns the catalog parent id of the action item.
        QString catalog() const;

        /// \brief Returns whether the action item is a top-level menu-like item such as a pop-up
        /// menu, menubar or toolbar.
        bool topLevel() const;

        /// \brief Returns the attributes of the action item.
        /// Reserved attributes:
        ///   - textTr: the translation context of \c text()
        ///   - classTr: the translation context of \c actionClass()
        ///   - descriptionTr: the translation context of \c description()
        /// Key format: ActionAttributeKey(attributeName, namespaceUri) -> value
        QMap<ActionAttributeKey, QString> attributes() const;

        /// \brief Returns the children of the action item, each child is an \c ActionLayoutEntry
        /// reference to another \c ActionItemInfo in the same extension.
        QVector<ActionLayoutEntry> children() const;

    private:
        const ActionExtensionData *e;
        int i;

        friend class ActionExtension;
        friend class ActionRegistry;
    };

    /// \class ActionInsertion
    /// \brief Provides metadata about an insertion routine  which should be applied when building
    /// the action layouts.
    class QAK_CORE_EXPORT ActionInsertion {
    public:
        ActionInsertion();
        bool isNull() const;

        /// \enum Anchor
        /// \brief The position where the insertion should be applied.
        enum Anchor {
            /// \brief Insert the items at the beginning.
            Last,
            /// \brief Insert the items at the end.
            First,
            /// \brief Insert the items after the item \c relativeTo.
            After,
            /// \brief Insert the items before the item \c relativeTo.
            Before,
        };
        Anchor anchor() const;

        /// \brief Returns the target item id.
        QString target() const;

        /// \brief Returns the relative item id in \c target(), only valid for \c After and
        /// \c Before anchors.
        QString relativeTo() const;

        /// \brief Returns the items to be inserted, each item is an \c ActionLayoutEntry reference
        /// to another \c ActionItemInfo in the same extension.
        QVector<ActionLayoutEntry> items() const;

    private:
        const ActionExtensionData *e;
        int i;

        friend class ActionExtension;
        friend class ActionRegistry;
    };

    /// \class ActionExtension
    /// \brief Contains the action item metadata to build the action layouts.
    /// \note An \c ActionExtension is created by the Action Extension Compiler in a generated C++
    /// source file, and is referenced by using \c QAK_STATIC_ACTION_EXTENSION macro.
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

/// \macro QAK_STATIC_ACTION_EXTENSION
/// \brief Returns a reference to the static action extension with the given name.
/// \warning This macro cannot be used in a namespace.
///
/// \example
///     A function that returns a reference to a given action extension with identifier
///     "core_actions"
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