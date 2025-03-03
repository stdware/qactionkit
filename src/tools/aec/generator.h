#ifndef GENERATOR_H
#define GENERATOR_H

#include "parser.h"

class Generator {
public:
    Generator(FILE *out, const QString &inputFileName, const QString &identifier,
              const ParseResult &parseResult);
    void generate();

    FILE *out;
    QString inputFileName;
    QString identifier;
    ParseResult parseResult;
};

#endif // GENERATOR_H
