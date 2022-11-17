/*--------------------------------------------------------------------*/
/* ft.c                                                               */
/* Author: Roy Mazumder and Rooshan Khalid                            */
/*--------------------------------------------------------------------*/

#include <stddef.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "ft.h"
#include "dynarray.h"
#include "path.h"
#include "fileNode.h"
#include "dirNode.h"

/*
  A File Tree is a representation of a hierarchy of directories and files,
  represented as an AO with 3 state variables:
*/

/* 1. a flag for being in an initialized state (TRUE) or not (FALSE) */
static boolean bIsInitialized;
/* 2. a pointer to the root node in the hierarchy */
static Dir_T oNRoot;
/* 3. a counter of the number of nodes in the hierarchy */
static size_t ulCount;

/* --------------------------------------------------------------------

  The DT_traversePath and DT_findNode functions modularize the common
  functionality of going as far as possible down an DT towards a path
  and returning either the node of however far was reached or the
  node if the full path was reached, respectively.
*/

/*
  Traverses the DT starting at the root as far as possible towards
  absolute path oPPath. If able to traverse, returns an int SUCCESS
  status and sets *poNFurthest to the furthest node reached (which may
  be only a prefix of oPPath, or even NULL if the root is NULL).
  Otherwise, sets *poNFurthest to NULL and returns with status:
  * CONFLICTING_PATH if the root's path is not a prefix of oPPath
  * MEMORY_ERROR if memory could not be allocated to complete request
*/
static int FT_traversePath(Path_T oPPath, Dir_T *poNFurthest)
{
  int iStatus;
  Path_T oPPrefix = NULL;
  Dir_T oNCurr;
  Dir_T oNChild = NULL;
  size_t ulDepth;
  size_t i;
  size_t ulChildID;

  assert(oPPath != NULL);
  assert(poNFurthest != NULL);

  /* root is NULL -> won't find anything */
  if (oNRoot == NULL)
  {
    *poNFurthest = NULL;
    return SUCCESS;
  }

  iStatus = Path_prefix(oPPath, 1, &oPPrefix);
  if (iStatus != SUCCESS)
  {
    *poNFurthest = NULL;
    return iStatus;
  }

  if (Path_comparePath(Dir_getPath(oNRoot), oPPrefix))
  {
    Path_free(oPPrefix);
    *poNFurthest = NULL;
    return CONFLICTING_PATH;
  }
  Path_free(oPPrefix);
  oPPrefix = NULL;

  oNCurr = oNRoot;
  ulDepth = Path_getDepth(oPPath);
  for (i = 2; i <= ulDepth; i++)
  {
    iStatus = Path_prefix(oPPath, i, &oPPrefix);
    if (iStatus != SUCCESS)
    {
      *poNFurthest = NULL;
      return iStatus;
    }
    if (Dir_hasChild(oNCurr, oPPrefix, &ulChildID))
    {
      /* go to that child and continue with next prefix */
      Path_free(oPPrefix);
      oPPrefix = NULL;
      iStatus = Dir_getSubDir(oNCurr, ulChildID, &oNChild);
      if (iStatus != SUCCESS)
      {
        *poNFurthest = NULL;
        return iStatus;
      }
      oNCurr = oNChild;
    }
    else
    {
      /* oNCurr doesn't have child with path oPPrefix:
         this is as far as we can go */
      break;
    }
  }

  Path_free(oPPrefix);
  *poNFurthest = oNCurr;
  return SUCCESS;
}

/*
  Traverses the DT to find a node with absolute path pcPath. Returns a
  int SUCCESS status and sets *poNResult to be the node, if found.
  Otherwise, sets *poNResult to NULL and returns with status:
  * INITIALIZATION_ERROR if the DT is not in an initialized state
  * BAD_PATH if pcPath does not represent a well-formatted path
  * CONFLICTING_PATH if the root's path is not a prefix of pcPath
  * NO_SUCH_PATH if no node with pcPath exists in the hierarchy
  * MEMORY_ERROR if memory could not be allocated to complete request
 */
