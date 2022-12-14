/*--------------------------------------------------------------------*/
/* fileNode.h                                                           */
/* Author: Roy Mazumder & Roshaan Khalid                              */
/*--------------------------------------------------------------------*/


#ifndef FILENODE_INCLUDED
#define FILENODE_INCLUDED


/* A Dir_T is a directory node in a Directory Tree */
typedef struct fileNode *File_T;


#include <stddef.h>
#include "a4def.h"
#include "path.h"
#include "dirNode.h"




/*Compared the first file oNFirst and second file oNSecond lexographically, returns int */
int File_compare(File_T oNFirst, File_T oNSecond);

/*Compares the strings contained by the oNFirst and the pcSecond, returns int.*/
int File_compareString(const File_T oNFirst, const char *pcSecond);


/*Gets the path of oNNode, returns Path_T*/
Path_T File_getPath(File_T oNNode);


/*Gets the parent of oNNode*, and returns as Dir_T/
Dir_T File_getParent(File_T oNNode);
/*
  Creates a new file in the Directory Tree, with path oPPath and
  parent oNParent. Returns an int SUCCESS status and sets *poNResult
  to be the new file if successful. Otherwise, sets *poNResult to NULL
  and returns status:
  * MEMORY_ERROR if memory could not be allocated to complete request
  * CONFLICTING_PATH if oNParent's path is not an ancestor of oPPath
  * NO_SUCH_PATH if oPPath is of depth 0
                 or oNParent's path is not oPPath's direct parent
                 or oNParent is NULL but oPPath is not of depth 1
  * ALREADY_IN_TREE if oNParent already has a file with this path
*/
int File_new(Path_T oPPath, Dir_T oNParent, File_T *poNResult);

/*
  Destroys file represented by oNNode, returns failure or success
*/
int File_free(File_T oNNode);

/* Returns the path object representing oNNode's absolute path as a Path_T. */
Path_T File_getPath(File_T oNNode);

/*
  Returns the parent directory of oNNode as Dir_tT
*/
Dir_T File_getParent(File_T oNNode);


/* sets the contents of oFile according to void * contents and its
length size_t length. Finally returns SUCCESS or FAILURE */
int File_setContents(File_T oFile, void *contents, size_t length);

/* Returns a pointer to the contents of oFile as a void **/
void *File_getContents(File_T oFile);

/*Gets the length of the contents stored by the oFile as a size_t, returns size_t*/
size_t File_getLength(File_T oFile);

#endif
