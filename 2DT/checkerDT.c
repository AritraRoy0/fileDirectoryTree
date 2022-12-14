/*--------------------------------------------------------------------*/
/* checkerDT.c                                                        */
/* Author:                                                            */
/*--------------------------------------------------------------------*/

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "checkerDT.h"
#include "dynarray.h"
#include "path.h"

/* see checkerDT.h for specification */
boolean CheckerDT_Node_isValid(Node_T oNNode)
{
   Node_T oNParent;
   Path_T oPNPath;
   Path_T oPPPath;
   size_t identifier;

   /* Sample check: a NULL pointer is not a valid node */
   if (oNNode == NULL)
   {
      fprintf(stderr, "A node is a NULL pointer\n");
      return FALSE;
   }

   /* Sample check: parent's path must be the longest possible
      proper prefix of the node's path */
   oNParent = Node_getParent(oNNode);
   if (oNParent != NULL)
   {
      oPNPath = Node_getPath(oNNode);
      oPPPath = Node_getPath(oNParent);

      if (Path_getSharedPrefixDepth(oPNPath, oPPPath) !=
          Path_getDepth(oPNPath) - 1)
      {
         fprintf(stderr, "P-C nodes don't have P-C paths: (%s) (%s)\n",
                 Path_getPathname(oPPPath), Path_getPathname(oPNPath));
         return FALSE;
      }
   }

   /* Check 1: Node_compare works, i.e. the function applied to two
      referances to the current node return 0 */
   if (Node_compare(oNNode, oNNode) != 0)
   {
      fprintf(stderr, "Node_compare not valid.\n");
      return FALSE;
   }

   /* Check 2: Current Node is identified as its parent's child
      by the Node_hasChild function
   */
   if (oNParent != NULL && !Node_hasChild(oNParent, oPNPath, &identifier))
   {
      fprintf(stderr, "Node_hasChild does not recognize"
                      " node as a child of its parent node.\n");
      return FALSE;
   }

   return TRUE;
}

/*
   Performs a pre-order traversal of the tree rooted at oNNode with
   total number of nodes ulCount. As the function runs, it accumulates
   the total number of nodes encountered at any given point in time
   at acCount.
   Returns FALSE if a broken invariant is found and
   returns TRUE otherwise.


*/
static boolean CheckerDT_treeCheck(Node_T oNNode, size_t ulCount, size_t *acCount)
{
   size_t ulIndex, ulIndex2;
   Node_T siblingNode;
   int pathCompare, iStatus;

   assert(acCount != NULL);

   if (oNNode != NULL)
   {

      /* Sample check on each node: node must be valid */
      /* If not, pass that failure back up immediately */
      if (!CheckerDT_Node_isValid(oNNode))
         return FALSE;

      /* Recur on every child of oNNode */
      for (ulIndex = 0; ulIndex < Node_getNumChildren(oNNode); ulIndex++)
      {
         Node_T oNChild = NULL;
         iStatus = Node_getChild(oNNode, ulIndex, &oNChild);
         *acCount += 1;

         /* Check 0: Check if number of grandchildren and no. of children returned are same */
         if (iStatus != SUCCESS)
         {
            fprintf(stderr, "getNumChildren claims more children than getChild returns\n");
            return FALSE;
         }

         for (ulIndex2 = 0; ulIndex2 < Node_getNumChildren(oNNode); ulIndex2++)
         {
            /* Check 1: file paths are unique for nodes in same directory */
            if (ulIndex == ulIndex2)
            {
               continue;
               /* for sibling nodes before current child */
            }

            siblingNode = NULL;
            iStatus = Node_getChild(oNNode, ulIndex2, &siblingNode);
            /* Check 0: Check if number of grandchildren and no. of children returned are same */
            if (iStatus != SUCCESS)
            {
               fprintf(stderr, "getNumChildren claims more children than getChild returns\n");
               return FALSE;
            }
            pathCompare = Path_comparePath(Node_getPath(oNChild), Node_getPath(siblingNode));
            if (!pathCompare)
            {
               fprintf(stderr, "sibling nodes have duplicate path names\n");
               return FALSE;
            }
            /* Check 2: check all children are in alphabetical order */
            /* for sibling nodes after current child */
            if (ulIndex2 < ulIndex)
            {

               if (pathCompare < 0)
               {
                  fprintf(stderr, "sibling nodes not in alphabetical order\n");
                  return FALSE;
               }

               /* for sibling nodes after current child */
            }
            else
            {
               if (pathCompare > 0)
               {
                  fprintf(stderr, "sibling nodes not in alphabetical order\n");
                  return FALSE;
               }
            }
         }

         /* if recurring down one subtree results in a failed check
            farther down, passes the failure back up immediately */
         if (!CheckerDT_treeCheck(oNChild, ulCount, acCount))
         {

            return FALSE;
         }
      }

      /* Check 4: Accumilative count of directories can't be greater than ulCount */

      if (*acCount > ulCount)
      {
         fprintf(stderr, "There are more directories than ulCount indicates\n");
         return FALSE;
      }
   }

   return TRUE;
}

/* see checkerDT.h for specification */
boolean CheckerDT_isValid(boolean bIsInitialized, Node_T oNRoot,
                          size_t ulCount)
{
   size_t *acCount;
   boolean ret = FALSE;
   /* Sample check on a top-level data structure invariant:
      if the DT is not initialized, its count should be 0. */
   if (!bIsInitialized)
      if (ulCount != 0)
      {
         fprintf(stderr, "Not initialized, but count is not 0\n");
         return FALSE;
      }
   acCount = malloc(sizeof(size_t));
   *acCount = 0;
   if (oNRoot != NULL)
      *acCount = 1;

   /* Now checks invariants recursively at each node from the root. */
   ret = CheckerDT_treeCheck(oNRoot, ulCount, acCount);
   free(acCount);
   return ret;
}
