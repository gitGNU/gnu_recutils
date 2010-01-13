/* -*- mode: C -*- Time-stamp: "10/01/13 14:20:04 jemarch"
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

#include <regex.h>

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

  if (!*status)
    {
      res = false;
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

#define GET_CHILD_VAL(DEST,NUM)                                         \
  do                                                                    \
    {                                                                   \
      (DEST) = rec_sex_eval_node (sex,                                  \
                                  record,                               \
                                  rec_sex_ast_node_child (node, (NUM)), \
                                  status);                              \
      if (!*status)                                                     \
        {                                                               \
          return res;                                                   \
        }                                                               \
    }                                                                   \
    while (0)


#define ATOI_VAL(DEST, VAL)                     \
  do                                            \
    {                                           \
      switch ((VAL).type)                       \
        {                                       \
        case REC_SEX_VAL_INT:                   \
          {                                     \
            (DEST) = (VAL).int_val;             \
            break;                              \
          }                                     \
        case REC_SEX_VAL_STR:                   \
          {                                     \
            /* XXX.  Check for errors. */       \
            if (strcmp ((VAL).str_val, "") == 0)\
            {                                   \
              (DEST) = 0;                       \
            }                                   \
          else                                  \
            {                                   \
              (DEST) = atoi ((VAL).str_val);    \
            }                                   \
          break;                                \
          }                                     \
        }                                       \
    }                                           \
  while (0)

