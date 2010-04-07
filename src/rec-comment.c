/* -*- mode: C -*- Time-stamp: "2010-04-07 17:56:21 jco"
 *
 *       File:         rec-comment.c
 *       Date:         Wed Apr  7 17:22:45 2010
 *
 *       GNU recutils - Comments.
 *
 */

#include <config.h>

#include <stdlib.h>
#include <string.h>

#include <rec.h>

/*
 * Public functions.
 */

rec_comment_t
rec_comment_new (char *text)
{
  return strdup (text);
}

void
rec_comment_destroy (rec_comment_t comment)
{
  free (comment);
}

rec_comment_t
rec_comment_dup (rec_comment_t comment)
{
  return strdup (comment);
}

char *
rec_comment_text (rec_comment_t comment)
{
  return comment;
}

void
rec_comment_set_text (rec_comment_t comment,
                      char *text)
{
  free (comment);
  comment = strdup (text);
}

bool
rec_comment_equal_p (rec_comment_t comment1,
                     rec_comment_t comment2)
{
  return (strcmp (comment1, comment2) == 0);
}

/* End of rec-comment.c */
