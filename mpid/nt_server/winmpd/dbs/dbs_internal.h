#include "database.h"

typedef struct DatabaseElementStruct
{
    char pszKey[MAX_DBS_KEY_LEN];
    char pszValue[MAX_DBS_VALUE_LEN];
    struct DatabaseElementStruct *pNext;
} DatabaseElement;

typedef struct DatabaseNodeStruct
{
    char pszName[MAX_DBS_NAME_LEN];
    DatabaseElement *pData, *pIter;
    struct DatabaseNodeStruct *pNext;
} DatabaseNode;
