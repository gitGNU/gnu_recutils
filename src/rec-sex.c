/* -*- mode: C -*-
 *
 *       File:         rec-sex.c
 *       Date:         Sat Jan  9 20:28:43 2010
 *
 *       GNU recutils - Record Selection Expressions.
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

#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <getdate.h>

#include <rec.h>
#include <rec-utils.h>
#include <rec-sex-ast.h>
#include <rec-sex-parser.h>
#include <rec-sex-tab.h>

/*
 * Data structures
 */

struct rec_sex_s
{
  rec_sex_ast_t ast;
  rec_sex_parser_t parser;
};

#define REC_SEX_VAL_INT  0
#define REC_SEX_VAL_REAL 1
#define REC_SEX_VAL_STR  2

struct rec_sex_val_s
{
  int type;

  int int_val;
  double real_val;
  char *str_val;
};

/* Static functions declarations.  */
static struct rec_sex_val_s rec_sex_eval_node (rec_sex_t sex,
                                               rec_record_t record,
                                               rec_sex_ast_node_t node,
                                               bool *status);
static bool rec_sex_op_real_p (struct rec_sex_val_s op1,
                               struct rec_sex_val_s op2);
static int timespec_subtract (struct timespec *result,
                              struct timespec *x,
                              struct timespec *y);

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

#define EXEC_AST(RECORD)                                                \
  do                                                                    \
    {                                                                   \
      val = rec_sex_eval_node (sex,                                     \
                               (RECORD),                                \
                               rec_sex_ast_top (sex->ast),              \
                               status);                                 \
                                                                        \
      switch (val.type)                                                 \
        {                                                               \
        case REC_SEX_VAL_INT:                                           \
          {                                                             \
            res = (val.int_val != 0);                                   \
            break;                                                      \
          }                                                             \
        case REC_SEX_VAL_REAL:                                          \
        case REC_SEX_VAL_STR:                                           \
          {                                                             \
            res = false;                                                \
            break;                                                      \
          }                                                             \
        }                                                               \
    }                                                                   \
  while (0)



