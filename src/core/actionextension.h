#ifndef ACTIONEXTENSION_H
#define ACTIONEXTENSION_H

#include <QtCore/QMap>
#include <QtCore/QStringList>

#include <QAKCore/qakglobal.h>

namespace QAK {

    class ActionScope;

    class ActionItemInfo;

    class ActionLayoutInfo;

    class ActionInsertion;

    class ActionExtension;

    class ActionExtensionData;

    class QAK_CORE_EXPORT ActionItemInfo {
    public:
        ActionItemInfo();
        bool isNull() const;

        enum Type {
            Action,
            Group,
            Menu,
        };

        QString id() const;
        Type type() const;

        QString text() const;
        QString actionClass() const;
        QString description() const;

        QStringList shortcuts() const;
        QStringList catalog() const;

        bool topLevel() const;

        QMap<QString, QString> attributes() const;

    private:
        const ActionExtensionData *e;
        int i;

        friend class ActionExtension;
        friend class ActionScope;
    };

    class QAK_CORE_EXPORT ActionLayoutInfo {
    public:
        ActionLayoutInfo();
        bool isNull() const;

        enum Type {
            Action,
            Separator,
            Stretch,
        };

        QString id() const;
        Type type() const;

        int childCount() const;
        ActionLayoutInfo child(int index) const;

    private:
        const ActionExtensionData *e;
        int i;

        friend class ActionInsertion;
        friend class ActionExtension;
        friend class ActionScope;
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

        int itemCount() const;
        ActionLayoutInfo item(int index) const;

    private:
        const ActionExtensionData *e;
        int i;

        friend class ActionExtension;
        friend class ActionScope;
    };

    class QAK_CORE_EXPORT ActionExtension {
    public:
        QString version() const;

        QString id() const;
        QString hash() const;

        int itemCount() const;
        ActionItemInfo item(int index) const;

        int layoutCount() const;
        ActionLayoutInfo layout(int index) const;

        int insertionCount() const;
        ActionInsertion insertion(int index) const;

        struct Data {
            const ActionExtensionData *data;
        };
        Data d;
    };

}

#endif // ACTIONEXTENSION_H