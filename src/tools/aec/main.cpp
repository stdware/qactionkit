#include <QtCore/QCommandLineOption>
#include <QtCore/QCommandLineParser>
#include <QtCore/QCoreApplication>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>

#include "parser.h"
#include "generator.h"

// Returns whether the text is a C++ identifier made of ASCII letters, digits and underscores.
// The check does not depend on the locale.
static bool isIdentifier(const QString &text) {
    if (text.isEmpty()) {
        return false;
    }
    for (qsizetype i = 0; i < text.size(); ++i) {
        const char16_t ch = text.at(i).unicode();
        const bool letter = (ch >= u'a' && ch <= u'z') || (ch >= u'A' && ch <= u'Z') || ch == u'_';
        const bool digit = ch >= u'0' && ch <= u'9';
        if (!letter && !(digit && i > 0)) {
            return false;
        }
    }
    return true;
}

static FILE *openForWriting(const QString &path) {
    FILE *file = nullptr;
#if defined(_MSC_VER)
    if (_wfopen_s(&file, reinterpret_cast<const wchar_t *>(path.utf16()), L"w") != 0) {
        file = nullptr;
    }
#else
    file = fopen(QFile::encodeName(path).constData(), "w");
#endif
    if (!file) {
        error("cannot create %s\n", QFile::encodeName(path).constData());
    }
    return file;
}

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

    QCommandLineOption headerOption(QStringLiteral("header"));
    headerOption.setDescription(
        QStringLiteral("Write the header declaring the function to file. Without this option, "
                       "the source file declares the function itself."));
    headerOption.setValueName(QStringLiteral("file"));
    parser.addOption(headerOption);

    QCommandLineOption functionOption(QStringLiteral("function"));
    functionOption.setDescription(
        QStringLiteral("Name of the function that returns the extension. Required."));
    functionOption.setValueName(QStringLiteral("name"));
    parser.addOption(functionOption);

    QCommandLineOption namespaceOption(QStringLiteral("namespace"));
    namespaceOption.setDescription(
        QStringLiteral("Namespace of the function, such as a::b. Global if omitted."));
    namespaceOption.setValueName(QStringLiteral("namespace"));
    parser.addOption(namespaceOption);

    QCommandLineOption exportDirectiveOption(QStringLiteral("export-directive"));
    exportDirectiveOption.setDescription(
        QStringLiteral("Macro placed before the declaration of the function."));
    exportDirectiveOption.setValueName(QStringLiteral("macro"));
    parser.addOption(exportDirectiveOption);

    QCommandLineOption exportFileNameOption(QStringLiteral("export-file-name"));
    exportFileNameOption.setDescription(
        QStringLiteral("Header that defines the export directive, included with angle brackets."));
    exportFileNameOption.setValueName(QStringLiteral("header"));
    parser.addOption(exportFileNameOption);

    QCommandLineOption defineOption(QStringLiteral("D"));
    defineOption.setDescription(QStringLiteral("Define a variable."));
    defineOption.setValueName(QStringLiteral("key[=value]"));
    defineOption.setFlags(QCommandLineOption::ShortOptionStyle);
    parser.addOption(defineOption);

    QCommandLineOption textTranslationContextOption(QStringLiteral("text-translation-context"));
    textTranslationContextOption.setDescription(QStringLiteral("Action text translation context."));
    textTranslationContextOption.setValueName(QStringLiteral("context"));
    parser.addOption(textTranslationContextOption);

    QCommandLineOption categoryTranslationContextOption(
        QStringLiteral("category-translation-context"));
    categoryTranslationContextOption.setDescription(
        QStringLiteral("Action category translation context."));
    categoryTranslationContextOption.setValueName(QStringLiteral("context"));
    parser.addOption(categoryTranslationContextOption);

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
        error("too many input files are specified: '%s'\n",
              qPrintable(files.join(QLatin1String("' '"))));
        parser.showHelp(1);
    } else if (files.isEmpty()) {
        error("the input file is not specified\n");
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

    const QString function = parser.value(functionOption);
    if (function.isEmpty()) {
        error("the function name is not specified\n");
        return 1;
    }
    if (!isIdentifier(function)) {
        error("the function name \"%s\" is not a valid identifier\n", qPrintable(function));
        return 1;
    }
    const QString nameSpace = parser.value(namespaceOption);
    if (!nameSpace.isEmpty()) {
        for (const auto &segment : nameSpace.split(QStringLiteral("::"))) {
            if (!isIdentifier(segment)) {
                error("the namespace \"%s\" is not valid\n", qPrintable(nameSpace));
                return 1;
            }
        }
    }
    const QString exportDirective = parser.value(exportDirectiveOption);
    const QString exportFileName = parser.value(exportFileNameOption);
    if (!exportFileName.isEmpty() && exportDirective.isEmpty()) {
        error("the export file name \"%s\" is given without an export directive\n",
              qPrintable(exportFileName));
        return 1;
    }
    if (!exportDirective.isEmpty() && !isIdentifier(exportDirective)) {
        error("the export directive \"%s\" is not a valid identifier\n",
              qPrintable(exportDirective));
        return 1;
    }

    QString output = parser.value(outputOption);
    const QString header = parser.value(headerOption);

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
    if (auto ctx = parser.value(categoryTranslationContextOption); !ctx.isEmpty()) {
        parseResult.categoryTranslationContext = ctx;
    }
    if (auto ctx = parser.value(descriptionTranslationContextOption); !ctx.isEmpty()) {
        parseResult.descriptionTranslationContext = ctx;
    }

    // Generate
    FILE *out = stdout;
    if (!output.isEmpty() && !(out = openForWriting(output))) {
        return 1;
    }

    Generator generator(out);
    generator.inputFileName = QFileInfo(filename).fileName();
    generator.function = function;
    generator.nameSpace = nameSpace;
    generator.exportDirective = exportDirective;
    generator.exportFileName = exportFileName;
    if (!header.isEmpty()) {
        // The source file includes the header by its path relative to the source file, so that
        // the include path of the target does not matter.
        const QDir sourceDir = output.isEmpty() ? QDir::current() : QFileInfo(output).absoluteDir();
        generator.headerInclude = sourceDir.relativeFilePath(QFileInfo(header).absoluteFilePath());
    }
    generator.parseResult = std::move(parseResult);

    generator.generate();

    if (!output.isEmpty())
        fclose(out);

    if (!header.isEmpty()) {
        FILE *headerOut = openForWriting(header);
        if (!headerOut) {
            return 1;
        }
        generator.generateHeader(headerOut);
        fclose(headerOut);
    }

    return 0;
}
