#include "dbs_internal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

DatabaseNode *g_pDatabase = NULL, *g_Iter = NULL;
int g_nNextAvailableID = 0;

int dbs_init()
{
    return DBS_SUCCESS;
}

int dbs_finalize()
{
    DatabaseNode *pNode, *pNext;
    DatabaseElement *pElement;

    pNode = g_pDatabase;
    while (pNode)
    {
	pNext = pNode->pNext;

	while (pNode->pData)
	{
	    pElement = pNode->pData;
	    pNode->pData = pNode->pData->pNext;
	    free(pElement);
	}
	free(pNode);
	
	pNode = pNext;
    }

    g_pDatabase = NULL;
    g_Iter = NULL;

    return DBS_SUCCESS;
}

int dbs_create(char *name)
{
    DatabaseNode *pNode, *pNodeTest;

    /* Lock */

    pNode = g_pDatabase;
    if (pNode)
    {
	while (pNode->pNext)
	    pNode = pNode->pNext;
	pNode->pNext = (DatabaseNode*)malloc(sizeof(DatabaseNode));
	pNode = pNode->pNext;
    }
    else
    {
	g_pDatabase = (DatabaseNode*)malloc(sizeof(DatabaseNode));
	pNode = g_pDatabase;
    }
    pNode->pNext = NULL;
    pNode->pData = NULL;
    pNode->pIter = NULL;
    do
    {
	sprintf(pNode->pszName, "%d", g_nNextAvailableID);
	g_nNextAvailableID++;
	pNodeTest = g_pDatabase;
	while (strcmp(pNodeTest->pszName, pNode->pszName) != 0)
	    pNodeTest = pNodeTest->pNext;
    } while (pNodeTest != pNode);
    strcpy(name, pNode->pszName);

    /* Unlock */

    return DBS_SUCCESS;
}

int dbs_create_name_in(char *name)
{
    DatabaseNode *pNode;

    if (strlen(name) < 1 || strlen(name) > MAX_DBS_NAME_LEN)
	return DBS_FAIL;

    /* Lock */

    /* Check if the name already exists */
    pNode = g_pDatabase;
    while (pNode)
    {
	if (strcmp(pNode->pszName, name) == 0)
	{
	    /* Unlock */
	    /*return DBS_FAIL;*/
	    /* Empty database? */
	    return DBS_SUCCESS;
	}
	pNode = pNode->pNext;
    }

    pNode = g_pDatabase;
    if (pNode)
    {
	while (pNode->pNext)
	    pNode = pNode->pNext;
	pNode->pNext = (DatabaseNode*)malloc(sizeof(DatabaseNode));
	pNode = pNode->pNext;
    }
    else
    {
	g_pDatabase = (DatabaseNode*)malloc(sizeof(DatabaseNode));
	pNode = g_pDatabase;
    }
    pNode->pNext = NULL;
    pNode->pData = NULL;
    pNode->pIter = NULL;
    strcpy(pNode->pszName, name);
    
    /* Unlock */

    return DBS_SUCCESS;
}

int dbs_get(char *name, char *key, char *value)
{
    DatabaseNode *pNode;
    DatabaseElement *pElement;

    pNode = g_pDatabase;
    while (pNode)
    {
	if (strcmp(pNode->pszName, name) == 0)
	{
	    pElement = pNode->pData;
	    while (pElement)
	    {
		if (strcmp(pElement->pszKey, key) == 0)
		{
		    strcpy(value, pElement->pszValue);
		    return DBS_SUCCESS;
		}
		pElement = pElement->pNext;
	    }
	}
	pNode = pNode->pNext;
    }

    return DBS_FAIL;
}

