/*--------------------------------------------------------------------*/
/* fileNode.h                                                           */
/* Author: Roy Mazumder & Roshaan Khalid                              */
/*--------------------------------------------------------------------*/


#ifndef NODE_INCLUDED
#define NODE_INCLUDED

#include <stddef.h>
#include "a4def.h"
#include "path.h"
#include <dirNode.h>


/* A Dir_T is a directory node in a Directory Tree */
typedef struct fileNode *File_T;



/* Please Implement the following functions @ Roshaan */


/*
  Creates a new node in the Directory Tree, with path oPPath and
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
int File_new(Path_T oPPath, Dir_T oNParent, File_T *poNResult);

/*
  Destroys and frees all memory allocated for the subtree rooted at
  oNNode, i.e., deletes this node and all its descendents. Returns the
  number of nodes deleted.
*/
int File_free(File_T oNNode);

/* Returns the path object representing oNNode's absolute path. */
Path_T File_getPath(File_T oNNode);

/*
  Returns a the parent node of oNNode.
  Returns NULL if oNNode is the root and thus has no parent.
*/
Dir_Tree File_getParent(File_T oNNode);



/*        Implement Together                        */

/* Returns a pointer to the contents of a file */
void *File_getContents(File_T oFile);

/* Replaces contents of a file with new content and 
Returns a pointer to the new contents of a file */
void *File_replaceContents(File_T oFile, void *newContents, size_t newLength);



#endif