static int FT_findDir(const char *pcPath, Dir_T *poNResult)
{
  Path_T oPPath = NULL;
  Dir_T oNFound = NULL;
  int iStatus;

  assert(pcPath != NULL);
  assert(poNResult != NULL);

  if (!bIsInitialized)
  {
    *poNResult = NULL;
    return INITIALIZATION_ERROR;
  }

  iStatus = Path_new(pcPath, &oPPath);
  if (iStatus != SUCCESS)
  {
    *poNResult = NULL;
    return iStatus;
  }

  iStatus = FT_traversePath(oPPath, &oNFound);
  if (iStatus != SUCCESS)
  {
    Path_free(oPPath);
    *poNResult = NULL;
    return iStatus;
  }

  if (oNFound == NULL)
  {
    Path_free(oPPath);
    *poNResult = NULL;
    return NO_SUCH_PATH;
  }

  if (Path_comparePath(Dir_getPath(oNFound), oPPath) != 0)
  {
    Path_free(oPPath);
    *poNResult = NULL;
    return NO_SUCH_PATH;
  }

  Path_free(oPPath);
  *poNResult = oNFound;
  return SUCCESS;
}
/*--------------------------------------------------------------------*/

/*
  Removes the FT file with absolute path pcPath.
  Returns SUCCESS if found and removed.
  Otherwise, returns:
  * INITIALIZATION_ERROR if the FT is not in an initialized state
  * BAD_PATH if pcPath does not represent a well-formatted path
  * CONFLICTING_PATH if the root exists but is not a prefix of pcPath
  * NO_SUCH_PATH if absolute path pcPath does not exist in the FT
  * NOT_A_FILE if pcPath is in the FT as a directory not a file
  * MEMORY_ERROR if memory could not be allocated to complete request
*/
int FT_rmFile(const char *pcPath)
{

  int iStatus;
  size_t ulIndex;
  Dir_T oNFoundParentDir = NULL;
  Path_T parentDirPath = NULL;
  Path_T oPPath = NULL;
  File_T oFile;
  assert(pcPath != NULL);
  iStatus = Path_new(pcPath, &oPPath);
  if (iStatus != SUCCESS)
    return iStatus;
  Path_prefix(oPPath, Path_getDepth(oPPath) - 1, &parentDirPath);

  iStatus = FT_findDir(Path_getPathname(parentDirPath), &oNFoundParentDir);

  if (iStatus != SUCCESS)
    return iStatus;

  oFile = DynArray_removeAt(Dir_getFiles(oNFoundParentDir),
                            DynArray_bsearch(Dir_getFiles(oNFoundParentDir), (char *)Path_getPathname(oPPath), &ulIndex,
                                             (int (*)(const void *, const void *))File_compareString

                                             ));

  if (Dir_hasChild(oNFoundParentDir, oPPath, &ulIndex))
  {
    return NOT_A_FILE;
  }
  File_free(oFile);

  if (ulCount == 0)
    oNRoot = NULL;

  return SUCCESS;
}

int FT_init(void)
{

  if (bIsInitialized)
    return INITIALIZATION_ERROR;

  bIsInitialized = TRUE;
  oNRoot = NULL;
  ulCount = 0;

  return SUCCESS;
}

/*
  Removes all contents of the data structure and
  returns it to an uninitialized state.
  Returns INITIALIZATION_ERROR if not already initialized,
  and SUCCESS otherwise.
*/
int FT_destroy(void)
{

  if (!bIsInitialized)
    return INITIALIZATION_ERROR;

  if (oNRoot)
  {
    ulCount -= Dir_free(oNRoot);
    oNRoot = NULL;
  }

  bIsInitialized = FALSE;

  return SUCCESS;
}

boolean FT_containsDir(const char *pcPath)
{
  int iStatus;
  Dir_T oNFound = NULL;

  assert(pcPath != NULL);

  iStatus = FT_findDir(pcPath, &oNFound);
  return (boolean)(iStatus == SUCCESS);
}

int FT_rmDir(const char *pcPath)
{
  int iStatus;
  Dir_T oNFound = NULL;

  assert(pcPath != NULL);

  iStatus = FT_findDir(pcPath, &oNFound);

  if (iStatus != SUCCESS)
    return iStatus;

  ulCount -= Dir_free(oNFound);
  if (ulCount == 0)
    oNRoot = NULL;
  return SUCCESS;
}

