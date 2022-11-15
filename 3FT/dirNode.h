/*--------------------------------------------------------------------*/
/* dirNode.h                                                           */
/* Author: Roy Mazumder & Roshaan Khalid                              */
/*--------------------------------------------------------------------*/


#ifndef NODE_INCLUDED
#define NODE_INCLUDED

#include <stddef.h>
#include "a4def.h"
#include "path.h"


/* A Dir_T is a directory node in a Directory Tree */
typedef struct dirNode *Dir_T;

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
int Dir_new(Path_T oPPath, Dir_T oNParent, Dir_T *poNResult);


/*
  Destroys and frees all memory allocated for the subtree rooted at
  oNNode, i.e., deletes this node and all its descendents. Returns the
  number of nodes deleted.
*/
size_t Dir_free(Dir_T oNNode);

/* Returns the path object representing oNNode's absolute path. */
Path_T Dir_getPath(Dir_T oNNode);

/* Returns the number of children that oNParent has. */
size_t Dir_getNumChildren(Dir_T oNParent);

/*
  Returns an int SUCCESS status and sets *poNResult to be the child
  node of oNParent with identifier ulChildID, if one exists.
  Otherwise, sets *poNResult to NULL and returns status:
  * NO_SUCH_PATH if ulChildID is not a valid child for oNParent
*/
int Dir_getChild(Dir_T oNParent, size_t ulChildID,
                  Dir_T *poNResult);

/*
  Returns a the parent node of oNNode.
  Returns NULL if oNNode is the root and thus has no parent.
*/
Dir_T Dir_getParent(Dir_T oNNode);


#endif