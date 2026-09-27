#ifndef GENERATOR_H
#define GENERATOR_H

#include "parser.h"

class Generator {
public:
    inline Generator(FILE *out) : out(out) {}

    // Writes the source file, which defines the function that returns the extension.
    void generate();

    // Writes the header, which declares the function.
    void generateHeader(FILE *header) const;

    FILE *out;
    QString inputFileName;

    QString function;
    QString nameSpace;
    QString exportDirective;
    QString exportFileName;

    // The path by which the source file includes the header, empty if no header is generated.
    QString headerInclude;

    ParseResult parseResult;
};

#endif // GENERATOR_H