/*
   Inserts a new directory into the FT with absolute path pcPath.
   Returns SUCCESS if the new directory is inserted successfully.
   Otherwise, returns:
   * INITIALIZATION_ERROR if the FT is not in an initialized state
   * BAD_PATH if pcPath does not represent a well-formatted path
   * CONFLICTING_PATH if the root exists but is not a prefix of pcPath
   * NOT_A_DIRECTORY if a proper prefix of pcPath exists as a file
   * ALREADY_IN_TREE if pcPath is already in the FT (as dir or file)
   * MEMORY_ERROR if memory could not be allocated to complete request
*/
int FT_insertDir(const char *pcPath)
{
  int iStatus;
  Path_T oPPath = NULL;
  Dir_T oNFirstNew = NULL;
  Dir_T oNCurr = NULL;
  size_t ulDepth, ulIndex;
  size_t ulNewNodes = 0;

  assert(pcPath != NULL);

  /* validate pcPath and generate a Path_T for it */
  if (!bIsInitialized)
    return INITIALIZATION_ERROR;

  iStatus = Path_new(pcPath, &oPPath);
  if (iStatus != SUCCESS)
    return iStatus;

  /* find the closest ancestor of oPPath already in the tree */
  iStatus = FT_traversePath(oPPath, &oNCurr);
  if (iStatus != SUCCESS)
  {
    Path_free(oPPath);
    return iStatus;
  }

  /* no ancestor node found, so if root is not NULL,
     pcPath isn't underneath root. */
  if (oNCurr == NULL && oNRoot != NULL)
  {
    Path_free(oPPath);
    return CONFLICTING_PATH;
  }

  ulDepth = Path_getDepth(oPPath);
  if (oNCurr == NULL) /* new root! */
    ulIndex = 1;
  else
  {
    ulIndex = Path_getDepth(Dir_getPath(oNCurr)) + 1;

    /* oNCurr is the node we're trying to insert */
    if (ulIndex == ulDepth + 1 && !Path_comparePath(oPPath,
                                                    Dir_getPath(oNCurr)))
    {
      Path_free(oPPath);
      return ALREADY_IN_TREE;
    }
  }

  /* starting at oNCurr, build rest of the path one level at a time */
  while (ulIndex <= ulDepth)
  {
    Path_T oPPrefix = NULL;
    Dir_T oNNewNode = NULL;

    /* generate a Path_T for this level */
    iStatus = Path_prefix(oPPath, ulIndex, &oPPrefix);
    if (iStatus != SUCCESS)
    {
      Path_free(oPPath);
      if (oNFirstNew != NULL)
        (void)Dir_free(oNFirstNew);
      return iStatus;
    }

    /* insert the new node for this level */
    iStatus = Dir_new(oPPrefix, oNCurr, &oNNewNode);
    if (iStatus != SUCCESS)
    {
      Path_free(oPPath);
      Path_free(oPPrefix);
      if (oNFirstNew != NULL)
        (void)Dir_free(oNFirstNew);
      return iStatus;
    }

    /* set up for next level */
    Path_free(oPPrefix);
    oNCurr = oNNewNode;
    ulNewNodes++;
    if (oNFirstNew == NULL)
      oNFirstNew = oNCurr;
    ulIndex++;
  }

  Path_free(oPPath);
  /* update DT state variables to reflect insertion */
  if (oNRoot == NULL)
    oNRoot = oNFirstNew;
  ulCount += ulNewNodes;

  return SUCCESS;
}

static int FT_findFile(const char *pcPath, File_T *poNResult)
{

  int iStatus;
  size_t ulIndex;
  Dir_T oNFoundParentDir = NULL;
  Path_T parentDirPath = NULL;
  Path_T oPPath = NULL;
  File_T oFile;
  assert(pcPath != NULL);

  iStatus = Path_new(pcPath, &oPPath);
  if (iStatus != SUCCESS)
    return iStatus;

  Path_prefix(oPPath, Path_getDepth(oPPath) - 1, &parentDirPath);

  iStatus = FT_findDir(Path_getPathname(parentDirPath), &oNFoundParentDir);

  if (iStatus != SUCCESS)
    return iStatus;

  oFile = DynArray_get(Dir_getFiles(oNFoundParentDir),
                       DynArray_bsearch(Dir_getFiles(oNFoundParentDir), (char *)Path_getPathname(oPPath), &ulIndex,
                                        (int (*)(const void *, const void *))File_compareString));

  if (oFile == NULL)
  {
    return NO_SUCH_PATH;
  }

  *poNResult = oFile;

  return SUCCESS;
}

/*
  Returns TRUE if the FT contains a file with absolute path
  pcPath and FALSE if not or if there is an error while checking.
*/
boolean FT_containsFile(const char *pcPath)
{
  int iStatus;
  File_T oNFound = NULL;

  assert(pcPath != NULL);

  iStatus = FT_findFile(pcPath, &oNFound);
  return (boolean)(iStatus == SUCCESS);
}

