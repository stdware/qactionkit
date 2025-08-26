#ifndef ACTIONFAMILY_P_H
#define ACTIONFAMILY_P_H

//
//  W A R N I N G !!!
//  -----------------
//
// This file is not part of the QActionKit API. It is used purely as an
// implementation detail. This header file may change from version to
// version without notice, or may even be removed.
//

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
                ActionIcon icon;
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
            QHash<QString, QHash<QString, ActionIcon>> singles; // theme -> [id -> icon]
            QHash<QString, QHash<QString, QHash<QString, ActionIcon>>>
                configFiles; // configFile -> [theme -> [id -> icon]]
            QHash<QString, QHash<QString, ActionIcon>> storage; // theme -> [id -> icon]
        };
        mutable IconChange iconChange;
        mutable IconStorage iconStorage;

        ActionFamily::ShortcutsFamily overriddenShortcuts;
        ActionFamily::IconFamily overriddenIcons;

        void flushIcons() const;
    };

}

#endif // ACTIONFAMILY_P_H
