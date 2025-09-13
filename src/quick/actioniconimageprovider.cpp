#include "actioniconimageprovider.h"
#include "actioniconimageprovider_p.h"

#include <QUrlQuery>
#include <QImageReader>

#include <QAKCore/actionfamily.h>

namespace QAK {
    ActionIconImageProvider::ActionIconImageProvider() : QQuickImageProvider(Image), d_ptr(new ActionIconImageProviderPrivate) {
        Q_D(ActionIconImageProvider);
        d->q_ptr = this;
    }
    ActionIconImageProvider::~ActionIconImageProvider() = default;
    ActionFamily *ActionIconImageProvider::actionFamily() const {
        Q_D(const ActionIconImageProvider);
        return d->actionFamily;
    }
    void ActionIconImageProvider::setActionFamily(ActionFamily *actionFamily) {
        Q_D(ActionIconImageProvider);
        d->actionFamily = actionFamily;
    }
    QString ActionIconImageProvider::theme() const {
        Q_D(const ActionIconImageProvider);
        return d->theme;
    }
    void ActionIconImageProvider::setTheme(const QString &theme) {
        Q_D(ActionIconImageProvider);
        d->theme = theme;
    }
    QImage ActionIconImageProvider::requestImage(const QString &id, QSize *size, const QSize &requestedSize) {
        Q_D(const ActionIconImageProvider);
        *size = requestedSize;
        if (!d->actionFamily || id.isEmpty()) {
            return {};
        }
        QUrl url(id);
        auto actionId = url.path();
        auto query = QUrlQuery(url);
        auto enabled = !query.hasQueryItem("enabled") || query.queryItemValue("enabled") == "true";
        auto checked = query.hasQueryItem("checked") && query.queryItemValue("checked") == "true";
        auto actionIcon = d->actionFamily->actionIcon(d->theme, actionId);
        auto filePath = actionIcon.filePath(enabled, checked);
        if (filePath.isEmpty()) {
            filePath = actionIcon.filePath();
        }
        QImageReader reader(filePath);
        reader.setScaledSize(requestedSize);
        return reader.read();
    }
} // QAK