/*
  Returns the contents of the file with absolute path pcPath.
  Returns NULL if unable to complete the request for any reason.

  Note: checking for a non-NULL return is not an appropriate
  contains check, because the contents of a file may be NULL.
*/
void *FT_getFileContents(const char *pcPath)
{
  File_T oFile;
  int iStatus;
  assert(pcPath != NULL);
  iStatus = FT_findFile(pcPath, &oFile);
  if (iStatus != SUCCESS)
  {
    return NULL;
  }

  return File_getContents(oFile);
}

/*
  Replaces current contents of the file with absolute path pcPath with
  the parameter pvNewContents of size ulNewLength bytes.
  Returns the old contents if successful. (Note: contents may be NULL.)
  Returns NULL if unable to complete the request for any reason.
*/
void *FT_replaceFileContents(const char *pcPath, void *pvNewContents,
                             size_t ulNewLength)
{
  void *retContent;
  File_T oFile;
  int iStatus;
  assert(pcPath != NULL);
  iStatus = FT_findFile(pcPath, &oFile);
  if (iStatus != SUCCESS)
    return NULL;
  retContent = File_getContents(oFile);

  File_replaceContents(oFile, pvNewContents, ulNewLength);

  return retContent;
}

/*
  Returns SUCCESS if pcPath exists in the hierarchy,
  Otherwise, returns:
  * INITIALIZATION_ERROR if the FT is not in an initialized state
  * BAD_PATH if pcPath does not represent a well-formatted path
  * CONFLICTING_PATH if the root's path is not a prefix of pcPath
  * NO_SUCH_PATH if absolute path pcPath does not exist in the FT
  * MEMORY_ERROR if memory could not be allocated to complete request

  When returning SUCCESS,
  if path is a directory: sets *pbIsFile to FALSE, *pulSize unchanged
  if path is a file: sets *pbIsFile to TRUE, and
                     sets *pulSize to the length of file's contents

  When returning another status, *pbIsFile and *pulSize are unchanged.
*/
int FT_stat(const char *pcPath, boolean *pbIsFile, size_t *pulSize)
{
  Dir_T foundDir;
  File_T foundFile;
  int iStatus;

  assert(pcPath != NULL);
  iStatus = FT_findDir(pcPath, &foundDir);
  if (iStatus == SUCCESS)
  {
    *pbIsFile = FALSE;
    return SUCCESS;
  }
  if (FT_findFile(pcPath, &foundFile) == SUCCESS)
  {
    *pbIsFile = TRUE;
    *pulSize = File_getLength(foundFile);
    return SUCCESS;
  }
  return iStatus;
}

int FT_insertFile(const char *pcPath, void *pvContents, size_t ulLength)
{
  int iStatus;
  Dir_T oNFoundParentDir = NULL;
  Path_T parentDirPath = NULL;
  Path_T oPPath = NULL;
  File_T oFile;
  assert(pcPath != NULL);

  /* validate pcPath and generate a Path_T for it */
  if (!bIsInitialized)
    return INITIALIZATION_ERROR;
  iStatus = Path_new(pcPath, &oPPath);
  if (iStatus != SUCCESS)
    return iStatus;
  Path_prefix(oPPath, Path_getDepth(oPPath) - 1, &parentDirPath);
  if (iStatus != SUCCESS)
    return iStatus;
  iStatus = Path_new(pcPath, &oPPath);
  if (iStatus != SUCCESS)
    return iStatus;

  iStatus = FT_findDir(Path_getPathname(parentDirPath), &oNFoundParentDir);

  /* appropiate parent directory found */
  if (iStatus == SUCCESS)
  {
    if (FT_containsFile(pcPath))
    {
      return ALREADY_IN_TREE;
    }
    else
    {
      File_new(oPPath, oNFoundParentDir, &oFile);
      File_setContents(oFile, pvContents, ulLength);
      return SUCCESS;
    }
  }

  iStatus = FT_insertDir(Path_getPathname(parentDirPath));
  if (iStatus != SUCCESS)
  {
    return iStatus;
  }
  iStatus = FT_findDir(Path_getPathname(parentDirPath), &oNFoundParentDir);
  if (iStatus != SUCCESS)
  {
    return iStatus;
  }
  File_new(oPPath, oNFoundParentDir, &oFile);
  File_setContents(oFile, pvContents, ulLength);
  return SUCCESS;
}

char *FT_toString(void)
{
  char *ret;
  if (!bIsInitialized)
    return NULL;
  ret = (char *) malloc(2 * sizeof(char));
  *ret = '\0';
  return ret;
}