bool
rec_sex_eval (rec_sex_t sex,
              rec_record_t record,
              bool *status)
{
  bool res;
  rec_field_t field;
  rec_field_t wfield;
  rec_record_t wrec;
  rec_record_elem_t elem_field;
  int j, nf;
  struct rec_sex_val_s val;
  
  res = false;
  wrec = NULL;

  rec_sex_ast_node_unfix (rec_sex_ast_top (sex->ast));
  EXEC_AST (record);
  if (res)
    {
      goto exit;
    }

  elem_field = rec_record_null_elem ();
  while (rec_record_elem_p (elem_field = rec_record_next_field (record, elem_field)))
    {
      field = rec_record_elem_field (elem_field);

      nf = rec_record_get_num_fields_by_name (record, rec_field_name (field));
      if (nf > 1)
        /* XXX.  Optimization: && (rec_sex_ast_name_p (sex->ast, field_name_str))) */
        {
          for (j = 0; j < nf; j++)
            {
              wfield = rec_record_get_field_by_name (record,
                                                     rec_field_name (field),
                                                     j);
              if (wrec)
                {
                  rec_record_destroy (wrec);
                }

              wrec = rec_record_dup (record);
              rec_record_remove_field_by_name (wrec,
                                               rec_field_name (field),
                                               -1); /* Delete all.  */
              rec_record_append_field (wrec, rec_field_dup (wfield));

              EXEC_AST(wrec);

              if (res)
                {
                  rec_record_destroy (wrec);
                  goto exit;
                }
            }
        }
    }

 exit:          

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


#define ATOI_VAL(DEST, VAL)                             \
  do                                                    \
    {                                                   \
      switch ((VAL).type)                               \
        {                                               \
        case REC_SEX_VAL_INT:                           \
          {                                             \
            (DEST) = (VAL).int_val;                     \
            break;                                      \
          }                                             \
        case REC_SEX_VAL_STR:                           \
          {                                             \
          if (strcmp ((VAL).str_val, "") == 0)          \
              {                                         \
                (DEST) = 0;                             \
              }                                         \
            else                                        \
              {                                         \
                if (!rec_atoi ((VAL).str_val, &(DEST))) \
                {                                       \
                  *status = false;                      \
                  return res;                           \
                }                                       \
              }                                         \
          break;                                        \
        }                                               \
    }                                                   \
  }                                                     \
  while (0)

#define ATOD_VAL(DEST, VAL)                             \
  do                                                    \
    {                                                   \
      switch ((VAL).type)                               \
        {                                               \
        case REC_SEX_VAL_REAL:                          \
          {                                             \
            (DEST) = (VAL).real_val;                    \
            break;                                      \
          }                                             \
        case REC_SEX_VAL_INT:                           \
          {                                             \
            (DEST) = (VAL).int_val;                     \
            break;                                      \
          }                                             \
        case REC_SEX_VAL_STR:                           \
          {                                             \
          if (strcmp ((VAL).str_val, "") == 0)          \
              {                                         \
                (DEST) = 0.0;                           \
              }                                         \
            else                                        \
              {                                         \
                if (!rec_atod ((VAL).str_val, &(DEST))) \
                {                                       \
                  *status = false;                      \
                  return res;                           \
                }                                       \
              }                                         \
          break;                                        \
        }                                               \
    }                                                   \
  }                                                     \
  while (0)

#define ATOTS_VAL(DEST, VAL)                            \
  do                                                    \
    {                                                   \
      switch ((VAL).type)                               \
        {                                               \
        case REC_SEX_VAL_REAL:                          \
          {                                             \
           *status = false;                             \
           return res;                                  \
           break;                                       \
          }                                             \
        case REC_SEX_VAL_INT:                           \
          {                                             \
            *status = false;                            \
            return res;                                 \
            break;                                      \
          }                                             \
        case REC_SEX_VAL_STR:                           \
          {                                             \
            if (!get_date (&(DEST), (VAL).str_val, NULL))\
            {                                           \
              *status = false;                          \
              return res;                               \
            }                                           \
                                                        \
            break;                                      \
          }                                             \
        }                                               \
    }                                                   \
  while (0)

struct rec_sex_val_s
rec_sex_eval_node (rec_sex_t sex,
                   rec_record_t record,
                   rec_sex_ast_node_t node,
                   bool *status)
{
  struct rec_sex_val_s res = {0, 0, 0, NULL};
  struct rec_sex_val_s child_val1 = {0, 0, 0, NULL};
  struct rec_sex_val_s child_val2 = {0, 0, 0, NULL};

  *status = true;

  switch (rec_sex_ast_node_type (node))
    {
    case REC_SEX_NOVAL:
      {
        fprintf (stderr, "Application bug: REC_SEX_NOVAL node found.\nPlease report this!\n");
        exit (EXIT_FAILURE);
        break;
      }
      /* Operations.  */
    case REC_SEX_OP_NEG:
    case REC_SEX_OP_ADD:
      {
        int op1;
        int op2;
        double op1_real;
        double op2_real;

        GET_CHILD_VAL (child_val1, 0);
        GET_CHILD_VAL (child_val2, 1);

        if (rec_sex_op_real_p (child_val1, child_val2))
          {
            /* Real operation.  */
            ATOD_VAL (op1_real, child_val1);
            ATOD_VAL (op2_real, child_val2);

            res.type = REC_SEX_VAL_REAL;
            res.real_val = op1_real + op2_real;
          }
        else
          {
            /* Integer operation.  */
            ATOI_VAL (op1, child_val1);
            ATOI_VAL (op2, child_val2);

            res.type = REC_SEX_VAL_INT;
            res.int_val = op1 + op2;
          }

        break;
      }
    case REC_SEX_OP_SUB:
      {
        int op1;
        int op2;
        double op1_real;
        double op2_real;

        GET_CHILD_VAL (child_val1, 0);
        GET_CHILD_VAL (child_val2, 1);

        if (rec_sex_op_real_p (child_val1, child_val2))
          {
            /* Real operation.  */
            ATOD_VAL (op1_real, child_val1);
            ATOD_VAL (op2_real, child_val2);

            res.type = REC_SEX_VAL_REAL;
            res.real_val = op1 - op2;
          }
        else
          {
            /* Integer operation.  */

            ATOI_VAL (op1, child_val1);
            ATOI_VAL (op2, child_val2);

            res.type = REC_SEX_VAL_INT;
            res.int_val = op1 - op2;
          }

        break;
      }
    case REC_SEX_OP_MUL:
      {
        int op1;
        int op2;
        double op1_real;
        double op2_real;

        GET_CHILD_VAL (child_val1, 0);
        GET_CHILD_VAL (child_val2, 1);

        if (rec_sex_op_real_p (child_val1, child_val2))
          {
            /* Real operation.  */
            ATOD_VAL (op1_real, child_val1);
            ATOD_VAL (op2_real, child_val2);

            res.type = REC_SEX_VAL_REAL;
            res.real_val = op1_real * op2_real;
          }
        else
          {
            /* Integer operation.  */
            ATOI_VAL (op1, child_val1);
            ATOI_VAL (op2, child_val2);
            
            res.type = REC_SEX_VAL_INT;
            res.int_val = op1 * op2;
          }

        break;
      }
    case REC_SEX_OP_DIV:
      {
        int op1;
        int op2;
        double op1_real;
        double op2_real;

        GET_CHILD_VAL (child_val1, 0);
        GET_CHILD_VAL (child_val2, 1);

        if (rec_sex_op_real_p (child_val1, child_val2))
          {
            /* Real operation.  */
            ATOD_VAL (op1_real, child_val1);
            ATOD_VAL (op2_real, child_val2);

            res.type = REC_SEX_VAL_REAL;
            res.real_val = op1_real / op2_real;
          }
        else
          {
            /* Integer operation.  */
            ATOI_VAL (op1, child_val1);
            ATOI_VAL (op2, child_val2);
            
            res.type = REC_SEX_VAL_INT;
            res.int_val = op1 / op2;
          }

        break;
      }
    case REC_SEX_OP_MOD:
      {
        int op1;
        int op2;

        GET_CHILD_VAL (child_val1, 0);
        GET_CHILD_VAL (child_val2, 1);

        /* Integer operation.  */
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
        double op1_real;
        double op2_real;

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
            if (rec_sex_op_real_p (child_val1, child_val2))
              {
                /* Real comparison.  */
                ATOD_VAL (op1_real, child_val1);
                ATOD_VAL (op2_real, child_val2);

                res.type = REC_SEX_VAL_INT;
                res.int_val = op1_real == op2_real;
              }
            else
              {
                /* Integer comparison.  */
                ATOI_VAL (op1, child_val1);
                ATOI_VAL (op2, child_val2);
                
                res.type = REC_SEX_VAL_INT;
                res.int_val = op1 == op2;
              }
          }

        break;
      }
    case REC_SEX_OP_NEQ:
      {
        int op1;
        int op2;
        double op1_real;
        double op2_real;

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
            if (rec_sex_op_real_p (child_val1, child_val2))
              {
                /* Real comparison.  */
                ATOD_VAL (op1_real, child_val1);
                ATOD_VAL (op2_real, child_val2);

                res.type = REC_SEX_VAL_INT;
                res.int_val = op1_real != op2_real;
              }
            else
              {
                /* Integer comparison.  */
                ATOI_VAL (op1, child_val1);
                ATOI_VAL (op2, child_val2);
            
                res.type = REC_SEX_VAL_INT;
                res.int_val = op1 != op2;
              }
          }

        break;
      }
    case REC_SEX_OP_MAT:
      {
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

            regfree (&regexp);
          }
        else
          {
            /* Error.  */
            *status = false;
            return res;
          }

        break;
      }
    case REC_SEX_OP_BEFORE:
      {
        struct timespec op1;
        struct timespec op2;
        struct timespec diff;

        GET_CHILD_VAL (child_val1, 0);
        GET_CHILD_VAL (child_val2, 1);

        ATOTS_VAL (op1, child_val1);
        ATOTS_VAL (op2, child_val2);

        res.type = REC_SEX_VAL_INT;
        res.int_val = timespec_subtract (&diff, &op1, &op2);
        break;
      }
    case REC_SEX_OP_AFTER:
      {
        struct timespec op1;
        struct timespec op2;
        struct timespec diff;

        GET_CHILD_VAL (child_val1, 0);
        GET_CHILD_VAL (child_val2, 1);

        ATOTS_VAL (op1, child_val1);
        ATOTS_VAL (op2, child_val2);

        res.type = REC_SEX_VAL_INT;
        res.int_val = (!timespec_subtract (&diff, &op1, &op2)
                       && ((diff.tv_sec != 0) || (diff.tv_nsec != 0)));
        break;
      }
    case REC_SEX_OP_SAMETIME:
      {
        struct timespec op1;
        struct timespec op2;
        struct timespec diff;

        GET_CHILD_VAL (child_val1, 0);
        GET_CHILD_VAL (child_val2, 1);

        ATOTS_VAL (op1, child_val1);
        ATOTS_VAL (op2, child_val2);

        timespec_subtract (&diff, &op1, &op2);

        res.type = REC_SEX_VAL_INT;
        res.int_val = ((diff.tv_sec == 0) && (diff.tv_nsec == 0));
        break;
      }
    case REC_SEX_OP_LT:
      {
        int op1;
        int op2;
        double op1_real;
        double op2_real;

        GET_CHILD_VAL (child_val1, 0);
        GET_CHILD_VAL (child_val2, 1);

        if (rec_sex_op_real_p (child_val1, child_val2))
          {
            /* Real comparison.  */
            ATOD_VAL (op1_real, child_val1);
            ATOD_VAL (op2_real, child_val2);

            res.type = REC_SEX_VAL_INT;
            res.int_val = op1_real < op2_real;
          }
        else
          {
            /* Integer comparison.  */
            ATOI_VAL (op1, child_val1);
            ATOI_VAL (op2, child_val2);
            
            res.type = REC_SEX_VAL_INT;
            res.int_val = op1 < op2;
          }

        break;
      }
    case REC_SEX_OP_GT:
      {
        int op1;
        int op2;
        double op1_real;
        double op2_real;

        GET_CHILD_VAL (child_val1, 0);
        GET_CHILD_VAL (child_val2, 1);

        if (rec_sex_op_real_p (child_val1, child_val2))
          {
            /* Real comparison.  */
            ATOD_VAL (op1_real, child_val1);
            ATOD_VAL (op2_real, child_val2);

            res.type = REC_SEX_VAL_INT;
            res.int_val = op1_real > op2_real;
          }
        else
          {
            /* Integer comparison.  */
            ATOI_VAL (op1, child_val1);
            ATOI_VAL (op2, child_val2);
            
            res.type = REC_SEX_VAL_INT;
            res.int_val = op1 > op2;
          }

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
        int n;
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

        n = rec_record_get_num_fields_by_name (record, field_name);

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
    case REC_SEX_REAL:
      {
        res.type = REC_SEX_VAL_REAL;
        res.real_val = rec_sex_ast_node_real (node);
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
        int index;
        bool tofix;

        if (rec_sex_ast_node_fixed (node))
          {
            res.type = REC_SEX_VAL_STR;
            res.str_val = rec_sex_ast_node_fixed_val (node);
          }
        else
          {
            field_name_str = rec_sex_ast_node_name (node);
            index = rec_sex_ast_node_index (node);
            tofix = (index != -1);
            if (index == -1)
              {
                index = 0;
              }

            field_name = rec_parse_field_name_str (field_name_str);
            field = rec_record_get_field_by_name (record, field_name, index);

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

            if (tofix)
              {
                /* Make this node fixed.  */
                rec_sex_ast_node_fix (node, res.str_val);
              }
          }

        break;
      }
    }

  return res;
}

