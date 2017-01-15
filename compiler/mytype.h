#include "analyzer.h"
#include <vector>

typedef union
{
    char id[32];
    char str[32];
    struct { char str1[32];
             char str2[32];
    } userList;
    std::vector<RegExpRule>* processList;
    ProcessGroup* processGroup;
    std::vector<ProcessGroup>* processGroupList;
    WorkLoad* workLoad;
} MyType;

#define YYSTYPE MyType
