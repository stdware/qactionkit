#include "actionicon.h"

namespace QAK {

    class ActionIconPrivate : public QSharedData {
    public:
        QIcon icon;
        QString currentColor;

        struct AddFileInfo {
            QString filePath;
            QSize size;
        };

        AddFileInfo files[QIcon::Selected + 1][QIcon::Off + 1]; // [mode][state]
    };

    ActionIcon::ActionIcon() : d_ptr(new ActionIconPrivate()) {
    }

    ActionIcon::~ActionIcon() = default;
    ActionIcon::ActionIcon(const ActionIcon &other) = default;
    ActionIcon::ActionIcon(ActionIcon &&other) noexcept = default;
    ActionIcon &ActionIcon::operator=(const ActionIcon &other) = default;
    ActionIcon &ActionIcon::operator=(ActionIcon &&other) noexcept = default;

    QString ActionIcon::filePath(QIcon::Mode mode, QIcon::State state) const {
        return d_ptr->files[mode][state].filePath;
    }

    void ActionIcon::addFile(const QString &filePath, QSize size, QIcon::Mode mode,
                             QIcon::State state) {
        d_ptr->files[mode][state] = {filePath, size};
        d_ptr->icon.addFile(filePath, {}, mode, state);
    }

    QIcon ActionIcon::icon() const {
        return d_ptr->icon;
    }

    bool ActionIcon::isNull() const {
        return d_ptr->icon.isNull();
    }

    QString ActionIcon::currentColor() const {
        return d_ptr->currentColor;
    }

    void ActionIcon::setCurrentColor(const QString &color) {
        d_ptr->currentColor = color;
    }

    QJsonValue ActionIcon::toJson() const {
        // TODO
        return {};
    }

    ActionIcon ActionIcon::fromJson(const QJsonValue &json) {
        // TODO
        return {};
    }

}