static bool
rec_sex_op_real_p (struct rec_sex_val_s op1,
                   struct rec_sex_val_s op2)
{
  bool ret;
  int integer;
  double real;

  ret = true;

  if ((op1.type == REC_SEX_VAL_INT)
      || ((op1.type == REC_SEX_VAL_STR)
          && rec_atoi (op1.str_val, &integer)))
    {
      /* Operand 1 is an integer.  */
      switch (op2.type)
        {
        case REC_SEX_VAL_INT:
          {
            ret = false;
            break;
          }
        case REC_SEX_VAL_REAL:
          {
            ret = true;
            break;
          }
        case REC_SEX_VAL_STR:
          {
            ret = (rec_atod (op2.str_val, &real)
                   && (!rec_atoi (op2.str_val, &integer)));
            break;
          }
        default:
          {
            ret = false;
            break;
          }
        }
    }

  if ((op1.type == REC_SEX_VAL_REAL)
      || ((op1.type == REC_SEX_VAL_STR)
          && rec_atod (op1.str_val, &real)
          && (!rec_atoi (op1.str_val, &integer))))
    {
      /* Operand 1 is a real.  */
      switch (op2.type)
        {
        case REC_SEX_VAL_INT:
          {
            ret = true;
            break;
          }
        case REC_SEX_VAL_REAL:
          {
            ret = true;
            break;
          }
        case REC_SEX_VAL_STR:
          {
            ret = rec_atod (op2.str_val, &real);
            break;
          }
        default:
          {
            ret = false;
            break;
          }
        }
    }

