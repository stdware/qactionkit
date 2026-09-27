#ifndef PARSER_H
#define PARSER_H

#include <QtCore/QHash>
#include <QtCore/QMap>

#include <QAKCore/actionextension.h>

void error(const char *fmt, ...) QACTIONKIT_PRINTF_FORMAT(1, 2);

struct ActionLayoutEntryMessage {
    QString id;
    QAK::ActionLayoutEntry::Type type;
};

struct ActionItemInfoMessage {
    QString id;
    QString rawId;
    QAK::ActionItemInfo::Type type;

    QString text;
    QString category;
    QString description;

    // Translation contexts written on the item, each empty if that of the extension applies
    QString textContext;
    QString categoryContext;
    QString descriptionContext;

    QString icon;
    QStringList shortcutTokens;
    QString catalog;

    bool topLevel = false;
    bool external = false;

    QMap<QAK::ActionAttributeKey, QString> attributes;

    QVector<ActionLayoutEntryMessage> children;

    // Metadata
    QString tag;
};

struct ActionInsertionMessage {
    QAK::ActionInsertion::Anchor anchor;
    QString target;
    QString relativeTo;
    QVector<ActionLayoutEntryMessage> items;
};

struct ActionExtensionMessage {
    QString version;

    QString id;
    QString hash;

    QVector<ActionItemInfoMessage> items;
    QVector<ActionInsertionMessage> insertions;
};

struct ParseResult {
    QString textTranslationContext;
    QString categoryTranslationContext;
    QString descriptionTranslationContext;

    ActionExtensionMessage extension;
};

class Parser {
public:
    inline Parser() = default;
    ParseResult parse(const QByteArray &data);

    QString fileName;
    QHash<QString, QString> variables;
};

#endif // PARSER_H
