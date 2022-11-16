/*--------------------------------------------------------------------*/
/* ft.c                                                               */
/* Author: Roy Mazumder and Rooshan Khalid                            */
/*--------------------------------------------------------------------*/

#include <stddef.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "dynarray.h"
#include "path.h"
#include "dirNode.h"
#include "fileNode.h"
#include "ft.h"

/*
  A File Tree is a representation of a hierarchy of directories and files,
  represented as an AO with 3 state variables:
*/

/* 1. a flag for being in an initialized state (TRUE) or not (FALSE) */
static boolean bIsInitialized;
/* 2. a pointer to the root node in the hierarchy */
static Node_T oNRoot;
/* 3. a counter of the number of nodes in the hierarchy */
static size_t ulCount;

<<<<<<< HEAD

/*
  Sets the FT data structure to an initialized state.
  The data structure is initially empty.
  Returns INITIALIZATION_ERROR if already initialized,
  and SUCCESS otherwise.
*/
int FT_init(void){
  assert(CheckerDT_isValid(bIsInitialized, oNRoot, ulCount));

   if(bIsInitialized)
      return INITIALIZATION_ERROR;

   bIsInitialized = TRUE;
   oNRoot = NULL;
   ulCount = 0;

   assert(CheckerDT_isValid(bIsInitialized, oNRoot, ulCount));
   return SUCCESS;
}

/*
  Removes all contents of the data structure and
  returns it to an uninitialized state.
  Returns INITIALIZATION_ERROR if not already initialized,
  and SUCCESS otherwise.
*/
int FT_destroy(void){
  assert(CheckerDT_isValid(bIsInitialized, oNRoot, ulCount));

   if(!bIsInitialized)
      return INITIALIZATION_ERROR;

   if(oNRoot) {
      ulCount -= Node_free(oNRoot);
      oNRoot = NULL;
   }

   bIsInitialized = FALSE;

   assert(CheckerDT_isValid(bIsInitialized, oNRoot, ulCount));
   return SUCCESS;
}

/*
  Returns TRUE if the FT contains a directory with absolute path
  pcPath and FALSE if not or if there is an error while checking.
*/
boolean FT_containsDir(const char *pcPath){
  int iStatus;
   Node_T oNFound = NULL;

   assert(pcPath != NULL);

   iStatus = FT_findNode(pcPath, &oNFound);
   return (boolean) (iStatus == SUCCESS);
}

/*
   Inserts a new directory into the FT with absolute path pcPath.
   Returns SUCCESS if the new directory is inserted successfully.
   Otherwise, returns:
   * INITIALIZATION_ERROR if the FT is not in an initialized state
   * BAD_PATH if pcPath does not represent a well-formatted path
   * CONFLICTING_PATH if the root exists but is not a prefix of pcPath
=======
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
   Inserts a new file into the FT with absolute path pcPath, with
   file contents pvContents of size ulLength bytes.
   Returns SUCCESS if the new file is inserted successfully.
   Otherwise, returns:
   * INITIALIZATION_ERROR if the FT is not in an initialized state
   * BAD_PATH if pcPath does not represent a well-formatted path
   * CONFLICTING_PATH if the root exists but is not a prefix of pcPath,
                      or if the new file would be the FT root
>>>>>>> 70f4abe09c98a89257d600a78d9a0948b7f931c4
   * NOT_A_DIRECTORY if a proper prefix of pcPath exists as a file
   * ALREADY_IN_TREE if pcPath is already in the FT (as dir or file)
   * MEMORY_ERROR if memory could not be allocated to complete request
*/
<<<<<<< HEAD
int FT_insertDir(const char *pcPath);