struct rec_sex_val_s
rec_sex_eval_node (rec_sex_t sex,
                   rec_record_t record,
                   rec_sex_ast_node_t node,
                   bool *status)
{
  int i;
  struct rec_sex_val_s res;
  rec_sex_ast_node_t child1;
  struct rec_sex_val_s child_val1;
  rec_sex_ast_node_t child2;
  struct rec_sex_val_s child_val2;

  *status = true;

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
      {
        int op1;
        int op2;

        GET_CHILD_VAL (child_val1, 0);
        GET_CHILD_VAL (child_val2, 1);

        ATOI_VAL (op1, child_val1);
        ATOI_VAL (op2, child_val2);

        res.type = REC_SEX_VAL_INT;
        res.int_val = op1 + op2;

        break;
      }
    case REC_SEX_OP_SUB:
      {
        int op1;
        int op2;

        GET_CHILD_VAL (child_val1, 0);
        GET_CHILD_VAL (child_val2, 1);

        ATOI_VAL (op1, child_val1);
        ATOI_VAL (op2, child_val2);

        res.type = REC_SEX_VAL_INT;
        res.int_val = op1 - op2;

        break;
      }
    case REC_SEX_OP_MUL:
      {
        int op1;
        int op2;

        GET_CHILD_VAL (child_val1, 0);
        GET_CHILD_VAL (child_val2, 1);

        ATOI_VAL (op1, child_val1);
        ATOI_VAL (op2, child_val2);

        res.type = REC_SEX_VAL_INT;
        res.int_val = op1 * op2;

        break;
      }
    case REC_SEX_OP_DIV:
      {
        int op1;
        int op2;

        GET_CHILD_VAL (child_val1, 0);
        GET_CHILD_VAL (child_val2, 1);

        ATOI_VAL (op1, child_val1);
        ATOI_VAL (op2, child_val2);

        res.type = REC_SEX_VAL_INT;
        res.int_val = op1 / op2;

        break;
      }
    case REC_SEX_OP_MOD:
      {
        int op1;
        int op2;

        GET_CHILD_VAL (child_val1, 0);
        GET_CHILD_VAL (child_val2, 1);

        ATOI_VAL (op1, child_val1);
        ATOI_VAL (op2, child_val2);

        res.type = REC_SEX_VAL_INT;
        res.int_val = op1 % op2;

        break;
      }
    case REC_SEX_OP_EQL:
      {
        int op1;
        int op2;

        GET_CHILD_VAL (child_val1, 0);
        GET_CHILD_VAL (child_val2, 1);

        if ((child_val1.type == REC_SEX_VAL_STR)
            && (child_val2.type == REC_SEX_VAL_STR))
          {
            /* String comparison.  */
            res.type = REC_SEX_VAL_INT;
            res.int_val = (strcmp (child_val1.str_val,
                                   child_val2.str_val) == 0);
          }
        else
          {
            /* Integer comparison.  */
            ATOI_VAL (op1, child_val1);
            ATOI_VAL (op2, child_val2);
            
            res.type = REC_SEX_VAL_INT;
            res.int_val = op1 == op2;
          }

        break;
      }
    case REC_SEX_OP_NEQ:
      {
        int op1;
        int op2;

        GET_CHILD_VAL (child_val1, 0);
        GET_CHILD_VAL (child_val2, 1);

        if ((child_val1.type == REC_SEX_VAL_STR)
            && (child_val2.type == REC_SEX_VAL_STR))
          {
            /* String comparison.  */
            res.type = REC_SEX_VAL_INT;
            res.int_val = (strcmp (child_val1.str_val,
                                   child_val2.str_val) != 0);
          }
        else
          {
            /* Integer comparison.  */
            ATOI_VAL (op1, child_val1);
            ATOI_VAL (op2, child_val2);
            
            res.type = REC_SEX_VAL_INT;
            res.int_val = op1 != op2;
          }

        break;
      }
    case REC_SEX_OP_MAT:
      {
        int op1;
        int op2;

        GET_CHILD_VAL (child_val1, 0);
        GET_CHILD_VAL (child_val2, 1);

        if ((child_val1.type == REC_SEX_VAL_STR)
            && (child_val2.type == REC_SEX_VAL_STR))
          {
            /* String match.  */
            regex_t regexp;
            int flags;

            flags = REG_EXTENDED;
            if (rec_sex_parser_case_insensitive (sex->parser))
              {
                flags |= REG_ICASE;
              }

            if (regcomp (&regexp,
                         child_val2.str_val,
                         flags) != 0)
              {
                *status = false;
                return res;
              }

            res.type = REC_SEX_VAL_INT;
            res.int_val = (regexec (&regexp,
                                    child_val1.str_val,
                                    0,
                                    NULL,
                                    0) == 0);
          }
        else
          {
            /* Error.  */
            *status = false;
            return res;
          }

        break;
      }
    case REC_SEX_OP_LT:
      {
        int op1;
        int op2;

        GET_CHILD_VAL (child_val1, 0);
        GET_CHILD_VAL (child_val2, 1);

        ATOI_VAL (op1, child_val1);
        ATOI_VAL (op2, child_val2);

        res.type = REC_SEX_VAL_INT;
        res.int_val = op1 < op2;

        break;
      }
    case REC_SEX_OP_GT:
      {
        int op1;
        int op2;

        GET_CHILD_VAL (child_val1, 0);
        GET_CHILD_VAL (child_val2, 1);

        ATOI_VAL (op1, child_val1);
        ATOI_VAL (op2, child_val2);

        res.type = REC_SEX_VAL_INT;
        res.int_val = op1 > op2;

        break;
      }
    case REC_SEX_OP_AND:
      {
        int op1;
        int op2;

        GET_CHILD_VAL (child_val1, 0);
        GET_CHILD_VAL (child_val2, 1);

        ATOI_VAL (op1, child_val1);
        ATOI_VAL (op2, child_val2);

        res.type = REC_SEX_VAL_INT;
        res.int_val = op1 && op2;

        break;
      }
    case REC_SEX_OP_OR:
      {
        int op1;
        int op2;

        GET_CHILD_VAL (child_val1, 0);
        GET_CHILD_VAL (child_val2, 1);

        ATOI_VAL (op1, child_val1);
        ATOI_VAL (op2, child_val2);

        res.type = REC_SEX_VAL_INT;
        res.int_val = op1 || op2;

        break;
      }
    case REC_SEX_OP_NOT:
      {
        int op;

        GET_CHILD_VAL (child_val1, 0);
        ATOI_VAL (op, child_val1);

        res.type = REC_SEX_VAL_INT;
        res.int_val = !op;

        break;
      }
    case REC_SEX_OP_SHA:
      {
        int i, n;
        rec_field_t field;
        rec_field_name_t field_name;
        char *field_name_str;
        rec_sex_ast_node_t child;

        /* The child should be a Name.  */
        child = rec_sex_ast_node_child (node, 0);
        if (rec_sex_ast_node_type (rec_sex_ast_node_child(node, 0))
            != REC_SEX_NAME)
          {
            *status = false;
            return res;
          }

        field_name_str = rec_sex_ast_node_name (child);
        field_name = rec_parse_field_name_str (field_name_str);

        n = 0;
        for (i = 0; i < rec_record_size (record); i++)
          {
            field = rec_record_get_field (record, i);
            if (rec_field_name_equal_p (field_name,
                                        rec_field_name (field)))
              {
                n++;
              }
          }

        res.type = REC_SEX_VAL_INT;
        res.int_val = n;
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
        rec_field_t field;
        rec_field_name_t field_name;
        char *field_name_str;
        int n;

        field_name_str = rec_sex_ast_node_name (node);
        field_name = rec_parse_field_name_str (field_name_str);
        /*        n = rec_sex_ast_node_index (node); */
        field = rec_record_get_field_by_name (record, field_name, 0);

        res.type = REC_SEX_VAL_STR;
        if (field)
          {
            res.str_val = rec_field_value (field);
          }
        else
          {
            /* No field => ""  */
            res.str_val = "";
          }

        break;
      }
    }

  return res;
}


 
/* End of rec-sex.c */
