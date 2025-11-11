#include "actionicon.h"

#include <QtCore/QJsonArray>
#include <QtCore/QFileInfo>

#include <util/util.h>

namespace QAK {

    static inline bool isEffectiveUrl(const QUrl &url) {
        return url.isValid() && !url.isEmpty();
    }

    class ActionIconPrivate : public QSharedData {
    public:
        QIcon icon;
        QString currentColor;

        struct AddFileInfo {
            QUrl url;
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

    QUrl ActionIcon::url(bool enabled, bool checked) const {
        return d_ptr->files[enabled][checked].url;
    }

    QSize ActionIcon::size(bool enabled, bool checked) const {
        return d_ptr->files[enabled][checked].size;
    }

    void ActionIcon::addUrl(const QUrl &url, QSize size, bool enabled, bool checked) {
        auto &d = *d_ptr;
        d.files[enabled][checked] = {url, size};

        if (!checked) {
            // set unchecked-enabled icon if not set
            if (!enabled && !isEffectiveUrl(d.files[true][false].url)) {
                d.files[true][false] = {url, size};
            }
        } else {
            // set unchecked-enabled icon if not set
            if (!isEffectiveUrl(d.files[true][false].url)) {
                d.files[true][false] = {url, size};
            }

            // set checked-disabled icon if not set
            if (!enabled && !isEffectiveUrl(d.files[false][true].url)) {
                d.files[false][true] = {url, size};
            }
        }
        if (url.isLocalFile())
            d.icon.addFile(url.toLocalFile(), size, enabled ? QIcon::Normal : QIcon::Disabled,
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
            obj["url"] = info.url.toString();
            if (!info.size.isEmpty()) {
                QJsonObject sizeObj;
                sizeObj["width"] = info.size.width();
                sizeObj["height"] = info.size.height();
                obj["size"] = sizeObj;
            }
            return obj;
        };

        QJsonObject uncheckedObj;
        if (auto &item = d.files[true][false]; isEffectiveUrl(item.url)) {
            uncheckedObj["enabled"] = toObject(item);
        }
        if (auto &item = d.files[false][false]; isEffectiveUrl(item.url)) {
            uncheckedObj["disabled"] = toObject(item);
        }
        if (!uncheckedObj.isEmpty()) {
            obj["unchecked"] = uncheckedObj;
        }

        QJsonObject checkedObj;
        if (auto &item = d.files[true][true]; isEffectiveUrl(item.url)) {
            checkedObj["enabled"] = toObject(item);
        }
        if (auto &item = d.files[false][true]; isEffectiveUrl(item.url)) {
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

    ActionIcon ActionIconFromJson(const QJsonValue &json, const QUrl &baseUrl) {
        if (json.isString()) {
            return ActionIcon(Util::absoluteUrl(json.toString(), baseUrl));
        }

        if (json.isObject()) {
            ActionIcon ai;

            /*
                {
                    "url": "file://C:/path/to/file.png",
                    "size": {
                        "width": 16,
                        "height": 16
                    }
                }
            */
            const auto testOne = [&](const QJsonObject &obj, bool enabled, bool checked) {
                if (auto it = obj.find("url"); it != obj.end() && it->isString()) {
                    QUrl url = Util::absoluteUrl(it.value().toString(), baseUrl);
                    QSize size;
                    if (it = obj.find("size"); it != obj.end()) {
                        size = sizeFromJson(it.value());
                    }
                    ai.addUrl(url, size, enabled, checked);
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
                    ai.addUrl(Util::absoluteUrl(value.toString(), baseUrl), {}, true, checked);
                    return;
                }

                if (value.isObject()) {
                    const auto &obj2 = value.toObject();
                    if (testOne(obj2, true, checked)) {
                        return;
                    }

                    if (auto it = obj2.find("enabled"); it != obj2.end()) {
                        if (it->isString()) {
                            ai.addUrl(Util::absoluteUrl(it.value().toString(), baseUrl), {}, true,
                                      checked);
                        } else if (it->isObject()) {
                            const auto &obj3 = it->toObject();
                            std::ignore = testOne(obj3, true, checked);
                        }
                    }

                    if (auto it = obj2.find("disabled"); it != obj2.end()) {
                        if (it->isString()) {
                            ai.addUrl(Util::absoluteUrl(it.value().toString(), baseUrl), {}, false,
                                      checked);
                        } else if (it->isObject()) {
                            const auto &obj3 = it->toObject();
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
