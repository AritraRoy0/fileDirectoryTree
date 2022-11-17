/*--------------------------------------------------------------------*/
/* ft_client.c                                                        */
/* Author: Christopher Moretti                                        */
/*--------------------------------------------------------------------*/

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "ft.h"
#include "a4def.h"

/* Tests the FT implementation with an assortment of checks.
   Prints the status of the data structure along the way to stderr.
   Returns 0. */
int main(void)
{
   enum
   {
      ARRLEN = 1000
   };
   char *temp;
   boolean bIsFile;
   size_t l;
   char arr[ARRLEN];
   arr[0] = '\0';
   /* Before the data structure is initialized:
    * insert*, rm*, and destroy should all return INITIALIZATION_ERROR
    * contains* should return FALSE
    * toString should return NULL.
    */
   assert(FT_insertDir("1root/2child/3gkid") == INITIALIZATION_ERROR);
   assert(FT_containsDir("1root/2child/3gkid") == FALSE);
   assert(FT_rmDir("1root/2child/3gkid") == INITIALIZATION_ERROR);
   assert(FT_insertFile("1root/2child/3gkid/4ggk", NULL, 0) ==
          INITIALIZATION_ERROR);
   assert(FT_containsFile("1root/2child/3gkid/4ggk") == FALSE);
   assert(FT_rmFile("1root/2child/3gkid/4ggk") == INITIALIZATION_ERROR);
   assert((temp = FT_toString()) == NULL);
   assert(FT_destroy() == INITIALIZATION_ERROR);

   /* After initialization, the data structure is empty, so
      contains* should still return FALSE for any non-NULL string,
      and toString should return the empty string.
   */
   assert(FT_init() == SUCCESS);
   assert(FT_containsDir("1root/2child/3gkid") == FALSE);
   assert(FT_containsFile("1root/2child/3gkid/4ggk") == FALSE);
   assert((temp = FT_toString()) != NULL);
   assert(!strcmp(temp, ""));
   free(temp);

   /* A valid path must not:
    * be the empty string
    * start with a '/'
    * end with a '/'
    * have consecutive '/' delimiters.
    */

   assert(FT_insertDir("") == BAD_PATH);
   assert(FT_insertDir("/1root/2child") == BAD_PATH);
   assert(FT_insertDir("1root/2child/") == BAD_PATH);
   assert(FT_insertDir("1root//2child") == BAD_PATH);

   assert(FT_insertFile("", NULL, 0) == BAD_PATH);
   assert(FT_insertFile("/1root/2child", NULL, 0) == BAD_PATH);
   assert(FT_insertFile("1root/2child/", NULL, 0) == BAD_PATH);
   assert(FT_insertFile("1root//2child", NULL, 0) == BAD_PATH);

   /* putting a file at the root is illegal */
   assert(FT_insertFile("A", NULL, 0) == CONFLICTING_PATH);
   /* After insertion, the data structure should contain every prefix
      of the inserted path, toString should return a string with these
      prefixes, trying to insert it again should return
      ALREADY_IN_TREE, and trying to insert some other root should
      return CONFLICTING_PATH.
   */

   assert(FT_insertDir("1root/2child/3gkid") == SUCCESS);
   assert(FT_containsDir("1root") == TRUE);
   assert(FT_containsFile("1root") == FALSE);
   assert(FT_containsDir("1root/2child") == TRUE);
   assert(FT_containsFile("1root/2child") == FALSE);
   assert(FT_containsDir("1root/2child/3gkid") == TRUE);
      /*     ---------------------     */
   assert(FT_containsDir("1root/2child/3gkid") == TRUE);
   assert(FT_containsFile("1root/2child/3gkid") == FALSE);

   assert(FT_insertFile("1root/2second/3gfile", NULL, 0) == SUCCESS);


   assert(FT_containsDir("1root/2second") == TRUE);
   assert(FT_containsFile("1root/2second") == FALSE);
   assert(FT_containsDir("1root/2second/3gfile") == FALSE);
   assert(FT_containsFile("1root/2second/3gfile") == TRUE);

   assert(FT_getFileContents("1root/2second/3gfile") == NULL);

   fprintf(stderr, "Checkpoint 1:--------------------------------\n");
   assert(FT_containsDir("1root/2child/3gkid") == TRUE);
   assert(FT_insertDir("1root/2child/3gkid") == ALREADY_IN_TREE);
   fprintf(stderr, "Checkpoint 2: -------------------------------\n");
   assert(FT_insertFile("1root/2child/3gkid", NULL, 0) ==
          ALREADY_IN_TREE);

   fprintf(stderr, "Checkpoint 3:");

   assert(FT_insertDir("1otherroot") == CONFLICTING_PATH);
   assert(FT_insertDir("1otherroot/2d") == CONFLICTING_PATH);
   assert(FT_insertFile("1otherroot/2f", NULL, 0) == CONFLICTING_PATH);

   /* Trying to insert a third child should succeed, unlike in BDT */
   assert(FT_insertFile("1root/2third", NULL, 0) == SUCCESS);
   assert(FT_insertDir("1root/2ok/3yes/4indeed") == SUCCESS);
   assert(FT_containsDir("1root") == TRUE);
   assert(FT_containsDir("1root/2child") == TRUE);
   assert(FT_containsDir("1root/2second") == TRUE);
   assert(FT_containsDir("1root/2third") == FALSE);
   assert(FT_containsFile("1root/2third") == TRUE);
   assert(FT_containsDir("1root/2ok") == TRUE);
   assert(FT_containsDir("1root/2ok/3yes") == TRUE);
   assert(FT_containsDir("1root/2ok/3yes/4indeed") == TRUE);
   assert((temp = FT_toString()) != NULL);
   fprintf(stderr, "Checkpoint 1:\n%s\n", temp);
   free(temp);

   /* Children must be unique, but individual directories or files
      in different paths needn't be
   */
   assert(FT_insertFile("1root/2child/3gkid", NULL, 0) ==
          ALREADY_IN_TREE);
   assert(FT_insertDir("1root/2child/3gkid") == ALREADY_IN_TREE);
   assert(FT_insertDir("1root/2child/3gk2/4ggk") == SUCCESS);
   assert(FT_containsDir("1root/2child/3gk2/4ggk") == TRUE);
   assert(FT_containsFile("1root/2child/3gk2/4ggk") == FALSE);
   assert(FT_insertDir("1root/2child/2child/2child") == SUCCESS);
   assert(FT_containsDir("1root/2child/2child/2child") == TRUE);
   assert(FT_containsFile("1root/2child/2child/2child") == FALSE);
   assert(FT_insertFile("1root/2child/2child/2child/2child", NULL, 0) ==
          SUCCESS);
   assert(FT_containsDir("1root/2child/2child/2child/2child") == FALSE);
   assert(FT_containsFile("1root/2child/2child/2child/2child") == TRUE);
   assert((temp = FT_toString()) != NULL);
   fprintf(stderr, "Checkpoint 2:\n%s\n", temp);
   free(temp);

   /* Attempting to insert a child of a file is illegal */
   assert(FT_insertDir("1root/2third/3nopeD") == NOT_A_DIRECTORY);
   assert(FT_containsDir("1root/2third/3nopeD") == FALSE);
   assert(FT_insertFile("1root/2third/3nopeF", NULL, 0) ==
          NOT_A_DIRECTORY);
   assert(FT_containsFile("1root/2third/3nopeF") == FALSE);

   /* calling rm* on a path that doesn't exist should return
      NO_SUCH_PATH, but on a path that does exist with the right
      flavor should return SUCCESS and remove entire subtree rooted at
      that path
   */
   assert(FT_containsDir("1root/2child/3gkid") == TRUE);
   assert(FT_containsFile("1root/2second/3gfile") == TRUE);
   assert(FT_containsDir("1root/2second/3gfile") == FALSE);
   assert(FT_rmDir("1root/2child/3nope") == NO_SUCH_PATH);
   assert(FT_rmDir("1root/2second/3gfile") == NOT_A_DIRECTORY);
   assert(FT_rmFile("1root/2child/3nope") == NO_SUCH_PATH);
   assert(FT_rmFile("1root/2child/3gkid") == NOT_A_FILE);
   assert(FT_rmDir("1root/2child/3gkid") == SUCCESS);
   assert(FT_rmFile("1root/2second/3gfile") == SUCCESS);
   assert(FT_containsDir("1root/2child/3gkid") == FALSE);
   assert(FT_containsFile("1root/2second/3gfile") == FALSE);
   assert(FT_rmFile("1root/2child/2child/2child/2child") == SUCCESS);
   assert(FT_rmDir("1root/2child/2child") == SUCCESS);
   assert((temp = FT_toString()) != NULL);
   fprintf(stderr, "Checkpoint 3:\n%s\n", temp);
   free(temp);

   /* removing the root doesn't uninitialize the structure */
   assert(FT_rmDir("1anotherroot") == CONFLICTING_PATH);
   assert(FT_rmDir("1root") == SUCCESS);
   assert(FT_rmDir("1root") == NO_SUCH_PATH);
   assert(FT_containsDir("1root/2child") == FALSE);
   assert(FT_containsDir("1root") == FALSE);
   assert(FT_rmDir("1root") == NO_SUCH_PATH);
   assert(FT_rmDir("1anotherroot") == NO_SUCH_PATH);
   assert((temp = FT_toString()) != NULL);
   assert(!strcmp(temp, ""));
   free(temp);

   /* checking that file contents work as expected */
   assert(FT_insertDir("1root") == SUCCESS);
   assert(FT_insertFile("1root/H", "hello, world!",
                        strlen("hello, world!") + 1) == SUCCESS);
   assert(!strcmp(FT_getFileContents("1root/H"), "hello, world!"));
   bIsFile = FALSE;
   l = -1;
   assert(FT_stat("1root/H", &bIsFile, &l) == SUCCESS);
   assert(bIsFile == TRUE);
   assert(l == (strlen("hello, world!") + 1));
   assert(!strcmp(FT_replaceFileContents("1root/H", "Kernighan",
                                         strlen("Kernighan") + 1),
                  "hello, world!"));
   assert(!strcmp((char *)FT_getFileContents("1root/H"), "Kernighan"));
   assert(FT_stat("1root/H", &bIsFile, &l) == SUCCESS);
   assert(bIsFile == TRUE);
   assert(l == (strlen("Kernighan") + 1));
   assert(!strcmp(FT_replaceFileContents("1root/H", arr, ARRLEN),
                  "Kernighan"));
   assert(!strcmp((char *)FT_getFileContents("1root/H"), ""));
   assert(FT_stat("1root/H", &bIsFile, &l) == SUCCESS);
   assert(bIsFile == TRUE);
   assert(l == ARRLEN);
   assert(FT_rmFile("1root/H") == SUCCESS);
   assert(FT_insertDir("1root/2d") == SUCCESS);
   assert(FT_stat("1root/2d", &bIsFile, &l) == SUCCESS);
   assert(bIsFile == FALSE);
   assert(l == ARRLEN);
   assert(FT_stat("1root/H", &bIsFile, &l) == NO_SUCH_PATH);
   assert(bIsFile == FALSE);
   assert(l == ARRLEN);
   assert(FT_rmDir("1root") == SUCCESS);
   assert((temp = FT_toString()) != NULL);
   assert(!strcmp(temp, ""));
   free(temp);

   /* children should be printed in lexicographic order,
      depth first, file children before directory children */
   assert(FT_insertDir("1root/y") == SUCCESS);
   assert((temp = FT_toString()) != NULL);
   fprintf(stderr, "Checkpoint 4.1:\n%s\n", temp);
   free(temp);
   assert(FT_insertDir("1root/x") == SUCCESS);
   assert((temp = FT_toString()) != NULL);
   fprintf(stderr, "Checkpoint 4.2:\n%s\n", temp);
   free(temp);
   assert(FT_insertFile("1root/x/C", "Ritchie",
                        strlen("Ritchie") + 1) == SUCCESS);
   assert(FT_insertDir("1root/x/c++") == SUCCESS);
   assert((temp = FT_toString()) != NULL);
   fprintf(stderr, "Checkpoint 4.3:\n%s\n", temp);
   free(temp);
   assert(FT_insertFile("1root/x/B", "Thompson",
                        strlen("Thompson") + 1) == SUCCESS);
   assert((temp = FT_toString()) != NULL);
   fprintf(stderr, "Checkpoint 4.4:\n%s\n", temp);
   free(temp);
   assert(FT_insertDir("1root/y/CHILD1DIR") == SUCCESS);
   assert(FT_insertDir("1root/y/CHILD2DIR") == SUCCESS);
   assert(FT_insertFile("1root/y/CHILD2FILE", NULL, 0) == SUCCESS);
   assert(FT_insertDir("1root/y/CHILD3DIR") == SUCCESS);
   assert(FT_insertFile("1root/y/CHILD1FILE", NULL, 0) == SUCCESS);
   assert(FT_insertDir("1root/y/CHILD2DIR/CHILD4DIR") == SUCCESS);
   assert((temp = FT_toString()) != NULL);
   fprintf(stderr, "Checkpoint 4.5:\n%s\n", temp);
   free(temp);

   assert(FT_destroy() == SUCCESS);
   assert(FT_destroy() == INITIALIZATION_ERROR);
   assert(FT_containsDir("1root") == FALSE);
   assert(FT_containsFile("1root") == FALSE);
   assert((temp = FT_toString()) == NULL);

   return 0;
}
