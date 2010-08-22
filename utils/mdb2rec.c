/* -*- mode: C -*- Time-stamp: ""
 *
 *       File:         mdb2rec.c
 *       Date:         Fri Aug 20 22:46:31 2010
 *
 *       GNU recutils - mdb to rec converter.
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
#include <ctype.h>

#include <glib.h>
#include <mdbtools.h>

#include <rec.h>
#include <recutl.h>

/* Forward declarations. */
static void parse_args (int argc, char **argv);
static rec_db_t process_mdb (void);
static rec_rset_t process_table (MdbCatalogEntry *entry);

/*
 * Global variables
 */

char *program_name; /* Initialized in main() */
char *mdb2rec_mdb_file = NULL;
bool mdb2rec_include_system = false;
bool mdb2rec_keep_empty_fields = false;

/*
 * Command line options management
 */

enum
  {
    COMMON_ARGS,
    SYSTEM_TABLES_ARG,
    KEEP_EMPTY_FIELDS_ARG
  };

static const struct option GNU_longOptions[] =
  {
    COMMON_LONG_ARGS,
    {"system-tables", no_argument, NULL, SYSTEM_TABLES_ARG},
    {"keep-empty-fields", no_argument, NULL, KEEP_EMPTY_FIELDS_ARG},
    {NULL, 0, NULL, 0}
  };

/* Messages */

RECUTL_COPYRIGHT_DOC ("mdb2rec");

char *recutl_help_msg="\
Usage: mdb2rec [OPTIONS]... MDB_FILE\n\
Convert an mdb file into a rec file.\n\
\n\
Mandatory arguments to long options are mandatory for short options too.\n\
  -s, --system-tables                 include system tables.\n\
  -e, --keep-empty-fields             don't prune empty fields in the rec\n\
                                        output\n"
COMMON_ARGS_DOC
"\n\
Examples:\n\
\n\
        mdb2rec database.mdb > database.rec\n\
\n"
  RECUTL_HELP_FOOTER_DOC ("mdb2rec");

static void
parse_args (int argc,
            char **argv)
{
  int ret;
  char c;

  while ((ret = getopt_long (argc,
                             argv,
                             "se",
                             GNU_longOptions,
                             NULL)) != -1)
    {
      c = ret;
      switch (c)
        {
          COMMON_ARGS_CASES
        case SYSTEM_TABLES_ARG:
        case 's':
          {
            mdb2rec_include_system = true;
            break;
          }
        case KEEP_EMPTY_FIELDS_ARG:
        case 'e':
          {
            mdb2rec_keep_empty_fields = true;
            break;
          }
        default:
          {
            exit (1);
          }
        }
    }

  /* Read the name of the mdb file.  */
  if ((argc - optind) != 1)
    {
      fprintf (stdout, "%s\n", recutl_help_msg);
      exit (1);
    }
  else
    {
      mdb2rec_mdb_file = argv[optind++];
    }
}

