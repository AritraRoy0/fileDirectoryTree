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
#include "a4def.h"
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

  The FT_traversePath and FT_findDir FT_findFile functions modularize the common
  functionality of going as far as possible down an DT towards a path
  and returning either the node of however far was reached or the
  node if the full path was reached, respectively.
*/

/*
  Traverses the FT starting at the root as far as possible towards
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
  /*  fprintf(stderr, "----------For pathname : %s\n", Path_getPathname(oPPath)); */
  ulDepth = Path_getDepth(oPPath);
  for (i = 2; i <= ulDepth; i++)
  {
    iStatus = Path_prefix(oPPath, i, &oPPrefix);
    if (iStatus != SUCCESS)
    {
      *poNFurthest = NULL;
      return iStatus;
    }
    /* fprintf(stderr, "Prefix at depth %ld : %s\n", i, Path_getPathname(oPPrefix)); */
    if (Dir_hasSubDir(oNCurr, oPPrefix, &ulChildID) == TRUE)
    {
      /* go to that child and continue with next prefix */
      /* fprintf(stderr, "Has subDir at depth %ld : %s\n", i, Path_getPathname(oPPrefix)); */
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
  Traverses the DT to find a directory node with absolute path pcPath. Returns a
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
  size_t ulChildID;
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
/*finding the closest ancestor*/
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
  if (Dir_hasFile(oNFound, oPPath, &ulChildID))
  {
    return NOT_A_DIRECTORY;
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
/*
  Traverses the DT to find a file node with absolute path pcPath. Returns a
  int SUCCESS status and sets *poNResult to be the node, if found.
  Otherwise, sets *poNResult to NULL and returns with status:
  * INITIALIZATION_ERROR if the DT is not in an initialized state
  * BAD_PATH if pcPath does not represent a well-formatted path
  * CONFLICTING_PATH if the root's path is not a prefix of pcPath
  * NO_SUCH_PATH if no node with pcPath exists in the hierarchy
  * MEMORY_ERROR if memory could not be allocated to complete request
 */
static int FT_findFile(const char *pcPath, File_T *poNResult)
{

  int iStatus;
  size_t ulIndex;
  Dir_T oNFoundParentDir = NULL;
  Path_T parentDirPath = NULL;
  Path_T oPPath = NULL;
  File_T oFile;
  assert(pcPath != NULL);
  assert(poNResult != NULL);

  if (FT_containsDir(pcPath))
  {
    *poNResult = NULL;
    return NOT_A_FILE;
  }

  iStatus = Path_new(pcPath, &oPPath);
  if (iStatus != SUCCESS)
  {
    *poNResult = NULL;
    return iStatus;
  }
  if (Path_getDepth(oPPath) == 1)
  {
    *poNResult = NULL;
    return CONFLICTING_PATH;
  }
  Path_prefix(oPPath, Path_getDepth(oPPath) - 1, &parentDirPath);

  iStatus = FT_findDir(Path_getPathname(parentDirPath), &oNFoundParentDir);

  if (iStatus != SUCCESS)
  {
    *poNResult = NULL;
    return iStatus;
  }
  DynArray_bsearch(Dir_getFiles(oNFoundParentDir), (char *)Path_getPathname(oPPath), &ulIndex,
                   (int (*)(const void *, const void *))File_compareString);

  if (ulIndex >= Dir_getNumFiles(oNFoundParentDir))
  {
    *poNResult = NULL;
    return NO_SUCH_PATH;
  }
  oFile = DynArray_get(Dir_getFiles(oNFoundParentDir), ulIndex);
  *poNResult = oFile;

  if (!Path_compareString(File_getPath(oFile), pcPath))
  {
    return SUCCESS;
  }
  else
  {
    *poNResult = NULL;
    return NO_SUCH_PATH;
  }
}

/*--------------------------------------------------------------------*/


int FT_rmFile(const char *pcPath)
{

  File_T oFile;
  int iStatus;
  assert(pcPath != NULL);

  iStatus = FT_findFile(pcPath, &oFile);
  if (iStatus != SUCCESS)
  {
    return iStatus;
  }
  File_free(oFile);

  ulCount--;

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


int FT_insertDir(const char *pcPath)
{
  
  int iStatus;
  Path_T oPPath, parentDirPath;
  Dir_T oNFirstNew = NULL;
  Dir_T oNCurr = NULL;
  size_t ulDepth, ulIndex;
  size_t ulNewNodes = 0;

  assert(pcPath != NULL);

  /* validate pcPath and generate a Path_T for it */
  if (!bIsInitialized)
  {
    return INITIALIZATION_ERROR;
  }

  if (FT_containsFile(pcPath))
  {
    return ALREADY_IN_TREE;
  }
  iStatus = Path_new(pcPath, &oPPath);
  if (iStatus != SUCCESS)
    return iStatus;

  iStatus = FT_findDir(pcPath, &oNCurr);
  if (iStatus == SUCCESS)
  {
    return ALREADY_IN_TREE;
  }
  if (Path_getDepth(oPPath) > 1)
  {
    Path_prefix(oPPath, Path_getDepth(oPPath) - 1, &parentDirPath);

    if (FT_containsFile(Path_getPathname(parentDirPath)))
      return NOT_A_DIRECTORY;

    /* find the closest ancestor of oPPath already in the tree */
  }

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


boolean FT_containsFile(const char *pcPath)
{
  int iStatus;
  File_T oNFound = NULL;

  assert(pcPath != NULL);

  iStatus = FT_findFile(pcPath, &oNFound);
  return (boolean)(iStatus == SUCCESS);
}


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

  File_setContents(oFile, pvNewContents, ulNewLength);

  return retContent;
}


int FT_stat(const char *pcPath, boolean *pbIsFile, size_t *pulSize)
{
  Dir_T foundDir;
  File_T foundFile;
  int iStatus;

  assert(pcPath != NULL);
  assert(pbIsFile != NULL);
  assert(pulSize != NULL);

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
  if (FT_containsDir(pcPath))
  {
    fprintf(stderr, "Already in tree dir\n");
    return ALREADY_IN_TREE;
  }
  if (FT_containsFile(pcPath))
  {
    fprintf(stderr, "Already in tree file\n");
    return ALREADY_IN_TREE;
  }

  iStatus = Path_new(pcPath, &oPPath);
  if (iStatus != SUCCESS)
    return iStatus;

  if (Path_getDepth(oPPath) == 1)
  {

    fprintf(stderr, "conflicting path\n");
    return CONFLICTING_PATH;
  }
  Path_prefix(oPPath, Path_getDepth(oPPath) - 1, &parentDirPath);

  iStatus = FT_findFile(Path_getPathname(parentDirPath), &oFile);

  if (iStatus == SUCCESS)
  {
    fprintf(stderr, "Not a directory\n");
    return NOT_A_DIRECTORY;
  }
  iStatus = FT_findDir(Path_getPathname(parentDirPath), &oNFoundParentDir);

  /* appropiate parent directory found */
  if (iStatus == SUCCESS)
  {

    File_new(oPPath, oNFoundParentDir, &oFile);
    File_setContents(oFile, pvContents, ulLength);
    ulCount++;
    return SUCCESS;
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
  ulCount++;
  return SUCCESS;
}

/* --------------------------------------------------------------------

  The following auxiliary functions are used for generating the
  string representation of the DT.
*/

/*
  Performs a pre-order traversal of the tree rooted at n,
  accumulating the total string length of the string 
  representation of the entire FT returns size_t ulLength
*/

static size_t FT_preOrderTraversal(Dir_T n, size_t i, size_t *ulLength)
{
  size_t c;
  File_T oFile;
  Dir_T oNChild = NULL;

  if (n != NULL)
  {
    i++;
    for (c = 0; c < Dir_getNumFiles(n); c++)
    {
      (void)Dir_getFile(n, c, &oFile);
      *ulLength += (Path_getStrLength(File_getPath(oFile)) + 1);
      i++;
    }
    for (c = 0; c < Dir_getNumSubDirs(n); c++)
    {
      int iStatus;
      oNChild = NULL;
      iStatus = Dir_getSubDir(n, c, &oNChild);
      *ulLength += (Path_getStrLength(Dir_getPath(oNChild)) + 1);
      assert(iStatus == SUCCESS);
      i = FT_preOrderTraversal(oNChild, i, ulLength);
    }
  }
  return i;
}

/*
  Performs a pre-order traversal of the tree rooted at n,
  accumulating the total string of the string 
  representation of the entire FT in retString. also accumulates
  the number of nodes passed through in i and returns it.
*/

static size_t FT_preOrderStringTraversal(Dir_T n, size_t i, char *retString)
{

  size_t c;
  File_T oFile;
  Dir_T oNChild = NULL;

  assert(retString != NULL);
  if (n != NULL)
  {
    strcat(retString, Path_getPathname(Dir_getPath(n)));
    strcat(retString, "\n");
    i++;
    for (c = 0; c < Dir_getNumFiles(n); c++)
    {
      (void)Dir_getFile(n, c, &oFile);
      strcat(retString, Path_getPathname(File_getPath(oFile)));
      strcat(retString, "\n");
      i++;
    }
    for (c = 0; c < Dir_getNumSubDirs(n); c++)
    {
      int iStatus;
      oNChild = NULL;
      iStatus = Dir_getSubDir(n, c, &oNChild);
      assert(iStatus == SUCCESS);

      i = FT_preOrderStringTraversal(oNChild, i, retString);
    }
  }
  return i;
}


char *FT_toString(void)
{
  size_t totalStrlen = 1;
  char *ret = NULL;

  if (!bIsInitialized)
    return NULL;

  (void)FT_preOrderTraversal(oNRoot, 0, &totalStrlen);

  ret = malloc(totalStrlen);
  if (ret == NULL)
  {
    return NULL;
  }
  *ret = '\0';

  (void)FT_preOrderStringTraversal(oNRoot, 0, ret);

  return ret;
}