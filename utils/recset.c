/* -*- mode: C -*-
 *
 *       File:         recset.c
 *       Date:         Fri Apr  9 17:06:39 2010
 *
 *       GNU recutils - recset
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

#include <getopt.h>
#include <string.h>
#include <stdlib.h>

#include <rec.h>
#include <recset.h>

/*
 * Forward prototypes.
 */

static bool recset_parse_db_from_file (FILE *in, char *file_name, rec_db_t db);
static void recset_parse_args (int argc, char **argv);
static void recset_process_actions (rec_db_t db);

/*
 * Global variables.
 */

char *program_name; /* Initialized in main().  */

/*
 * Command line options management
 */

static const struct option GNU_longOptions[] =
  {
    {"help", no_argument, NULL, HELP_ARG},
    {"version", no_argument, NULL, VERSION_ARG},
    {"fields", required_argument, NULL, FIELD_EXPR_ARG},
    {"expression", required_argument, NULL, EXPRESSION_ARG},
    {"append", required_argument, NULL, APPEND_ACTION_ARG},
    {"delete", no_argument, NULL, DELETE_ACTION_ARG},
    {"comment", no_argument, NULL, COMMENT_ACTION_ARG},
    {"set", required_argument, NULL, SET_ACTION_ARG},
    {"case-insensitive", required_argument, NULL, CASE_INSENSITIVE_ARG},
    {"num", required_argument, NULL, NUM_ARG},
    {"type", required_argument, NULL, TYPE_ARG},
    {NULL, 0, NULL, 0}
  };

/* Messages */

char *recset_version_msg = "recset (GNU recutils) 1.0\n\
Copyright (C) 2010 Jose E. Marchesi.\n\
License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>.\n\
This is free software: you are free to change and redistribute it.\n\
There is NO WARRANTY, to the extent permitted by law.\n\
\n\
Written by Jose E. Marchesi.";

char *recset_help_msg = "\
Usage: recset [OPTION]... [FILE]...\n\
Alter or delete fields in records.\n\
\n\
Mandatory arguments to long options are mandatory for short options too.\n\
  -i, --case-insensitive              make strings case-insensitive in selection\n\
      --help                          print a help message and exit.\n\
      --version                       show recset version and exit.\n\
\n\
Record selection options:\n\
  -t, --type=TYPE                     print records of the specified type only.\n\
  -e, --expression=EXPR               selection expression.\n\
  -n, --number=NUM                    select an specific record.\n\
\n\
Fields selection options:\n\
  -f, --fields=FIELDS                 comma-separated list of field names with optional\n\
                                        subscripts.\n\
Actions:\n\
  -s, --set=VALUE                     change the value of the selected fields.\n\
  -a, --append=VALUE                  append the selected fields with the given value.\n\
  -d, --delete                        delete the selected fields.\n\
  -c, --comment                       comment out the selected fields.\n\
\n\
Examples:\n\
\n\
        recset -f TmpName -d data.rec\n\
        recset -f Email[1] -s invalid@email.com friends.rec\n\
        recset -e \"Name ~ 'Smith'\" -f Email -a new@email.com friends.rec\n\
\n\
Report recset bugs to bug-recutils@gnu.org\n\
GNU recutils home page: <http://www.gnu.org/software/recutils/>\n\
General help using GNU software: <http://www.gnu.org/gethelp/>\
";

/* String containing the selection expression.  */
char *recset_sex_str = NULL;
rec_sex_t recset_sex = NULL;

/* Field expression.  */
char *recset_fex_str = NULL;
rec_fex_t recset_fex = NULL;

/* Record type.  */
char *recset_type = NULL;

/* Action.  */

#define RECSET_ACT_NONE    0
#define RECSET_ACT_SET     1
#define RECSET_ACT_APPEND  2
#define RECSET_ACT_DELETE  3
#define RECSET_ACT_COMMENT 4

int recset_action = RECSET_ACT_NONE;

/* Field value.  */

char *recset_value = NULL;

/* Whether to be case-insensitive while evaluating selection
   expressions.  */
bool recset_insensitive = false;

/* Whether to process an specific record.  */
long recset_index = -1;


