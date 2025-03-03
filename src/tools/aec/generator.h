#ifndef GENERATOR_H
#define GENERATOR_H

#include "parser.h"

class Generator {
public:
    inline Generator(FILE *out) : out(out) {}
    void generate();

    FILE *out;
    QString inputFileName;
    QString identifier;
    ParseResult parseResult;
};

#endif // GENERATOR_H
