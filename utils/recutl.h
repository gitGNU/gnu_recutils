/* -*- mode: C -*- Time-stamp: "2010-04-22 23:10:51 jemarch"
 *
 *       File:         recutl.h
 *       Date:         Thu Apr 22 17:29:52 2010
 *
 *       GNU recutils - Common code for the utilities.
 *
 */

#ifndef RECUTL_H
#define RECUTL_H

#define RECUTL_VERSION "0.1"

#define RECUTL_COPYRIGHT_DOC(NAME)              \
  char *recutl_version_msg =                    \
   NAME " (GNU recutils) " RECUTL_VERSION "\n\
Copyright (C) 2010 Jose E. Marchesi.\n\
License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>.\n\
This is free software: you are free to change and redistribute it.\n\
There is NO WARRANTY, to the extent permitted by law.\n\
\n\
Written by Jose E. Marchesi."

#define RECUTL_HELP_FOOTER_DOC(NAME)           \
  "Report " NAME " bugs to bug-recutils@gnu.org\n\
GNU recutils home page: <http://www.gnu.org/software/recutils/>\n\
General help using GNU software: <http://www.gnu.org/gethelp/>"

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
     fprintf (stdout, "%s\n", recutl_help_msg);    \
     exit (0);                                     \
     break;                                        \
   }                                               \
 case VERSION_ARG:                                 \
   {                                               \
     fprintf (stdout, "%s\n", recutl_version_msg); \
     exit (0);                                     \
     break;                                        \
   }

#define COMMON_ARGS_DOC                            \
  "      --help                          print a help message and exit.\n\
      --version                       show recsel version and exit.\n"  

/*
 * Record selection arguments.
 */

#define RECORD_SELECTION_ARGS                   \
  TYPE_ARG,                                     \
  EXPRESSION_ARG,                               \
  NUM_ARG,                                      \
  INSENSITIVE_ARG

#define RECORD_SELECTION_LONG_ARGS                                     \
   {"type", required_argument, NULL, TYPE_ARG},                        \
   {"expression", required_argument, NULL, EXPRESSION_ARG},            \
   {"num", required_argument, NULL, NUM_ARG},                          \
   {"case-insensitive", no_argument, NULL, INSENSITIVE_ARG}

#define RECORD_SELECTION_SHORT_ARGS             \
   "it:e:n:"

#define RECORD_SELECTION_ARGS_CASES                            \
    case TYPE_ARG:                                             \
    case 't':                                                  \
      {                                                        \
        recutl_type = strdup (optarg);                         \
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
             exit (1);                                         \
          }                                                    \
                                                               \
         recutl_sex_str = strdup (optarg);                     \
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
                exit (1);                                      \
             }                                                 \
          }                                                    \
                                                               \
         break;                                                \
      }                                                        \
      case NUM_ARG:                                            \
      case 'n':                                                \
      {                                                        \
         if (recutl_sex)                                       \
          {                                                    \
             fprintf (stderr,                                  \
                      "%s: cannot specify -n and also -e.\n",  \
                      program_name);                           \
             exit (1);                                         \
          }                                                    \
                                                               \
          /* XXX: check for conversion errors.  */             \
          recutl_num = atoi (optarg);                          \
          break;                                               \
      }                                                        \
      case INSENSITIVE_ARG:                                    \
      case 'i':                                                \
      {                                                        \
          recutl_insensitive = true;                           \
          break;                                               \
      }

#define RECORD_SELECTION_ARGS_DOC               \
  "Record selection options:\n\
  -i, --case-insensitive              make strings case-insensitive in selection\n\
                                        expressions.\n\
  -t, --type=TYPE                     print records of the specified type only.\n\
  -e, --expression=EXPR               selection expression.\n\
  -n, --number=NUM                    select an specific record.\n"   

/*
 * Function prototypes.
 */

bool recutl_parse_db_from_file (FILE *in, char *file_name, rec_db_t db);
rec_db_t recutl_build_db (int argc, char **argv);
char *recutl_eval_field_expression (rec_fex_t fex, rec_record_t record, bool print_values_p);

#endif /* recutl.h */

/* End of recutl.h */