int dbs_put(char *name, char *key, char *value)
{
    DatabaseNode *pNode;
    DatabaseElement *pElement;

    pNode = g_pDatabase;
    while (pNode)
    {
	if (strcmp(pNode->pszName, name) == 0)
	{
	    pElement = pNode->pData;
	    while (pElement)
	    {
		if (strcmp(pElement->pszKey, key) == 0)
		{
		    strcpy(pElement->pszValue, value);
		    return DBS_SUCCESS;
		}
		pElement = pElement->pNext;
	    }
	    pElement = (DatabaseElement*)malloc(sizeof(DatabaseElement));
	    pElement->pNext = pNode->pData;
	    strcpy(pElement->pszKey, key);
	    strcpy(pElement->pszValue, value);
	    pNode->pData = pElement;
	    return DBS_SUCCESS;
	}
	pNode = pNode->pNext;
    }

    return DBS_FAIL;
}

int dbs_delete(char *name, char *key)
{
    DatabaseNode *pNode;
    DatabaseElement *pElement, *pElementTrailer;

    pNode = g_pDatabase;
    while (pNode)
    {
	if (strcmp(pNode->pszName, name) == 0)
	{
	    pElementTrailer = pElement = pNode->pData;
	    while (pElement)
	    {
		if (strcmp(pElement->pszKey, key) == 0)
		{
		    if (pElementTrailer != pElement)
			pElementTrailer->pNext = pElement->pNext;
		    else
			pNode->pData = pElement->pNext;
		    free(pElement);
		    return DBS_SUCCESS;
		}
		pElementTrailer = pElement;
		pElement = pElement->pNext;
	    }
	    return DBS_FAIL;
	}
	pNode = pNode->pNext;
    }

    return DBS_FAIL;
}

int dbs_destroy(char *name)
{
    DatabaseNode *pNode, *pNodeTrailer;
    DatabaseElement *pElement;

    pNodeTrailer = pNode = g_pDatabase;
    while (pNode)
    {
	if (strcmp(pNode->pszName, name) == 0)
	{
	    while (pNode->pData)
	    {
		pElement = pNode->pData;
		pNode->pData = pNode->pData->pNext;
		free(pElement);
	    }
	    if (pNodeTrailer == pNode)
		g_pDatabase = g_pDatabase->pNext;
	    else
		pNodeTrailer->pNext = pNode->pNext;
	    free(pNode);
	    return DBS_SUCCESS;
	}
	pNodeTrailer = pNode;
	pNode = pNode->pNext;
    }

    return DBS_FAIL;
}

int dbs_first(char *name, char *key, char *value)
{
    DatabaseNode *pNode;

    if (key != NULL)
	key[0] = '\0';
    pNode = g_pDatabase;
    while (pNode)
    {
	if (strcmp(pNode->pszName, name) == 0)
	{
	    if (key != NULL)
	    {
		if (pNode->pData)
		{
		    strcpy(key, pNode->pData->pszKey);
		    strcpy(value, pNode->pData->pszValue);
		    pNode->pIter = pNode->pData->pNext;
		}
		else
		    key[0] = '\0';
	    }
	    else
	    {
		pNode->pIter = pNode->pData;
	    }
	    return DBS_SUCCESS;
	}
	pNode = pNode->pNext;
    }

    return DBS_FAIL;
}

int dbs_next(char *name, char *key, char *value)
{
    DatabaseNode *pNode;

    key[0] = '\0';
    pNode = g_pDatabase;
    while (pNode)
    {
	if (strcmp(pNode->pszName, name) == 0)
	{
	    if (pNode->pIter)
	    {
		strcpy(key, pNode->pIter->pszKey);
		strcpy(value, pNode->pIter->pszValue);
		pNode->pIter = pNode->pIter->pNext;
	    }
	    else
		key[0] = '\0';
	    return DBS_SUCCESS;
	}
	pNode = pNode->pNext;
    }

    return DBS_FAIL;
}

int dbs_firstdb(char *name)
{
    g_Iter = g_pDatabase;
    if (name != NULL)
    {
	if (g_Iter)
	    strcpy(name, g_Iter->pszName);
	else
	    name[0] = '\0';
    }
    return DBS_SUCCESS;
}

int dbs_nextdb(char *name)
{
    if (g_Iter == NULL)
	name[0] = '\0';
    else
    {
	g_Iter = g_Iter->pNext;
	if (g_Iter)
	    strcpy(name, g_Iter->pszName);
	else
	    name[0] = '\0';
    }
    return DBS_SUCCESS;
}
