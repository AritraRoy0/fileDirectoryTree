/*--------------------------------------------------------------------*/
/* fileNode.c                                                           */
/* Author: Christopher Moretti                                        */
/*--------------------------------------------------------------------*/

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "dynarray.h"
#include "fileNode.h"
#include "dirNode.h"
#include "checkerDT.h"

/* A file node in a DT */
struct node {
   /* the object corresponding to the node's absolute path */
   Path_T path;
   /* this node's parent */
   Dir_T parentDir;
   /* the pointer to the begining of the contents of the file */
   void * contents;
    /* the lenght of the contents */
   size_t conLen; 
};