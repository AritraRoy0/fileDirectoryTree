/*--------------------------------------------------------------------*/
/* fileNode.c                                                         */
/* Author: Roy Mazumder and Roshaan Khalid                            */
/*--------------------------------------------------------------------*/

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "a4def.h"
#include "dynarray.h"
#include "dirNode.h"
#include "fileNode.h"
#include "path.h"

/* A file node in a DT */
struct fileNode
{
   /* the object corresponding to the node's absolute path */
   Path_T path;
   /* this node's parent */
   Dir_T parentDir;
   /* the pointer to the begining of the contents of the file */
   char *contents;
   /* the lenght of the contents */
   size_t conLen;
};

int File_compare(File_T oNFirst, File_T oNSecond)
{
   assert(oNFirst != NULL);
   assert(oNSecond != NULL);

   return Path_comparePath(oNFirst->path, oNSecond->path);
}

int File_compareString(const File_T oNFirst, const char *pcSecond)
{
   assert(oNFirst != NULL);
   assert(pcSecond != NULL);
   return Path_compareString(oNFirst->path, pcSecond);
}

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
int File_new(Path_T oPPath, Dir_T oNParent, File_T *poNResult)
{
   struct fileNode *psNew;
   Path_T oPParentPath = NULL;
   Path_T oPNewPath = NULL;
   size_t ulParentDepth;
   size_t ulIndex;
   int iStatus;

   assert(oPPath != NULL);
   assert(oNParent == NULL);

   /* allocate space for a new node */
   psNew = (fileNode *)calloc(sizeof(struct fileNode));
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

      oPParentPath = Dir_getPath(oNParent);
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

   /* Link into parent's children list */
   if (oNParent != NULL)
   {
      iStatus = Dir_addFile(oNParent, psNew, ulIndex);
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

/*
  Destroys and frees all memory allocated for the subtree rooted at
  oNNode, i.e., deletes this node and all its descendents. Returns the
  number of nodes deleted.
*/
int File_free(File_T oNNode)
{
   size_t ulIndex;
   assert(oNNode != NULL);

   /* remove from parent's list */
   if (oNNode->parentDir != NULL)
   {
      if (DynArray_bsearch(
              Dir_getFiles(oNNode->parentDir),
              oNNode, &ulIndex,
              (int (*)(const void *, const void *))File_compare))
         (void)DynArray_removeAt(Dir_getFiles(oNNode->parentDir),
                                 ulIndex);
   }

   /* remove path */
   Path_free(oNNode->path);

   /* finally, free the struct node */
   free(oNNode);
   return SUCCESS;
}

/* Returns the path object representing oNNode's absolute path. */
Path_T File_getPath(File_T oNNode)
{
   assert(oNNode != NULL);
   return oNNode->path;
}
/*
  Returns a the parent node of oNNode.
  Returns NULL if oNNode is the root and thus has no parent.
*/
Dir_T File_getParent(File_T oNNode)
{
   assert(oNNode != NULL);
   return oNNode->parentDir;
}

/* sets the contents of a file according to void * contents and its
length size_t length. Finally returns SUCCESS or FAILURE */
int File_setContents(File_T oFile, void *contents, size_t length)
{

   assert(oFile != NULL);

   oFile->contents = calloc(length, sizeof(char));
   if (oFile->contents == NULL)
   {
      return MEMORY_ERROR;
   }
   *(oFile->contents) = *((char *)contents);
   oFile->conLen = length;
   return SUCCESS;
}

/* Returns a pointer to the contents of a file */
void *File_getContents(File_T oFile)
{
   assert(oFile != NULL);

   return (void *)oFile->contents;
}

/* Replaces contents of a file with new content and
Returns a pointer to the new contents of a file */
void *File_replaceContents(File_T oFile, void *newContents, size_t newLength)
{
   assert(oFile != NULL);

   oFile->contents = calloc(newLength, sizeof(char));
   if (oFile->contents == NULL)
   {
      return NULL;
   }
   oFile->contents = newContents;
   oFile->conLen = newLength;
   return SUCCESS;
} /* this is buggy */

size_t File_getLength(File_T oFile)
{
   assert(oFile != NULL);

   return oFile->conLen;
}
