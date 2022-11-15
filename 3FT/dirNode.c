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
#include "checkerDT.h"

/* A file node in a DT */
struct node
{
    /* the object corresponding to the node's absolute path */
    Path_T path;
    /* this node's parent */
    Dir_T parentDir;
    /* the object containing links to its sub dirs */
    DynArray_T subDirs;
    /* the object containing links to its files */
    DynArray_T files;
}

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
int
Dir_new(Path_T oPPath, Dir_T oNParent, Dir_T *poNResult);

/*
  Destroys and frees all memory allocated for the subtree rooted at
  oNNode, i.e., deletes this node and all its descendents. Returns the
  number of nodes deleted.
*/
size_t Dir_free(Dir_T oNNode);

/* Returns the path object representing oNNode's absolute path. */
Path_T Dir_getPath(Dir_T oNNode)
{

    assert(oNNode != NULL);

    return oNNode->path;
}

/*
  Returns a the parent node of oNNode.
  Returns NULL if oNNode is the root and thus has no parent.
*/
Dir_T Dir_getParent(Dir_T oNNode)
{
    assert(oNNode != NULL);

    return oNNode->parentDir;
}

/* Returns the number of children that oNParent has. */
size_t Dir_getNumSubDirs(Dir_T oNParent)
{
    assert(oNParent != NULL);

    return DynArray_getLength(oNParent->subDirs);
}

/*
  Returns an int SUCCESS status and sets *poNResult to be the child
  node of oNParent with identifier ulChildID, if one exists.
  Otherwise, sets *poNResult to NULL and returns status:
  * NO_SUCH_PATH if ulChildID is not a valid child for oNParent
*/
int Dir_getSubDir(Dir_T oNParent, size_t ulChildID,
                  Dir_T *poNResult)
{
    assert(oNParent != NULL);
    assert(poNResult != NULL);

    /* ulChildID is the index into oNParent->oDChildren */
    if (ulChildID >= Node_getNumSubDirs(oNParent))
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

/* Returns the number of children that oNParent has. */
size_t Dir_getNumFiles(Dir_T oNParent)
{
    assert(oNParent != NULL);

    return DynArray_getLength(oNParent->files);
}

/*
  Returns an int SUCCESS status and sets *poNResult to be the child
  node of oNParent with identifier ulChildID, if one exists.
  Otherwise, sets *poNResult to NULL and returns status:
  * NO_SUCH_PATH if ulChildID is not a valid child for oNParent
*/
int Dir_getFile(Dir_T oNParent, size_t ulChildID,
                Dir_T *poNResult)
{

    assert(oNParent != NULL);
    assert(poNResult != NULL);

    /* ulChildID is the index into oNParent->oDChildren */
    if (ulChildID >= Node_getNumFiles(oNParent))
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