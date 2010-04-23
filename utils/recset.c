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
#include <recutl.h>

/*
 * Forward prototypes.
 */

static void recset_parse_args (int argc, char **argv);
static void recset_process_actions (rec_db_t db);

/*
 * Global variables.
 */

#define RECSET_ACT_NONE    0
#define RECSET_ACT_SET     1
#define RECSET_ACT_ADD     2
#define RECSET_ACT_DELETE  3
#define RECSET_ACT_COMMENT 4

char      *program_name       = NULL;
char      *recutl_sex_str     = NULL;
rec_sex_t  recutl_sex         = NULL;
char      *recutl_fex_str     = NULL;
rec_fex_t  recutl_fex         = NULL;
char      *recutl_type        = NULL;
int        recset_action      = RECSET_ACT_NONE;
char      *recset_value       = NULL;
bool       recutl_insensitive = false;
long       recutl_num         = -1;
char      *recset_file        = NULL;

/*
 * Command line options management
 */

enum
  {
    COMMON_ARGS,
    RECORD_SELECTION_ARGS,
    FIELD_EXPR_ARG,
    ADD_ACTION_ARG,
    DELETE_ACTION_ARG,
    COMMENT_ACTION_ARG,
    SET_ACTION_ARG,
  };

static const struct option GNU_longOptions[] =
  {
    COMMON_LONG_ARGS,
    RECORD_SELECTION_LONG_ARGS,
    {"fields", required_argument, NULL, FIELD_EXPR_ARG},
    {"add", required_argument, NULL, ADD_ACTION_ARG},
    {"delete", no_argument, NULL, DELETE_ACTION_ARG},
    {"comment", no_argument, NULL, COMMENT_ACTION_ARG},
    {"set", required_argument, NULL, SET_ACTION_ARG},
    {NULL, 0, NULL, 0}
  };

/* Messages */

RECUTL_COPYRIGHT_DOC ("recset");

char *recutl_help_msg = "\
Usage: recset [OPTION]... [FILE]...\n\
Alter or delete fields in records.\n\
\n\
Mandatory arguments to long options are mandatory for short options too.\n\
  -i, --case-insensitive              make strings case-insensitive in selection\n"
COMMON_ARGS_DOC
"\n"
RECORD_SELECTION_ARGS_DOC
"\n\
Fields selection options:\n\
  -f, --fields=FIELDS                 comma-separated list of field names with optional\n\
                                        subscripts.\n\
Actions:\n\
  -s, --set=VALUE                     change the value of the selected fields.\n\
  -a, --add=VALUE                  add the selected fields with the given value.\n\
  -d, --delete                        delete the selected fields.\n\
  -c, --comment                       comment out the selected fields.\n\
\n\
Examples:\n\
\n\
        recset -f TmpName -d data.rec\n\
        recset -f Email[1] -s invalid@email.com friends.rec\n\
        recset -e \"Name ~ 'Smith'\" -f Email -a new@email.com friends.rec\n\
\n"
  RECUTL_HELP_FOOTER_DOC ("recset");

