/*--------------------------------------------------------------------*/
/* fileNode.c                                                         */
/* Author: Roy Mazumder and Roshaan Khalid                            */
/*--------------------------------------------------------------------*/

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "dynarray.h"
#include "fileNode.h"
#include "dirNode.h"

/* A file node in a DT */
struct dirNode
{
    /* the object corresponding to the node's absolute path */
    Path_T path;
    /* this node's parent */
    Dir_T parentDir;
    /* the object containing links to its sub dirs */
    DynArray_T subDirs;
    /* the object containing links to its files */
    DynArray_T files;
};

/*
  Creates a new dir node in the Directory Tree, with path oPPath and
  parent oNParent. Returns an int SUCCESS status and sets *poNResult
  to be the new node if successful. Otherwise, sets *poNResult to NULL
  and returns status:
  * MEMORY_ERROR if memory could not be allocated to complete request
  * CONFLICTING_PATH if oNParent's path is not an ancestor of oPPath
  * NO_SUCH_PATH if oPPath is of depth 0
                 or oNParent's path is not oPPath's direct parent
                 or oNParent is NULL but oPPath is not of depth 1
  * ALREADY_IN_TREE if oNParent already has a child with this path
*/
int Dir_new(Path_T oPPath, Dir_T oNParent, Dir_T *poNResult)
{
    struct dirNode *psNew;
    Path_T oPParentPath = NULL;
    Path_T oPNewPath = NULL;
    size_t ulParentDepth;
    size_t ulIndex;
    int iStatus;

    assert(oPPath != NULL);
    assert(oNParent == NULL);

    /* allocate space for a new node */
    psNew = malloc(sizeof(struct dirNode));
    if (psNew == NULL)
    {
        *poNResult = NULL;
        return MEMORY_ERROR;
    }

    /* set the new node's path */
    iStatus = Path_dup(oPPath, &oPNewPath);
    if (iStatus != SUCCESS)
    {
        free(psNew);
        *poNResult = NULL;
        return iStatus;
    }
    psNew->path = oPNewPath;

    /* validate and set the new node's parent */
    if (oNParent != NULL)
    {
        size_t ulSharedDepth;

        oPParentPath = oNParent->path;
        ulParentDepth = Path_getDepth(oPParentPath);
        ulSharedDepth = Path_getSharedPrefixDepth(psNew->path,
                                                  oPParentPath);
        /* parent must be an ancestor of child */
        if (ulSharedDepth < ulParentDepth)
        {
            Path_free(psNew->path);
            free(psNew);
            *poNResult = NULL;
            return CONFLICTING_PATH;
        }

        /* parent must be exactly one level up from child */
        if (Path_getDepth(psNew->path) != ulParentDepth + 1)
        {
            Path_free(psNew->path);
            free(psNew);
            *poNResult = NULL;
            return NO_SUCH_PATH;
        }

        /* parent must not already have child with this path */
        if (Dir_hasChild(oNParent, oPPath, &ulIndex))
        {
            Path_free(psNew->path);
            free(psNew);
            *poNResult = NULL;
            return ALREADY_IN_TREE;
        }
    }
    else
    {
        /* new node must be root */
        /* can only create one "level" at a time */
        if (Path_getDepth(psNew->path) != 1)
        {
            Path_free(psNew->path);
            free(psNew);
            *poNResult = NULL;
            return NO_SUCH_PATH;
        }
    }
    psNew->parentDir = oNParent;

    /* initialize the new node */
    psNew->subDirs = DynArray_new(0);
    psNew->files = DynArray_new(0);
    if (psNew->subDirs == NULL || psNew->files == NULL)
    {
        Path_free(psNew->path);
        free(psNew);
        *poNResult = NULL;
        return MEMORY_ERROR;
    }

    /* Link into parent's children list */
    if (oNParent != NULL)
    {
        iStatus = Dir_addSubDir(oNParent, psNew, ulIndex);
        if (iStatus != SUCCESS)
        {
            Path_free(psNew->path);
            free(psNew);
            *poNResult = NULL;
            return iStatus;
        }
    }

    *poNResult = psNew;

    return SUCCESS;
}
int Dir_compare(Dir_T oNFirst, Dir_T oNSecond)
{
    assert(oNFirst != NULL);
    assert(oNSecond != NULL);

    return Path_comparePath(oNFirst->path, oNSecond->path);
}
/*
  Destroys and frees all memory allocated for the subtree rooted at
  oNNode, i.e., deletes this node and all its descendents. Returns the
  number of nodes deleted.
*/
size_t Dir_free(Dir_T oNNode)
{
    size_t ulIndex;
    size_t ulCount = 0;

    assert(oNNode != NULL);

    /* remove from parent's list */
    if (oNNode->parentDir != NULL)
    {
        if (DynArray_bsearch(
                oNNode->parentDir->subDirs,
                oNNode, &ulIndex,
                (int (*)(const void *, const void *))Dir_compare))
            (void)DynArray_removeAt(oNNode->parentDir->subDirs,
                                    ulIndex);
    }
    /* remove all files */
    while (DynArray_getLength(oNNode->files) != 0)
    {
        (void)File_free(DynArray_get(oNNode->files, 0));
        ulCount++;
    }
    /* recursively remove subDirs */
    while (DynArray_getLength(oNNode->subDirs) != 0)
    {
        ulCount += Dir_free(DynArray_get(oNNode->subDirs, 0));
    }
    DynArray_free(oNNode->files);
    DynArray_free(oNNode->subDirs);

    /* remove path */
    Path_free(oNNode->path);

    /* finally, free the struct node */
    free(oNNode);
    ulCount++;
    return ulCount;
}

