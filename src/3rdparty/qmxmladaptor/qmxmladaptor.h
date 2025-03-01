#ifndef QMXMLADAPTOR_H
#define QMXMLADAPTOR_H

#include <QJsonObject>
#include <QXmlStreamWriter>

class QMXmlAdaptorElement {
public:
    using Ref = QSharedPointer<QMXmlAdaptorElement>;

    QString name;                      // Tag name
    QMap<QString, QString> properties; // Tag properties
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
        "properties": {
            "key1": "val1",
            ...
        },
        "value": "...",
        "children": [
            ...
        ]
    }

*/

#endif // QMXMLADAPTOR_H
