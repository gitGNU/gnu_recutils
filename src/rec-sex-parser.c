/* -*- mode: C -*- Time-stamp: "10/01/14 15:29:05 jemarch"
 *
 *       File:         rec-sex-parser.c
 *       Date:         Tue Jan 12 18:01:37 2010
 *
 *       GNU Record Utilities - Sexy parser
 *
 */

#include <config.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <rec-sex-parser.h>

/*
 * Data types
 */

struct rec_sex_parser_s
{
  char *in;              /* String to be parsed.  */
  size_t index;          /* Index in in_str.  */
  void *scanner;         /* Flex scanner.  */
  bool case_insensitive;

  rec_sex_ast_t ast;
};

/*
 * Public functions
 */

rec_sex_parser_t
rec_sex_parser_new (void)
{
  rec_sex_parser_t new;

  new = malloc (sizeof (struct rec_sex_parser_s));
  if (new)
    {
      new->in = NULL;
      new->index = 0;
      new->case_insensitive = false;

      /* Initialize the sexy scanner.  */
      sexlex_init (&(new->scanner));
      sexset_extra (new, new->scanner);
    }

  return new;
}

void *
rec_sex_parser_scanner (rec_sex_parser_t parser)
{
  return parser->scanner;
}

void
rec_sex_parser_destroy (rec_sex_parser_t parser)
{
  if (parser->scanner)
    {
      sexlex_destroy (parser->scanner);
    }

  if (parser->in)
    {
      free (parser->in);
    }

  free (parser);
}

rec_sex_ast_t
rec_sex_parser_ast (rec_sex_parser_t parser)
{
  return parser->ast;
}

void
rec_sex_parser_set_ast (rec_sex_parser_t parser,
                        rec_sex_ast_t ast)
{
  parser->ast = ast;
}

bool
rec_sex_parser_case_insensitive (rec_sex_parser_t parser)
{
  return parser->case_insensitive;
}

void
rec_sex_parser_set_case_insensitive (rec_sex_parser_t parser,
                                     bool case_insensitive)
{
  parser->case_insensitive = case_insensitive;
}

void
rec_sex_parser_set_in (rec_sex_parser_t parser,
                       char *str)
{
  if (parser->in)
    {
      free (parser->in);
      parser->in = NULL;
    }

  parser->in = strdup (str);
  parser->index = 0;
}

int
rec_sex_parser_getc (rec_sex_parser_t parser)
{
  int res;

  res = -1;
  if ((parser->in)
      && (parser->index < strlen (parser->in)))
    {
      res = parser->in[parser->index++];
    }

  return res;
}

bool
rec_sex_parser_run (rec_sex_parser_t parser,
                    char *expr)
{
  int res;

  rec_sex_parser_set_in (parser, expr);
  if (!sexparse (parser))
    {
      res = true;
    }
  else
    {
      /* Parse error.  */
      printf ("Parse error in selection expression.\n");
      res = false;
    }

  return res;
}

void
rec_sex_parser_print_ast (rec_sex_parser_t parser)
{
  rec_sex_ast_print (parser->ast);
}

/* End of rec-sex-parser.c */
