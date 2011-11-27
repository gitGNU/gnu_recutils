/* -*- mode: C -*-
 *
 *       File:         recutl.h
 *       Date:         Thu Apr 22 17:29:52 2010
 *
 *       GNU recutils - Common code for the utilities.
 *
 */

/* Copyright (C) 2010, 2011 Jose E. Marchesi */

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

#ifndef RECUTL_H
#define RECUTL_H

#include <progname.h>

/*
 * Common arguments.
 */

#define COMMON_ARGS                             \
  HELP_ARG,                                     \
  VERSION_ARG

#define COMMON_LONG_ARGS                        \
  {"help", no_argument, NULL, HELP_ARG},        \
  {"version", no_argument, NULL, VERSION_ARG}

#define COMMON_ARGS_CASES                          \
  case HELP_ARG:                                   \
    {                                              \
      recutl_print_help ();                        \
      exit (EXIT_SUCCESS);                         \
      break;                                       \
    }                                              \
  case VERSION_ARG:                                \
    {                                              \
      recutl_print_version ();                     \
      exit (EXIT_SUCCESS);                         \
      break;                                       \
    }

/*
 * Record selection arguments.
 */

#define RECORD_SELECTION_ARGS                   \
  TYPE_ARG,                                     \
  EXPRESSION_ARG,                               \
  QUICK_ARG,                                    \
  NUM_ARG,                                      \
  INSENSITIVE_ARG

#define RECORD_SELECTION_LONG_ARGS                                     \
   {"type", required_argument, NULL, TYPE_ARG},                        \
   {"expression", required_argument, NULL, EXPRESSION_ARG},            \
   {"quick", required_argument, NULL, QUICK_ARG},                      \
   {"num", required_argument, NULL, NUM_ARG},                          \
   {"case-insensitive", no_argument, NULL, INSENSITIVE_ARG}

#define RECORD_SELECTION_SHORT_ARGS             \
   "it:e:n:q:"

#define RECORD_SELECTION_ARGS_CASES                            \
    case TYPE_ARG:                                             \
    case 't':                                                  \
      {                                                        \
        recutl_type = xstrdup (optarg);                        \
        if (!rec_field_name_part_str_p (recutl_type))          \
          {                                                    \
             recutl_fatal ("invalid record type %s\n",         \
                           recutl_type);                       \
          }                                                    \
        break;                                                 \
      }                                                        \
    case EXPRESSION_ARG:                                       \
    case 'e':                                                  \
      {                                                        \
        if (recutl_num != -1)                                  \
          {                                                    \
             fprintf (stderr,                                  \
                      "%s: cannot specify -e and also -n.\n",  \
                      program_name);                           \
             exit (EXIT_FAILURE);                              \
          }                                                    \
        if (recutl_quick_str)                                  \
          {                                                    \
             fprintf (stderr,                                  \
                      "%s: cannot specify -e and also -q.\n",  \
                      program_name);                           \
             exit (EXIT_FAILURE);                              \
          }                                                    \
                                                               \
         recutl_sex_str = xstrdup (optarg);                    \
                                                               \
         /* Compile the search expression.  */                 \
         if (recutl_sex_str)                                   \
          {                                                    \
            recutl_sex = rec_sex_new (recutl_insensitive);     \
            if (!rec_sex_compile (recutl_sex, recutl_sex_str)) \
             {                                                 \
                fprintf (stderr,                               \
                         "%s: error: invalid selection expression.\n",\
                         program_name);                        \
                exit (EXIT_FAILURE);                           \
             }                                                 \
          }                                                    \
                                                               \
         break;                                                \
      }                                                        \
      case NUM_ARG:                                            \
      case 'n':                                                \
      {                                                        \
         long int li;                                          \
         char *end;                                            \
         char *str;                                            \
                                                               \
         if (recutl_sex)                                       \
          {                                                    \
             fprintf (stderr,                                  \
                      "%s: cannot specify -n and also -e.\n",  \
                      program_name);                           \
             exit (EXIT_FAILURE);                              \
          }                                                    \
        if (recutl_quick_str)                                  \
          {                                                    \
             fprintf (stderr,                                  \
                      "%s: cannot specify -n and also -q.\n",  \
                      program_name);                           \
             exit (EXIT_FAILURE);                              \
          }                                                    \
                                                               \
          str = xstrdup (optarg);                              \
          li = strtol (str, &end, 10);                         \
          if ((*str != '\0') && (*end == '\0'))                \
            {                                                  \
              /* Valid number.  */                             \
              recutl_num = (int) li;                           \
            }                                                  \
          else                                                 \
            {                                                  \
              fprintf (stderr,                                 \
                       "%s: invalid number '%s' in -n.\n",     \
                       program_name, str);                     \
              exit (EXIT_FAILURE);                             \
            }                                                  \
          break;                                               \
      }                                                        \
      case QUICK_ARG:                                          \
      case 'q':                                                \
      {                                                        \
         if (recutl_sex)                                       \
          {                                                    \
             fprintf (stderr,                                  \
                      "%s: cannot specify -n and also -e.\n",  \
                      program_name);                           \
             exit (EXIT_FAILURE);                              \
          }                                                    \
        if (recutl_num != -1)                                  \
          {                                                    \
             fprintf (stderr,                                  \
                      "%s: cannot specify -e and also -n.\n",  \
                      program_name);                           \
             exit (EXIT_FAILURE);                              \
          }                                                    \
                                                               \
        recutl_quick_str = xstrdup (optarg);                   \
        break;                                                 \
      }                                                        \
      case INSENSITIVE_ARG:                                    \
      case 'i':                                                \
      {                                                        \
          recutl_insensitive = true;                           \
          break;                                               \
      }

/*
 * Function prototypes.
 */

void recutl_init (char *util_name);
void recutl_print_version (void);
void recutl_print_help_common (void);
void recutl_print_help_footer (void);
void recutl_print_help_record_selection (void);


void recutl_error (const char *fmt, ...);
void recutl_warning (const char *fmt, ...);
void recutl_fatal (const char *fmt, ...);

bool recutl_parse_db_from_file (FILE *in, char *file_name, rec_db_t db);
rec_db_t recutl_build_db (int argc, char **argv);
char *recutl_eval_field_expression (rec_fex_t fex,
                                    rec_record_t record,
                                    rec_writer_mode_t mode,
                                    bool print_values_p,
                                    bool print_in_a_row_p,
                                    char *password);

rec_db_t recutl_read_db_from_file (char *file_name);
void recutl_write_db_to_file (rec_db_t db, char *file_name);

char *recutl_read_file (char *file_name);

void recutl_check_integrity (rec_db_t db,
                             bool verbose_p,
                             bool external_p);

void recutl_sorting_parser (bool sort_p,
                            char *rset_name,
                            rec_field_name_t field_name);

bool recutl_yesno (char *prompt);

#endif /* recutl.h */

/* End of recutl.h */
