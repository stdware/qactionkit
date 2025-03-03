#include <QtCore/QCommandLineOption>
#include <QtCore/QCommandLineParser>
#include <QtCore/QCoreApplication>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>

#include "parser.h"
#include "generator.h"

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);
    QCoreApplication::setApplicationVersion(QString::fromLatin1(APP_VERSION));

    // Build command line parser
    QCommandLineParser parser;
    parser.setApplicationDescription(
        QStringLiteral("QActionKit Action Extension Compiler version %1 (Qt %2)")
            .arg(QString::fromLatin1(APP_VERSION), QString::fromLatin1(QT_VERSION_STR)));
    parser.setSingleDashWordOptionMode(QCommandLineParser::ParseAsLongOptions);

    QCommandLineOption outputOption(QStringLiteral("o"));
    outputOption.setDescription(QStringLiteral("Write output to file rather than stdout."));
    outputOption.setValueName(QStringLiteral("file"));
    outputOption.setFlags(QCommandLineOption::ShortOptionStyle);
    parser.addOption(outputOption);

    QCommandLineOption identifierOption(QStringLiteral("i"));
    identifierOption.setDescription(
        QStringLiteral("Override extension identifier rather than the file name."));
    identifierOption.setValueName(QStringLiteral("identifier"));
    parser.addOption(identifierOption);

    QCommandLineOption defineOption(QStringLiteral("D"));
    defineOption.setDescription(QStringLiteral("Define a variable."));
    defineOption.setValueName(QStringLiteral("key[=value]"));
    defineOption.setFlags(QCommandLineOption::ShortOptionStyle);
    parser.addOption(defineOption);

    QCommandLineOption textTranslationContextOption(QStringLiteral("text-translation-context"));
    textTranslationContextOption.setDescription(QStringLiteral("Action text translation context."));
    textTranslationContextOption.setValueName(QStringLiteral("context"));
    parser.addOption(textTranslationContextOption);

    QCommandLineOption classTranslationContextOption(QStringLiteral("class-translation-context"));
    classTranslationContextOption.setDescription(
        QStringLiteral("Action class translation context."));
    classTranslationContextOption.setValueName(QStringLiteral("context"));
    parser.addOption(classTranslationContextOption);

    QCommandLineOption descriptionTranslationContextOption(
        QStringLiteral("description-translation-context"));
    descriptionTranslationContextOption.setDescription(
        QStringLiteral("Action description translation context."));
    descriptionTranslationContextOption.setValueName(QStringLiteral("context"));
    parser.addOption(descriptionTranslationContextOption);

    parser.addPositionalArgument(QStringLiteral("<file>"),
                                 QStringLiteral("Manifest file to read from."));

    parser.addHelpOption();
    parser.addVersionOption();

    if (argc == 1) {
        parser.showHelp(0);
    }
    parser.process(QCoreApplication::arguments());

    // Parse command line arguments
    Parser pp;
    QString filename;
    if (const QStringList files = parser.positionalArguments(); files.count() > 1) {
        error(qPrintable(QLatin1String("Too many input files specified: '") +
                         files.join(QLatin1String("' '")) + QLatin1Char('\'')));
        parser.showHelp(1);
    } else if (files.isEmpty()) {
        error(qPrintable(QLatin1String("Input file not specified.")));
        parser.showHelp(1);
    } else {
        filename = files.first();
        pp.fileName = filename;
    }

    for (const QString &arg : parser.values(defineOption)) {
        QString name = arg;
        QString value = name;
        int eq = name.indexOf('=');
        if (eq >= 0) {
            value = name.mid(eq + 1);
            name = name.left(eq);
        }
        if (name.isEmpty()) {
            error("Missing key name");
            parser.showHelp(1);
        }
        pp.variables.insert(name, value);
    }

    QString identifier = parser.value(identifierOption);
    if (identifier.isEmpty()) {
        identifier = QFileInfo(filename).baseName();
    }
    for (auto &ch : identifier) {
        if (!ch.isLetterOrNumber() && ch != '_') {
            ch = '_';
        }
    }
    pp.identifier = identifier;

    QString output = parser.value(outputOption);

    // Parse XML file
    QFile in;
    in.setFileName(filename);
    if (!in.open(QIODevice::ReadOnly)) {
        error("%s: No such file\n", qPrintable(filename));
        return 1;
    }

    // Override configuration with command line options
    auto parseResult = pp.parse(in.readAll());
    if (auto ctx = parser.value(textTranslationContextOption); !ctx.isEmpty()) {
        parseResult.textTranslationContext = ctx;
    }
    if (auto ctx = parser.value(classTranslationContextOption); !ctx.isEmpty()) {
        parseResult.classTranslationContext = ctx;
    }
    if (auto ctx = parser.value(descriptionTranslationContextOption); !ctx.isEmpty()) {
        parseResult.descriptionTranslationContext = ctx;
    }

    // Generate
    FILE *out;
    if (!output.isEmpty()) {
#if defined(_MSC_VER)
        if (_wfopen_s(&out, reinterpret_cast<const wchar_t *>(output.utf16()), L"w") != 0)
#else
        out = fopen(QFile::encodeName(output).constData(), "w"); // create output file
        if (!out)
#endif
        {
            error("Cannot create %s\n", QFile::encodeName(output).constData());
            return 1;
        }
    } else {
        out = stdout;
    }

    Generator generator(out);
    generator.inputFileName = QFileInfo(filename).fileName();
    generator.identifier = identifier;
    generator.parseResult = std::move(parseResult);

    generator.generate();

    if (!output.isEmpty())
        fclose(out);

    return 0;
}