bool
recset_parse_db_from_file (FILE *in,
                           char *file_name,
                           rec_db_t db)
{
  bool res;
  rec_rset_t rset;
  rec_parser_t parser;

  res = true;
  parser = rec_parser_new (in);

  while (rec_parse_rset (parser, &rset))
    {
      char *rset_type;
      rset_type = rec_rset_type (rset);
      if (rec_db_type_p (db, rset_type))
        {
          fprintf (stderr, "recset: error: duplicated record set '%s' from %s.\n",
                   rset_type, file_name ? file_name : "stdin");
          exit (1);
        }

      if (!rec_db_insert_rset (db, rset, rec_db_size (db)))
        {
          /* Error.  */
          res = false;
          break;
        }
    }

  if (rec_parser_error (parser))
    {
      res = false;
      rec_parser_perror (parser, "stdin");
    }
  
  return res;
}

void
recset_parse_args (int argc,
                   char **argv)
{
  int ret;
  char c;

  while ((ret = getopt_long (argc,
                             argv,
                             "idce:n:t:s:a:f:",
                             GNU_longOptions,
                             NULL)) != -1)
    {
      c = ret;
      switch (c)
        {
        case HELP_ARG:
          {
            fprintf (stdout, "%s\n", recset_help_msg);
            exit (0);
            break;
          }
        case VERSION_ARG:
          {
            fprintf (stdout, "%s\n", recset_version_msg);
            exit (0);
            break;
          }
        case FIELD_EXPR_ARG:
        case 'f':
          {
            recset_fex_str = strdup (optarg);
            if (!rec_fex_check (recset_fex_str))
              {
                exit (1);
              }

            /* Create the field expressin.  */
            recset_fex = rec_fex_new (recset_fex_str);
            if (!recset_fex)
              {
                fprintf (stderr, "%s: internal error: creating the field expression.\n", argv[0]);
                exit (1);
              }

            break;
          }
        case EXPRESSION_ARG:
        case 'e':
        default:
          {
            if (recset_index != -1)
              {
                fprintf (stderr, "%s: cannot specify -e and also -n.\n",
                         argv[0]);
                exit (1);
              }

            recset_sex_str = strdup (optarg);

            /* Compile the search expression.  */
            if (recset_sex_str)
              {
                recset_sex = rec_sex_new (recset_insensitive);
                if (!rec_sex_compile (recset_sex, recset_sex_str))
                  {
                    fprintf (stderr, "%s: error: invalid selection expression.\n",
                             argv[0]);
                    exit (1);
                  }
              }

            break;
          }
        case CASE_INSENSITIVE_ARG:
        case 'i':
          {
            recset_insensitive = true;
            break;
          }
        case NUM_ARG:
        case 'n':
          {
            if (recset_sex)
              {
                fprintf (stderr, "%s: cannot specify -n and also -e.\n",
                         argv[0]);
                exit (1);
              }
            
            /* XXX: check for conversion errors.  */
            recset_index = atoi (optarg);
            break;
          }
        case TYPE_ARG:
        case 't':
          {
            recset_type = strdup (optarg);
            break;
          }
        case SET_ACTION_ARG:
        case 's':
          {
            if (recset_action != RECSET_ACT_NONE)
              {
                fprintf (stderr, "s: please specify just one action.\n",
                         argv[0]);
                exit (1);
              }
            
            recset_action = RECSET_ACT_SET;
            recset_value = strdup (optarg);
            break;
          }
        case APPEND_ACTION_ARG:
        case 'a':
          {
            if (recset_action != RECSET_ACT_NONE)
              {
                fprintf (stderr, "s: please specify just one action.\n",
                         argv[0]);
                exit (1);
              }

            recset_action = RECSET_ACT_APPEND;
            recset_value = strdup (optarg);
            break;
          }
        case DELETE_ACTION_ARG:
        case 'd':
          {
            if (recset_action != RECSET_ACT_NONE)
              {
                fprintf (stderr, "s: please specify just one action.\n",
                         argv[0]);
                exit (1);
              }

            recset_action = RECSET_ACT_DELETE;
            break;
          }
        case COMMENT_ACTION_ARG:
        case 'c':
          {
            if (recset_action != RECSET_ACT_NONE)
              {
                fprintf (stderr, "s: please specify just one action.\n",
                         argv[0]);
                exit (1);
              }

            recset_action = RECSET_ACT_COMMENT;
            break;
          }
        }
    }
}

