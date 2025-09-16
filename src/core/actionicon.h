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
        inline ActionIcon(const QString &filePath, QSize size = {});
        ~ActionIcon();

        ActionIcon(const ActionIcon &other);
        ActionIcon(ActionIcon &&other) noexcept;
        ActionIcon &operator=(const ActionIcon &other);
        ActionIcon &operator=(ActionIcon &&other) noexcept;

    public:
        QString filePath(bool enabled = true, bool checked = false) const;
        QSize size(bool enabled = true, bool checked = false) const;

        void addFile(const QString &filePath, QSize size = {}, bool enabled = true,
                     bool checked = false);

        QIcon icon() const;
        bool isNull() const;

        QString currentColor() const;
        void setCurrentColor(const QString &color);

        QJsonValue toJson() const;
        static ActionIcon fromJson(const QJsonValue &json);

    protected:
        QSharedDataPointer<ActionIconPrivate> d_ptr;
    };

    inline ActionIcon::ActionIcon(const QString &filePath, QSize size) : ActionIcon() {
        addFile(filePath, size);
    }

}

#endif // ACTIONICON_H