static void
recset_parse_args (int argc,
                   char **argv)
{
  int ret;
  char c;

  while ((ret = getopt_long (argc,
                             argv,
                             RECORD_SELECTION_SHORT_ARGS
                             "dct:s:a:f:",
                             GNU_longOptions,
                             NULL)) != -1)
    {
      c = ret;
      switch (c)
        {
          COMMON_ARGS_CASES
          RECORD_SELECTION_ARGS_CASES
        case FIELD_EXPR_ARG:
        case 'f':
          {
            recutl_fex_str = strdup (optarg);
            if (!rec_fex_check (recutl_fex_str))
              {
                exit (1);
              }

            /* Create the field expression.  */
            recutl_fex = rec_fex_new (recutl_fex_str);
            if (!recutl_fex)
              {
                fprintf (stderr, "%s: error: creating the field expression.\n", program_name);
                exit (1);
              }

            /* Sort it.  */
            rec_fex_sort (recutl_fex);

            break;
          }
        case SET_ACTION_ARG:
        case 's':
          {
            if (!recutl_fex)
              {
                fprintf (stderr, "%s: error: please specify some field with -f.\n",
                         program_name);
                exit (1);
              }

            if (recset_action != RECSET_ACT_NONE)
              {
                fprintf (stderr, "%s: error: please specify just one action.\n",
                         program_name);
                exit (1);
              }
            
            recset_action = RECSET_ACT_SET;
            recset_value = strdup (optarg);
            break;
          }
        case ADD_ACTION_ARG:
        case 'a':
          {
            if (!recutl_fex)
              {
                fprintf (stderr, "%s: error: please specify some field with -f.\n",
                         program_name);
                exit (1);
              }

            if (recset_action != RECSET_ACT_NONE)
              {
                fprintf (stderr, "%s: please specify just one action.\n",
                         program_name);
                exit (1);
              }

            recset_action = RECSET_ACT_ADD;
            recset_value = strdup (optarg);
            break;
          }
        case DELETE_ACTION_ARG:
        case 'd':
          {
            if (!recutl_fex)
              {
                fprintf (stderr, "%s: error: please specify some field with -f.\n",
                         program_name);
                exit (1);
              }

            if (recset_action != RECSET_ACT_NONE)
              {
                fprintf (stderr, "%s: error: please specify just one action.\n",
                         program_name);
                exit (1);
              }

            recset_action = RECSET_ACT_DELETE;
            break;
          }
        case COMMENT_ACTION_ARG:
        case 'c':
          {
            if (!recutl_fex)
              {
                fprintf (stderr, "%s: error: please specify some field with -f.\n",
                         program_name);
                exit (1);
              }

            if (recset_action != RECSET_ACT_NONE)
              {
                fprintf (stderr, "%s: error: please specify just one action.\n",
                         program_name);
                exit (1);
              }

            recset_action = RECSET_ACT_COMMENT;
            break;
          }
        default:
          {
            exit (1);
            break;
          }
        }
    }

  /* Read the name of the data source.  */
  if (optind < argc)
    {
      if ((argc - optind) != 1)
        {
          fprintf (stderr, "%s\n", recutl_help_msg);
          exit (1);
        }

      recset_file = argv[optind++];
    }

}