void
recset_process_actions (rec_db_t db)
{
  int n_rset, n_rec, rset_size, numrec, i, min, max;
  rec_rset_t rset;
  rec_record_t record;
  rec_field_t field;
  bool parse_status = true;
  rec_rset_elem_t rec_elem;
  rec_fex_elem_t fex_elem;
  rec_field_name_t field_name;

  /* If the database contains more than one type of records and the
     user did'nt specify the recsel_type then ask the user to clear
     the request.  */
  if (!recset_type && (rec_db_size (db) > 1))
    {
      fprintf (stderr, "Several record types found.  Please use -t to specify one.\n");
      exit (1);
    }

  for (n_rset = 0; n_rset < rec_db_size (db); n_rset++)
    {
      rset = rec_db_get_rset (db, n_rset);
      rset_size = rec_rset_num_records (rset);

      /* Don't process empty record sets.  */
      if (rset_size == 0)
        {
          continue;
        }

      /* If the user specified a type, process the record set only if
       * it is of the given size.  */
      if (recset_type
          && (!rec_rset_type (rset)
              || (strcmp (recset_type, rec_rset_type (rset)) != 0)))
        {
          continue;
        }

      /* If the user didn't specify a type, process a record set if
       * and only if:
       *
       * -  It is the default record set.
       * -  The file contains just one record set.
       */

      if (!recset_type
          && rec_rset_type (rset)
          && (rec_db_size (db) > 1))
        {
          continue;
        }

      /* Process this record set.  */
      numrec = 0;

      rec_elem = rec_rset_first_record (rset);
      while (rec_rset_elem_p (rec_elem))
        {
          record = rec_rset_elem_record (rec_elem);
          
          if (((recset_index == -1) && !recset_sex)
              || ((recset_index == -1) &&
                  ((recset_sex &&
                    (rec_sex_eval (recset_sex, record, &parse_status))))
                  || (recset_index == numrec)))
            {
              /* Process this record.  */

              switch (recset_action)
                {
                case RECSET_ACT_SET:
                  {
                    
                    break;
                  }
                case RECSET_ACT_APPEND:
                  {
                    /* Create new fields and append them to the
                       record.  */
                    for (i = 0; i < rec_fex_size (recset_fex); i++)
                      {
                        fex_elem = rec_fex_get (recset_fex, i);
                        
                        field_name = rec_fex_elem_field_name (fex_elem);
                        field = rec_field_new (rec_field_name_dup (field_name), recset_value);
                        
                        rec_record_append_field (record, field);
                      }
                    
                    break;
                  }
                case RECSET_ACT_DELETE:
                  {
                    
                    
                    break;
                  }
                case RECSET_ACT_COMMENT:
                  {
                    
                    break;
                  }
                }
            }
          
          /* Process the next record.  */
          rec_elem = rec_rset_next_record (rset, rec_elem);
          
          if (!parse_status)
            {
              fprintf (stderr, "recset: error: evaluating selection expression.\n");
              exit (1);
            }
          
          numrec++;
        }
    }
}

int
main (int argc, char *argv[])
{
  char *file_name = NULL;
  char *tmp_file_name = NULL;
  FILE *in;
  FILE *out;
  rec_db_t db;
  rec_writer_t writer;
  
  program_name = strdup (argv[0]);

  /* Parse arguments.  */
  recset_parse_args (argc, argv);

  db = rec_db_new ();

  /* Read the name of the data source.  */
  if (optind < argc)
    {
      if ((argc - optind) != 1)
        {
          fprintf (stderr, "%s\n", recset_help_msg);
          exit (1);
        }

      file_name = argv[optind++];
    }

  if (file_name)
    {
      in = fopen (file_name, "r");
      if (in == NULL)
        {
          fprintf (stderr, "%s: error: cannot read %s.\n", argv[0], file_name);
          exit (1);
        }
    }
  else
    {
      /* Process the standard input.  */
      in = stdin;
    }

  if (!recset_parse_db_from_file (in, file_name, db))
    {
      exit (1);
    }

  /* ACTION... */
  recset_process_actions (db);

  /* Output.  */
  if (!file_name)
    {
      out = stdout;
    }
  else
    {
      int des;

      /* Create a temporary file with the results. */
      tmp_file_name = malloc (100);
      strcpy (tmp_file_name, "recXXXXXX");
      des = mkstemp (tmp_file_name);
      if (des == -1)
        {
          fprintf(stderr, "%s: error: cannot create a unique name.\n", argv[0]);
          exit (1);
        }
      out = fdopen (des, "w+");
    }

  writer = rec_writer_new (out);
  rec_write_db (writer, db);
  fclose (out);
  rec_db_destroy (db);

  if (file_name)
    {
      /* Rename the temporary file to file_name.  */
      if (rename (tmp_file_name, file_name) == -1)
        {
          fprintf (stderr, "%s: error: moving %s to %s\n",
                   argv[0], tmp_file_name, file_name);
          remove (tmp_file_name);
          exit (1);
        }
    }

  return 0;
}

/* End of recset.c */
