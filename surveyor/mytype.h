#include "surveyor.h"

typedef union
{
    char id[32];
    char str[256];
    double value;
    Message message;
    vector<Message>* messages;
    vector<WatchValues>* watchValue;
} MyType;

#define YYSTYPE MyType