static void
recset_process_actions (rec_db_t db)
{
  int n_rset, n_rec, rset_size;
  int numrec, i, j, min, max;
  int num_fields;
  rec_rset_t rset;
  rec_record_t record;
  rec_field_t field;
  bool parse_status = true;
  rec_rset_elem_t rec_elem;
  rec_fex_elem_t fex_elem;
  rec_field_name_t field_name;
  rec_record_elem_t field_elem;
  rec_record_t record_aux;
  rec_comment_t comment;
  rec_record_elem_t comment_elem;
  bool *deletion_mask;

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
      if (recutl_type
          && (!rec_rset_type (rset)
              || (strcmp (recutl_type, rec_rset_type (rset)) != 0)))
        {
          continue;
        }

      /* If the user didn't specify a type, process a record set if
       * and only if:
       *
       * -  It is the default record set.
       * -  The file contains just one record set.
       */

      if (!recutl_type
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
          
          if (((recutl_num == -1) && !recutl_sex)
              || ((recutl_num == -1) &&
                  ((recutl_sex &&
                    (rec_sex_eval (recutl_sex, record, &parse_status))))
                  || (recutl_num == numrec)))
            {
              /* Process this record.  */

              switch (recset_action)
                {
                case RECSET_ACT_SET:
                  {
                    /* Set the value of the specified existing fields
                       to the specified values.  */
                    for (i = 0; i < rec_fex_size (recutl_fex); i++)
                      {
                        fex_elem = rec_fex_get (recutl_fex, i);
                        field_name = rec_fex_elem_field_name (fex_elem);
                        min = rec_fex_elem_min (fex_elem);
                        max = rec_fex_elem_max (fex_elem);

                        num_fields =
                          rec_record_get_num_fields_by_name (record, field_name);
                        if (min == -1)
                          {
                            /* Process all the fields with the given name.  */
                            min = 0;
                            max = num_fields - 1;
                          }
                        if (max == -1)
                          {
                            max = min;
                          }

                        for (j = 0; j < num_fields; j++)
                          {
                            if ((j >= min) && (j <= max))
                              {
                                /* Set the value of the Jth field
                                   named FIELD_NAME, if it exists.*/
                                field = rec_record_get_field_by_name (record,
                                                                      field_name,
                                                                      j);
                                if (field)
                                  {
                                    rec_field_set_value (field, recset_value);
                                  }
                              }
                          }

                      }

                    break;
                  }
                case RECSET_ACT_ADD:
                  {
                    /* Create new fields and add them to the
                       record.  */
                    for (i = 0; i < rec_fex_size (recutl_fex); i++)
                      {
                        fex_elem = rec_fex_get (recutl_fex, i);
                        field_name = rec_fex_elem_field_name (fex_elem);
                        field = rec_field_new (rec_field_name_dup (field_name), recset_value);
                        
                        /* XXX: sort the record afterwards.  */
                        rec_record_append_field (record, field);
                        /* rec_rset_sort_record (rset, record);  */
                      }
                    
                    break;
                  }
                case RECSET_ACT_DELETE:
                case RECSET_ACT_COMMENT:
                  {
                    /* Initialize the deletion mask.  */
                    deletion_mask = malloc (sizeof (bool) * rec_record_num_fields (record));
                    for (i = 0; i < rec_record_num_fields (record); i++)
                      {
                        deletion_mask[i] = false;
                      }
                    
                    /* Mark fields that will be deleted from the record.  */
                    for (i = 0; i < rec_fex_size (recutl_fex); i++)
                      {
                        fex_elem = rec_fex_get (recutl_fex, i);
                        field_name = rec_fex_elem_field_name (fex_elem);
                        min = rec_fex_elem_min (fex_elem);
                        max = rec_fex_elem_max (fex_elem);

                        num_fields =
                          rec_record_get_num_fields_by_name (record, field_name);
                        if (min == -1)
                          {
                            /* Delete all the fields with the given name.  */
                            min = 0;
                            max = num_fields - 1;
                          }
                        if (max == -1)
                          {
                            max = min;
                          }

                        for (j = 0; j < num_fields; j++)
                          {
                            if ((j >= min) && (j <= max))
                              {
                                /* Mark this field for deletion.  */
                                field = rec_record_get_field_by_name (record,
                                                                      rec_fex_elem_field_name (fex_elem),
                                                                      j);
                                deletion_mask[rec_record_get_field_index (record, field)] = true;
                              }
                          }
                      }
                    
                    /* Delete the marked fields.  */
                    i = 0;
                    field_elem = rec_record_first_field (record);
                    while (rec_record_elem_p (field_elem))
                      {
                        if (deletion_mask[i])
                          {
                            if (recset_action == RECSET_ACT_COMMENT)
                              {
                                comment = rec_field_to_comment (rec_record_elem_field (field_elem));
                                comment_elem = rec_record_elem_comment_new (record, comment);
                                rec_record_insert_after (record, field_elem, comment_elem);
                              }

                            field_elem = rec_record_remove_field (record, field_elem);
                          }
                        else
                          {
                            field_elem = rec_record_next_field (record, field_elem);
                          }

                        i++;
                      }

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
  rec_db_t db;
  
  program_name = strdup (argv[0]);

  /* Parse arguments.  */
  recset_parse_args (argc, argv);

  db = recutl_read_db_from_file (recset_file);
  recset_process_actions (db);
  recutl_write_db_to_file (db, recset_file);

  return 0;
}

/* End of recset.c */
