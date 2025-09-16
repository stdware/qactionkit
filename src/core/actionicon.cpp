#include "actionicon.h"

#include <QtCore/QJsonArray>
#include <QtCore/QFileInfo>

#include <util/util.h>

namespace QAK {

    class ActionIconPrivate : public QSharedData {
    public:
        QIcon icon;
        QString currentColor;

        struct AddFileInfo {
            QString filePath;
            QSize size;
        };

        AddFileInfo files[2][2]; // [enabled][checked]
    };

    ActionIcon::ActionIcon() : d_ptr(new ActionIconPrivate()) {
    }

    ActionIcon::~ActionIcon() = default;
    ActionIcon::ActionIcon(const ActionIcon &other) = default;
    ActionIcon::ActionIcon(ActionIcon &&other) noexcept = default;
    ActionIcon &ActionIcon::operator=(const ActionIcon &other) = default;
    ActionIcon &ActionIcon::operator=(ActionIcon &&other) noexcept = default;

    QString ActionIcon::filePath(bool enabled, bool checked) const {
        return d_ptr->files[enabled][checked].filePath;
    }

    QSize ActionIcon::size(bool enabled, bool checked) const {
        return d_ptr->files[enabled][checked].size;
    }

    void ActionIcon::addFile(const QString &filePath, QSize size, bool enabled, bool checked) {
        auto &d = *d_ptr;
        d.files[enabled][checked] = {filePath, size};

        if (!checked) {
            // set unchecked-enabled icon if not set
            if (!enabled && d.files[true][false].filePath.isEmpty()) {
                d.files[true][false] = {filePath, size};
            }
        } else {
            // set unchecked-enabled icon if not set
            if (d.files[true][false].filePath.isEmpty()) {
                d.files[true][false] = {filePath, size};
            }

            // set checked-disabled icon if not set
            if (!enabled && d.files[true][true].filePath.isEmpty()) {
                d.files[false][true] = {filePath, size};
            }
        }
        d.icon.addFile(filePath, size, enabled ? QIcon::Normal : QIcon::Disabled,
                       checked ? QIcon::On : QIcon::Off);
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
        auto &d = *d_ptr;

        QJsonObject obj;
        if (!d.currentColor.isEmpty()) {
            obj["currentColor"] = d.currentColor;
        }

        const auto &toObject = [](const ActionIconPrivate::AddFileInfo &info) {
            QJsonObject obj;
            obj["path"] = info.filePath;
            if (!info.size.isEmpty()) {
                QJsonObject sizeObj;
                sizeObj["width"] = info.size.width();
                sizeObj["height"] = info.size.height();
                obj["size"] = sizeObj;
            }
            return obj;
        };

        QJsonObject uncheckedObj;
        if (auto &item = d.files[true][false]; !item.filePath.isEmpty()) {
            uncheckedObj["enabled"] = toObject(item);
        }
        if (auto &item = d.files[false][false]; !item.filePath.isEmpty()) {
            uncheckedObj["disabled"] = toObject(item);
        }
        if (!uncheckedObj.isEmpty()) {
            obj["unchecked"] = uncheckedObj;
        }

        QJsonObject checkedObj;
        if (auto &item = d.files[true][true]; !item.filePath.isEmpty()) {
            checkedObj["enabled"] = toObject(item);
        }
        if (auto &item = d.files[false][true]; !item.filePath.isEmpty()) {
            checkedObj["disabled"] = toObject(item);
        }
        if (!checkedObj.isEmpty()) {
            obj["checked"] = checkedObj;
        }
        return obj;
    }

    static QSize sizeFromJson(const QJsonValue &json) {
        if (json.isObject()) {
            const auto &obj = json.toObject();
            if (auto it = obj.find("width"); it != obj.end() && it.value().isDouble()) {
                if (auto it2 = obj.find("height"); it2 != obj.end() && it2.value().isDouble()) {
                    return {it.value().toInt(), it2.value().toInt()};
                }
            }
        }
        return {};
    }

    ActionIcon ActionIconFromJson(const QJsonValue &json, const QString &baseDir) {
        if (json.isString()) {
            // path
            return ActionIcon(Util::absolutePath(json.toString(), baseDir));
        }

        if (json.isObject()) {
            ActionIcon ai;

            /*
                {
                    "path": "path/to/file.png",
                    "size": {
                        "width": 16,
                        "height": 16
                    }
                }
            */
            const auto testOne = [&](const QJsonObject &obj, bool enabled, bool checked) {
                if (auto it = obj.find("path"); it != obj.end() && it->isString()) {
                    QString path = Util::absolutePath(it.value().toString(), baseDir);
                    QSize size;
                    if (it = obj.find("size"); it != obj.end()) {
                        size = sizeFromJson(it.value());
                    }
                    ai.addFile(path, size, enabled, checked);
                    return true;
                }
                return false;
            };

            const auto &obj = json.toObject();
            if (auto it = obj.find("currentColor"); it != obj.end() && it->isString()) {
                ai.setCurrentColor(it.value().toString());
            }

            // only a path and size
            if (testOne(obj, true, false)) {
                return ai;
            }

            const auto &testState = [&](const QJsonValue &value, bool checked) {
                if (value.isString()) {
                    ai.addFile(Util::absolutePath(value.toString(), baseDir), {}, true, checked);
                    return;
                }

                if (value.isObject()) {
                    const auto &obj2 = value.toObject();
                    if (testOne(obj2, true, checked)) {
                        return;
                    }

                    if (auto it = obj2.find("enabled"); it != obj2.end()) {
                        if (it->isString()) {
                            ai.addFile(Util::absolutePath(it.value().toString(), baseDir), {}, true,
                                       checked);
                        } else if (it->isObject()) {
                            const auto &obj3 = it->toObject();
                            std::ignore = testOne(obj3, true, checked);
                        }
                    }

                    if (auto it2 = obj2.find("disabled"); it2 != obj2.end()) {
                        if (it2->isString()) {
                            ai.addFile(Util::absolutePath(it2.value().toString(), baseDir), {},
                                       false, checked);
                        } else if (it2->isObject()) {
                            const auto &obj3 = it2->toObject();
                            std::ignore = testOne(obj3, false, checked);
                        }
                    }
                }
            };

            // checked
            if (auto it = obj.find("unchecked"); it != obj.end()) {
                testState(it.value(), false);
            }

            // checked
            if (auto it = obj.find("checked"); it != obj.end()) {
                testState(it.value(), true);
            }
            return ai;
        }
        return {};
    }

    ActionIcon ActionIcon::fromJson(const QJsonValue &json) {
        return ActionIconFromJson(json, {});
    }

}
