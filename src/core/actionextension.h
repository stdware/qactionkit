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

    class ActionLayoutEntry {
    public:
        enum Type {
            Action,
            Group,
            Menu,
            Separator,
            Stretch,
        };

        inline ActionLayoutEntry(const QString &id = {}, Type type = Action)
            : m_id(id), m_type(type) {}
        inline QString id() const { return m_id; }
        inline Type type() const { return m_type; }
        inline bool isNull() const { return !m_id.isEmpty(); }

    protected:
        QString m_id;
        Type m_type;
    };

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

        QString text() const;
        QString actionClass() const;
        QString description() const;
        QString icon() const; // icon id
        QList<QKeySequence> shortcuts() const;
        QString catalog() const;
        bool topLevel() const;

        /**
         * Reserved keys:
         * - textTr
         * - classTr
         * - descriptionTr
         */
        QMap<QString, QString> attributes() const;

        QVector<ActionLayoutEntry> children() const;

    private:
        const ActionExtensionData *e;
        int i;

        friend class ActionExtension;
        friend class ActionRegistry;
    };

    class QAK_CORE_EXPORT ActionInsertion {
    public:
        ActionInsertion();
        bool isNull() const;

        enum Anchor {
            Last,
            First,
            After,
            Before,
        };

        Anchor anchor() const;
        QString target() const;
        QString relativeTo() const;

        QVector<ActionLayoutEntry> items() const;

    private:
        const ActionExtensionData *e;
        int i;

        friend class ActionExtension;
        friend class ActionRegistry;
    };

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

#endif // ACTIONEXTENSION_H