static rec_rset_t
process_table (MdbCatalogEntry *entry)
{
  rec_rset_t rset;
  MdbTableDef *table;
  MdbHandle *mdb;
  size_t i, j;
  MdbColumn *col;
  char *table_name;
  char *column_name;
  char *field_name_str;
  char *field_value;
  char **bound_values;
  int *bound_lens;
#define TYPE_VALUE_SIZE 256
  char type_value[TYPE_VALUE_SIZE];
  rec_record_t record;
  rec_field_t field;
  rec_field_name_t field_name;

  mdb = entry->mdb;
  table_name = entry->object_name;
  table = mdb_read_table (entry);

  /* XXX: Check for the validity of the table name,
     and normalize.  */

  /* Create the record set.  */
  rset = rec_rset_new ();
  if (!rset)
    {
      recutl_fatal ("out of memory\n");
    }

  /* Create the record descriptor and add the %rec: entry.  */
  field_name_str = rec_field_name_part_normalise (table_name);
  if (!field_name_str)
    {
      recutl_fatal ("failed to normalise record type name %s\n",
                    table_name);
    }

  record = rec_record_new ();
  field = rec_field_new_str ("%rec", field_name_str);
  rec_record_append_field (record, field);
  free (field_name_str);

  /* Get the columns of the table.  */
  mdb_read_columns (table);

  /* Loop on the columns.  We will define the key and the types.  */
  for (i = 0; i < table->num_cols; i++)
    {
      col = g_ptr_array_index (table->columns, i);
      column_name = col->name;
      type_value[0] = 0;

      /* Emit a field type specification.  */
      switch (col->col_type)
        {
        case MDB_BOOL:
          {
            snprintf (type_value, TYPE_VALUE_SIZE,
                      "%s bool", col->name);
            break;
          }
        case MDB_BYTE:
          {
            snprintf (type_value, TYPE_VALUE_SIZE,
                      "%s range 256", col->name);
            break;
          }
        case MDB_INT:
        case MDB_LONGINT:
        case MDB_NUMERIC:
          {
            snprintf (type_value, TYPE_VALUE_SIZE,
                      "%s int", col->name);
            break;
          }
        case MDB_FLOAT:
        case MDB_DOUBLE:
          {
            snprintf (type_value, TYPE_VALUE_SIZE,
                      "%s real", col->name);
            break;
          }
        case MDB_SDATETIME:
          {
            snprintf (type_value, TYPE_VALUE_SIZE,
                      "%s date", col->name);
            break;
          }
        case MDB_REPID:
        case MDB_MEMO:
        case MDB_OLE:
        case MDB_TEXT:
        case MDB_MONEY:
        default:
          {
            /* Don't include a type specification.  */
            break;
          }
        }

      if (type_value[0] != 0)
        {
          /* Insert a type field for this column.  */
          field = rec_field_new_str ("%type", type_value);
          rec_record_append_field (record, field);
        }
    }

  rec_rset_set_descriptor (rset, record);
  
  /* Add the records for this table.  */
  mdb_rewind_table (table);

  bound_values = (char **) malloc (table->num_cols * sizeof(char *));
  bound_lens = (int *) g_malloc(table->num_cols * sizeof(int));
  for (i = 0; i < table->num_cols; i++)
    {
      bound_values[i] = (char *) malloc (MDB_BIND_SIZE);
      mdb_bind_column (table, i+1, bound_values[i], &bound_lens[i]);
    }

  while (mdb_fetch_row (table))
    {
      record = rec_record_new ();
      if (!record)
        {
          recutl_fatal ("out of memory\n");
        }

      for (i = 0; i < table->num_cols; i++)
        {
          col = g_ptr_array_index (table->columns, i);

          if (col->col_type == MDB_OLE)
            {
              continue;
            }

          /* Compute the name of the field.  */
          /* XXX: manage foreign keys.  */
          field_name_str = rec_field_name_part_normalise (col->name);
          if (!field_name_str)
            {
              recutl_fatal ("failed to normalise field name %s\n",
                            table_name);
            }

          /* Compute the value of the field.  */
          field_value = malloc (bound_lens[i] + 1);
          strncpy (field_value, bound_values[i], bound_lens[i]);
          field_value[bound_lens[i]] = '\0';

          if (mdb2rec_keep_empty_fields || (strlen (field_value) > 0))
            {
              /* Create the field and append it into the record.  */
              field = rec_field_new_str (field_name_str, field_value);
              if (!field)
                {
                  recutl_fatal ("invalid field name %s\n", col->name);
                }

              rec_record_append_field (record, field);
            }

          free (field_name_str);
          free (field_value);
        }

      rec_rset_append_record (rset, record);
    }

  mdb_free_tabledef (table);

  return rset;
}

static rec_db_t
process_mdb (void)
{
  rec_db_t db;
  rec_rset_t rset;
  FILE *in;
  MdbHandle *mdb;
  MdbCatalogEntry *entry;
  int i;

  /* Create the rec database.  */
  db = rec_db_new ();
  if (!db)
    {
      recutl_fatal ("out of memory");
    }

  /* Initialize libmdb and open the input file.  */
  mdb_init();
  mdb_set_date_fmt ("%Y-%m-%dT%H:%M:%S%z"); /* ISO 8601 */

  mdb = mdb_open (mdb2rec_mdb_file, MDB_NOFLAGS);
  if (!mdb)
    {
      recutl_fatal ("could not open file %s\n",
                    mdb2rec_mdb_file);
    }

  /* Read the catalog.  */
  if (!mdb_read_catalog (mdb, MDB_TABLE))
    {
      recutl_fatal ("file does not appear to be an Access database\n");
    }

  /* Iterate on the catalogs.  */
  for (i = 0; i < mdb->num_catalog; i++)
    {
      entry = g_ptr_array_index (mdb->catalog, i);

      if ((entry->object_type == MDB_TABLE)
          && (mdb_is_user_table (entry) || mdb2rec_include_system))
        {
          rec_db_insert_rset (db,
                              process_table (entry),
                              rec_db_size (db));
        }
    }

  return db;
}

int
main (int argc, char *argv[])
{
  int ret;
  rec_db_t db;
  rec_writer_t writer;

  recutl_init ("mdb2rec");

  parse_args (argc, argv);
  db = process_mdb ();

  if (db)
    {
      writer = rec_writer_new (stdout);
      rec_write_db (writer, db);
      
      rec_writer_destroy (writer);
      rec_db_destroy (db);
    }

  return ret;
}

/* End of mdb2rec.c */
