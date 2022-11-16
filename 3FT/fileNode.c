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
#include "path.h"

/* A file node in a DT */
struct fileNode
{
   /* the object corresponding to the node's absolute path */
   Path_T path;
   /* this node's parent */
   Dir_T parentDir;
   /* the pointer to the begining of the contents of the file */
   void *contents;
   /* the lenght of the contents */
   size_t conLen;
};

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
   struct File_T *psNew;
   Path_T oPParentPath = NULL;
   Path_T oPNewPath = NULL;
   size_t ulParentDepth;
   size_t ulIndex;
   int iStatus;

   assert(oPPath != NULL);
   assert(oNParent == NULL);

   /* allocate space for a new node */
   psNew = malloc(sizeof(struct fileNode));
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
   psNew->oPPath = oPNewPath;

   /* validate and set the new node's parent */
   if (oNParent != NULL)
   {
      size_t ulSharedDepth;

      oPParentPath = oNParent->oPPath;
      ulParentDepth = Path_getDepth(oPParentPath);
      ulSharedDepth = Path_getSharedPrefixDepth(psNew->oPPath,
                                                oPParentPath);
      /* parent must be an ancestor of child */
      if (ulSharedDepth < ulParentDepth)
      {
         Path_free(psNew->oPPath);
         free(psNew);
         *poNResult = NULL;
         return CONFLICTING_PATH;
      }

      /* parent must be exactly one level up from child */
      if (Path_getDepth(psNew->oPPath) != ulParentDepth + 1)
      {
         Path_free(psNew->oPPath);
         free(psNew);
         *poNResult = NULL;
         return NO_SUCH_PATH;
      }
      /* parent must not already have child with this path */
        if (Dir_hasChild(oNParent, oPPath, &ulIndex))
        {
            Path_free(psNew->oPPath);
            free(psNew);
            *poNResult = NULL;
            return ALREADY_IN_TREE;
        }
   }
   else
   {
      /* new node must be root */
      /* can only create one "level" at a time */
      if (Path_getDepth(psNew->oPPath) != 1)
      {
         Path_free(psNew->oPPath);
         free(psNew);
         *poNResult = NULL;
         return NO_SUCH_PATH;
      }
   }
   psNew->oNParent = oNParent;

   /* initialize the new node */
   psNew->oDChildren = DynArray_new(0);
   if (psNew->oDChildren == NULL)
   {
      Path_free(psNew->oPPath);
      free(psNew);
      *poNResult = NULL;
      return MEMORY_ERROR;
   }

   /* Link into parent's children list */
   if (oNParent != NULL)
   {
      iStatus = Dir_addFile(oNParent, psNew, ulIndex);
      if (iStatus != SUCCESS)
      {
         Path_free(psNew->oPPath);
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
size_t File_free(File_T oNNode)
{
   size_t ulIndex;
   size_t ulCount = 0;

   assert(oNNode != NULL);

   /* remove from parent's list */
   if (oNNode->oNParent != NULL)
   {
      if (DynArray_bsearch(
              oNNode->oNParent->oDChildren,
              oNNode, &ulIndex,
              (int (*)(const void *, const void *))Node_compare))
         (void)DynArray_removeAt(oNNode->oNParent->oDChildren,
                                 ulIndex);
   }

   /* recursively remove children */
   while (DynArray_getLength(oNNode->oDChildren) != 0)
   {
      ulCount += Node_free(DynArray_removeAt(oNNode->oDChildren, 0));
   }
   DynArray_free(oNNode->oDChildren);

   /* remove path */
   Path_free(oNNode->oPPath);

   /* finally, free the struct node */
   free(oNNode);
   ulCount++;
   return ulCount;
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
Dir_Tree File_getParent(File_T oNNode)
{
   assert(oNNode != NULL);
   return oNNode->parentDir;
}