/*-------------------------------------------------------*/
Path_T Dir_getPath(Dir_T oNNode)
{

    assert(oNNode != NULL);

    return oNNode->path;
}

/*-------------------------------------------------------*/
Dir_T Dir_getParent(Dir_T oNNode)
{
    assert(oNNode != NULL);

    return oNNode->parentDir;
}

/*-------------------------------------------------------*/
size_t Dir_getNumSubDirs(Dir_T oNParent)
{
    assert(oNParent != NULL);

    return DynArray_getLength(oNParent->subDirs);
}

/*-------------------------------------------------------*/
int Dir_getSubDir(Dir_T oNParent, size_t ulChildID,
                  Dir_T *poNResult)
{
    assert(oNParent != NULL);
    assert(poNResult != NULL);

    /* ulChildID is the index into oNParent->oDChildren */
    if (ulChildID >= Dir_getNumSubDirs(oNParent))
    {
        *poNResult = NULL;
        return NO_SUCH_PATH;
    }
    else
    {
        *poNResult = DynArray_get(oNParent->subDirs, ulChildID);
        return SUCCESS;
    }
}

/*-------------------------------------------------------*/
size_t Dir_getNumFiles(Dir_T oNParent)
{
    assert(oNParent != NULL);

    return DynArray_getLength(oNParent->files);
}

/*-------------------------------------------------------*/

int Dir_getFile(Dir_T oNParent, size_t ulChildID,
                File_T *poNResult)
{

    assert(oNParent != NULL);
    assert(poNResult != NULL);

    /* ulChildID is the index into oNParent->oDChildren */
    if (ulChildID >= Dir_getNumFiles(oNParent))
    {
        *poNResult = NULL;
        return NO_SUCH_PATH;
    }
    else
    {
        *poNResult = DynArray_get(oNParent->files, ulChildID);
        return SUCCESS;
    }
}
static int Dir_compareString(const Dir_T oNFirst,
                             const char *pcSecond)
{
    assert(oNFirst != NULL);
    assert(pcSecond != NULL);

    return Path_compareString(oNFirst->path, pcSecond);
}
/*
  Links new Dir_T oNChild into oNParent's children array at index
  ulIndex. Returns SUCCESS if the new sub dir child was added successfully,
  or  MEMORY_ERROR if allocation fails adding oNChild to the array.
*/

int Dir_addSubDir(Dir_T oNParent, Dir_T oNChild, size_t ulIndex)
{
    assert(oNParent != NULL);
    assert(oNChild != NULL);

    if (DynArray_addAt(oNParent->subDirs, ulIndex, oNChild))
        return SUCCESS;
    else
        return MEMORY_ERROR;
}
int Dir_addFile(Dir_T oNParent, File_T oNChild, size_t ulIndex)
{
    assert(oNParent != NULL);
    assert(oNChild != NULL);

    if (DynArray_addAt(oNParent->files, ulIndex, oNChild))
        return SUCCESS;
    else
        return MEMORY_ERROR;
}

boolean Dir_hasChild(Dir_T oNParent, Path_T oPPath, size_t *pulChildID)
{

    boolean ret1, ret2;

    assert(oNParent != NULL);
    assert(oPPath != NULL);
    assert(pulChildID != NULL);

    /* *pulChildID is the index into oNParent->oDChildren */
    ret1 = DynArray_bsearch(oNParent->subDirs,
                            (char *)Path_getPathname(oPPath), pulChildID,
                            (int (*)(const void *, const void *))Dir_compareString);
    ret2 = DynArray_bsearch(oNParent->files,
                            (char *)Path_getPathname(oPPath), pulChildID,
                            (int (*)(const void *, const void *))File_compareString);
    return ret1 || ret2;
}

DynArray_T Dir_getFiles(Dir_T oNParent)
{
    assert(oNParent != NULL);

    return oNParent->files;
}