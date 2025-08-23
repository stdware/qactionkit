#ifndef QMXMLADAPTOR_H
#define QMXMLADAPTOR_H

#include <QJsonObject>
#include <QXmlStreamWriter>

struct QMXmlAdaptorAttributeKey {
    QString name;
    QString namespaceUri;

    QMXmlAdaptorAttributeKey() = default;
    QMXmlAdaptorAttributeKey(const QString &name, const QString &namespaceUri = QString())
        : name(name), namespaceUri(namespaceUri) {}

    bool operator==(const QMXmlAdaptorAttributeKey &other) const {
        return name == other.name && namespaceUri == other.namespaceUri;
    }

    bool operator!=(const QMXmlAdaptorAttributeKey &other) const {
        return !(*this == other);
    }

    bool operator<(const QMXmlAdaptorAttributeKey &other) const {
        if (name != other.name)
            return name < other.name;
        return namespaceUri < other.namespaceUri;
    }
};

class QMXmlAdaptorElement {
public:
    using Ref = QSharedPointer<QMXmlAdaptorElement>;

    QString name;                      // Tag name
    QString namespaceUri;              // Element namespace URI
    QMap<QMXmlAdaptorAttributeKey, QString> properties; // Tag properties with namespace (name, namespaceUri) -> value
    QString value;                     // Characters if no children
    QList<Ref> children;               // Children

    QMXmlAdaptorElement(){};
    ~QMXmlAdaptorElement() = default;

    QJsonObject toObject() const;
    static QMXmlAdaptorElement fromObject(const QJsonObject &obj);

    void writeXml(QXmlStreamWriter &writer) const;
};

class QMXmlAdaptor {
public:
    QMXmlAdaptor();
    ~QMXmlAdaptor();

public:
    bool load(const QString &filename);
    bool save(const QString &filename) const;

    bool loadData(const QByteArray &data);
    QByteArray saveData() const;

    QMXmlAdaptorElement root;
};

// Example

/*

    {
        "name": "document",
        "namespaceUri": "http://example.com/ns",
        "properties": [
            {"name": "attr1", "namespace": "http://example.com/ns", "value": "value1"},
            {"name": "attr2", "namespace": "", "value": "value2"}
        ],
        "value": "...",
        "children": [
            ...
        ]
    }

*/

#endif // QMXMLADAPTOR_H
