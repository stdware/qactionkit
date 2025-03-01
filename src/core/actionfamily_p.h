#ifndef ACTIONFAMILY_P_H
#define ACTIONFAMILY_P_H

#include <variant>

#include <QAKCore/actionfamily.h>
#include <QAKCore/private/qakglobal_p.h>

#include <stdcorelib/linked_map.h>

namespace QAK {

    class QAK_CORE_EXPORT ActionFamilyPrivate {
        Q_DECLARE_PUBLIC(ActionFamily)
    public:
        ActionFamilyPrivate();
        virtual ~ActionFamilyPrivate();

        void init();

        ActionFamily *q_ptr;

        // Icons
        struct IconChange {
            struct Single {
                QString theme;
                QString id;
                QString fileName;
                bool remove;
            };
            struct Config {
                QString fileName;
                bool remove;
            };
            struct All {
                int dummy;
            };
            stdc::linked_map<QStringList, std::variant<Single, Config, All>> items;
        };
        struct IconStorage {
            QHash<QString, QHash<QString, QString>> singles; // theme -> [id -> file]
            QHash<QString, QHash<QString, QHash<QString, QString>>>
                configFiles; // configFile -> [theme -> [id -> file]]
            stdc::linked_map<QStringList, int /* NOT USED */> indexes;
            QHash<QString, QHash<QString, QString>> storage;
        };
        mutable IconChange iconChange;
        mutable IconStorage iconStorage;

        ActionFamily::ShortcutsFamily overriddenShortcuts;
        ActionFamily::IconFamily overriddenIcons;

        void flushIcons() const;
    };

}

#endif // ACTIONFAMILY_P_H
