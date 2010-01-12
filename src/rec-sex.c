/* -*- mode: C -*- Time-stamp: "10/01/12 23:39:16 jemarch"
 *
 *       File:         rec-sex.c
 *       Date:         Sat Jan  9 20:28:43 2010
 *
 *       GNU Records - Sexy expressions
 *
 */

/* Copyright (C) 2010 Jose E. Marchesi */

/* This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <config.h>

#include <rec.h>

#include <rec-sex-ast.h>
#include <rec-sex-parser.h>

#include <rec-sex.tab.h>
#include <rec-sex.lex.h>

/*
 * Data structures
 */

struct rec_sex_s
{
  rec_sex_ast_t ast;
  rec_sex_parser_t parser;
};

#define REC_SEX_VAL_INT 0
#define REC_SEX_VAL_STR 1

struct rec_sex_val_s
{
  int type;

  int int_val;
  char *str_val;
};

/* Static functions declarations.  */
static struct rec_sex_val_s rec_sex_eval_node (rec_sex_t sex,
                                               rec_record_t record,
                                               rec_sex_ast_node_t node,
                                               bool *status);

/*
 * Public functions.
 */

rec_sex_t
rec_sex_new (bool case_insensitive)
{
  rec_sex_t new;

  new = malloc (sizeof (struct rec_sex_s));
  if (new)
    {
      /* Initialize a new parser.  */
      new->parser = rec_sex_parser_new ();
      rec_sex_parser_set_case_insensitive (new->parser,
                                           case_insensitive);

      /* Initialize a new AST.  */
      new->ast = NULL;
    }

  return new;
}

void
rec_sex_destroy (rec_sex_t sex)
{
  if (sex->parser)
    {
      rec_sex_parser_destroy (sex->parser);
    }

  if (sex->ast)
    {
      rec_sex_ast_destroy (sex->ast);
    }

  free (sex);  /* yeah! :D */
}

bool
rec_sex_compile (rec_sex_t sex,
                 char *expr)
{
  bool res;

  res = rec_sex_parser_run (sex->parser, expr);
  sex->ast = rec_sex_parser_ast (sex->parser);
  return res;
}

bool
rec_sex_eval (rec_sex_t sex,
              rec_record_t record,
              bool *status)
{
  bool res;
  struct rec_sex_val_s val;

  val = rec_sex_eval_node (sex,
                           record,
                           rec_sex_ast_top (sex->ast),
                           status);

  switch (val.type)
    {
    case REC_SEX_VAL_INT:
      {
        res = (val.int_val != 0);
        break;
      }
    case REC_SEX_VAL_STR:
      {
        res = (strcmp (val.str_val, "") != 0);
        break;
      }
    }

  return res;
}

void
rec_sex_print_ast (rec_sex_t sex)
{
  rec_sex_parser_print_ast (sex->parser);
}

/*
 * Private functions.
 */

struct rec_sex_val_s
rec_sex_eval_node (rec_sex_t sex,
                   rec_record_t record,
                   rec_sex_ast_node_t node,
                   bool *status)
{
  int i;
  struct rec_sex_val_s res;

  for (i = 0; i < rec_sex_ast_node_num_children (node); i++)
    {
      rec_sex_eval_node (sex,
                         record,
                         rec_sex_ast_node_child (node, i),
                         status);

      if (!*status)
        {
          /* Error: roll back!  */
          *status = false;
          return res;
        }
    }

  switch (rec_sex_ast_node_type (node))
    {
    case REC_SEX_NOVAL:
      {
        fprintf (stderr, "Application bug: REC_SEX_NOVAL node found.\nPlease report this!\n");
        *status = false;
        break;
      }
      /* Operations.  */
    case REC_SEX_OP_NEG:
    case REC_SEX_OP_ADD:
    case REC_SEX_OP_SUB:
    case REC_SEX_OP_MUL:
    case REC_SEX_OP_DIV:
    case REC_SEX_OP_MOD:
    case REC_SEX_OP_EQL:
    case REC_SEX_OP_NEQ:
    case REC_SEX_OP_MAT:
    case REC_SEX_OP_LT:
    case REC_SEX_OP_GT:
    case REC_SEX_OP_AND:
    case REC_SEX_OP_OR:
    case REC_SEX_OP_NOT:
    case REC_SEX_OP_SHA:
      {
        break;
      }
      /* Values.  */
    case REC_SEX_INT:
      {
        res.type = REC_SEX_VAL_INT;
        res.int_val = rec_sex_ast_node_int (node);
        break;
      }
    case REC_SEX_STR:
      {
        res.type = REC_SEX_VAL_STR;
        res.str_val = rec_sex_ast_node_str (node);
        break;
      }
    case REC_SEX_NAME:
      {
        
        break;
      }
    }

  return res;
}


 
/* End of rec-sex.c */
