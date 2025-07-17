#ifndef ACTIONICON_H
#define ACTIONICON_H

#include <QtCore/QJsonObject>
#include <QtCore/QSharedData>
#include <QtGui/QIcon>

#include <QAKCore/qakglobal.h>

namespace QAK {

    class ActionIconPrivate;

    class QAK_CORE_EXPORT ActionIcon {
    public:
        ActionIcon();
        ~ActionIcon();

        ActionIcon(const ActionIcon &other);
        ActionIcon(ActionIcon &&other) noexcept;
        ActionIcon &operator=(const ActionIcon &other);
        ActionIcon &operator=(ActionIcon &&other) noexcept;

    public:
        QString filePath(QIcon::Mode mode, QIcon::State state) const;
        void addFile(const QString &filePath, QSize size = {}, QIcon::Mode mode = QIcon::Normal,
                     QIcon::State state = QIcon::Off);

        QIcon icon() const;
        bool isNull() const;

        QJsonValue toJson() const;
        static ActionIcon fromJson(const QJsonValue &json);

    protected:
        QSharedDataPointer<ActionIconPrivate> d_ptr;
    };

}

#endif // ACTIONICON_H