  return ret;
}

static int
timespec_subtract (struct timespec *result,
                   struct timespec *x,
                   struct timespec *y)
{
  result->tv_sec = x->tv_sec - y->tv_sec;
  result->tv_nsec = x->tv_nsec - y->tv_nsec;
  if (result->tv_nsec < 0)
    {
      /* Overflow.  Subtract one second.  */
      result->tv_sec--;
      result->tv_nsec += 1000000000;
    }

  /* Return whether there is an overflow in the 'tv_sec' field.  */
  return (result->tv_sec < 0);
}


#if 0
}
  /* Perform the carry for the later subtraction by updating Y. */
  if (x->tv_usec < y->tv_usec)
    {
      int nsec = (y->tv_usec - x->tv_usec) / 1000000 + 1;
      y->tv_usec -= 1000000 * nsec;
      y->tv_sec += nsec;
    }
  if (x->tv_usec - y->tv_usec > 1000000)
    {
      int nsec = (x->tv_usec - y->tv_usec) / 1000000;
      y->tv_usec += 1000000 * nsec;
      y->tv_sec -= nsec;
    }

  /* Compute the time remaining to wait.
     `tv_usec' is certainly positive. */
  result->tv_sec = x->tv_sec - y->tv_sec;
  result->tv_usec = x->tv_usec - y->tv_usec;
  
  /* Return 1 if result is negative. */
  return x->tv_sec < y->tv_sec;
}
#endif /* 0 */
 
/* End of rec-sex.c */