/*
  Removes the FT hierarchy (subtree) at the directory with absolute
  path pcPath. Returns SUCCESS if found and removed.
=======
int FT_insertFile(const char *pcPath, void *pvContents,
                  size_t ulLength)
{
    int iStatus;
    size_t ulIndex;
    Dir_T oNFoundParentDir = NULL;
    Path_T parentDirPath, grandparentDirPath;
    assert(pcPath != NULL);

    Path_prefix(pcPath, Path_getDepth(pcPath) - 1, &parentDirPath);

    iStatus = Ft_findDir(parentDirPath, &oNFoundParentDir);

    // no valid parent directory found
    if (iStatus != SUCCESS)
    {
        // check if same name file directory exists
        Path_prefix(pcPath, Path_getDepth(pcPath) - 2, &grandparentDirPath);

        iStatus = Ft_findDir(parentDirPath, &oNFoundParentDir);
        if (iStatus != SUCCESS)
            return iStatus;

        if (Dir_hasChild(oNFoundParentDir, parentDirPath, ulIndex))

            return NOT_A_DIRECTORY;

        else
            return iStatus;
    }
}

/*
  Returns TRUE if the FT contains a file with absolute path
  pcPath and FALSE if not or if there is an error while checking.
*/
boolean FT_containsFile(const char *pcPath)
{
    int iStatus;
    Dir_T oNFoundParentDir = NULL;
    Path_T parentDirPath;
    size_t ulChildID;
    assert(pcPath != NULL);

    Path_prefix(pcPath, Path_getDepth(pcPath) - 1, &parentDirPath);

    iStatus = Ft_findDir(parentDirPath, &oNFoundParentDir);
    if (iStatus != SUCCESS)
        return FALSE;

    return Dir_hasChild(oNFoundParentDir, pcPath, ulChildID) && (Dir_get);
}

/*
  Removes the FT file with absolute path pcPath.
  Returns SUCCESS if found and removed.
>>>>>>> 70f4abe09c98a89257d600a78d9a0948b7f931c4
  Otherwise, returns:
  * INITIALIZATION_ERROR if the FT is not in an initialized state
  * BAD_PATH if pcPath does not represent a well-formatted path
  * CONFLICTING_PATH if the root exists but is not a prefix of pcPath
  * NO_SUCH_PATH if absolute path pcPath does not exist in the FT
<<<<<<< HEAD
  * NOT_A_DIRECTORY if pcPath is in the FT as a file not a directory
  * MEMORY_ERROR if memory could not be allocated to complete request
*/
int FT_rmDir(const char *pcPath){
  int iStatus;
   File_T oNFound = NULL;

   assert(pcPath != NULL);
   assert(CheckerDT_isValid(bIsInitialized, oNRoot, ulCount));

   iStatus = DT_findNode(pcPath, &oNFound);

   if(iStatus != SUCCESS)
       return iStatus;

   ulCount -= Node_free(oNFound);
   if(ulCount == 0)
      oNRoot = NULL;

   assert(CheckerDT_isValid(bIsInitialized, oNRoot, ulCount));
   return SUCCESS;
}

=======
  * NOT_A_FILE if pcPath is in the FT as a directory not a file
  * MEMORY_ERROR if memory could not be allocated to complete request
*/
int FT_rmFile(const char *pcPath)
{

    int iStatus;
    size_t ulIndex;
    Dir_T oNFoundParentDir = NULL;
    Path_T parentDirPath;
    assert(pcPath != NULL);

    Path_prefix(pcPath, Path_getDepth(pcPath) - 1, &parentDirPath);

    iStatus = Ft_findDir(parentDirPath, &oNFoundParentDir);

    if (iStatus != SUCCESS)
        return iStatus;

    (void)DynArray_removeAt(oNFoundParentDir->files,
                            DynArray_bsearch(oNFoundParentDir->files, (char *)Path_getPathname(oPPath), pulChildID,
                                             (int (*)(const void *, const void *))File_compareString

                                             ));

    if (Dir_contains(oNFoundParentDir, pcPath, ulIndex))
    {
        return NOT_A_FILE;
    }

    if (ulCount == 0)
        oNRoot = NULL;

    return SUCCESS;
}
>>>>>>> 70f4abe09c98a89257d600a78d9a0948b7f931c4
