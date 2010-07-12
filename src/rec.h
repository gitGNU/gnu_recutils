/* -*- mode: C -*-
 *
 *       File:         rec.h
 *       Date:         Fri Feb 27 20:04:59 2009
 *
 *       GNU recutils - Main Header
 *
 */

/* Copyright (C) 2009, 2010 Jose E. Marchesi */

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

#ifndef REC_H
#define REC_H

#include <config.h>

#include <stdbool.h>
#include <stdio.h>
#include <fcntl.h>

/* 
 * rec format version implemented by this library.
 */

#define REC_VERSION_MAJOR 1
#define REC_VERSION_MINOR 0
#define REC_VERSION_STRING "1.0"

/*
 * COMMENTS
 *
 * A comment is a block of text.  The printed representation of a
 * comment includes a sharp (#) character after each newline (\n)
 * character.
 */

typedef char *rec_comment_t;

/* Creation and destruction of comments.  */

rec_comment_t rec_comment_new (char *text);
void rec_comment_destroy (rec_comment_t comment);

rec_comment_t rec_comment_dup (rec_comment_t comment);

/* Getting/Setting properties.  */
char *rec_comment_text (rec_comment_t comment);
void rec_comment_set_text (rec_comment_t comment, char *text);

/* Testing comments.  */
bool rec_comment_equal_p (rec_comment_t comment1, rec_comment_t comment2);

/* FIELD NAMES
 *
 * A field name is a list of name-parts:
 *
 *   namepart1:namepart2: ...
 *
 * Each name-part is finished by a colon (:) character.
 */

typedef struct rec_field_name_s *rec_field_name_t;

/* Creating a field name.
 *
 * In the case of an error NULL is returned.
 */
rec_field_name_t rec_field_name_new ();

/* Destroy a field name.  */
void rec_field_name_destroy (rec_field_name_t fname);

/* Get a copy of a field name.  */
rec_field_name_t rec_field_name_dup (rec_field_name_t fname);

/* Comparing field names.
 *
 * Two given field names are equal if and only if they contain the
 * same number of name parts and they are identical.
 */

bool rec_field_name_eql_p (rec_field_name_t fname1,
                           rec_field_name_t fname2);

/* XXX: explain.  */
bool rec_field_name_equal_p (rec_field_name_t fname1,
                             rec_field_name_t fname2);

/* Get the size of a field name measured in number of parts.  */
int rec_field_name_size (rec_field_name_t fname);

/* Set a part in a field name.
 *
 * If INDEX is out of bounds then 'false' is returned.
 */
bool rec_field_name_set (rec_field_name_t fname,
                         int index,
                         const char *str);

/* Get a part of a field name.
 *
 * If INDEX is out of bounds then NULL is returned.
 */
const char *rec_field_name_get (rec_field_name_t fname,
                                int index);
/*
 * FIELDS
 *
 * A field is a name-value pair.
 */

typedef struct rec_field_s *rec_field_t;

/* Return a newly created field.  In the case of an error NULL is
   returned.  */
rec_field_t rec_field_new (const rec_field_name_t name,
                           const char *value);

/* Destroy a field, freeing any occupied memory. */
void rec_field_destroy (rec_field_t field);

/* Return a pointer to the string containing the field name. */
rec_field_name_t rec_field_name (rec_field_t field);
char *rec_field_name_str (rec_field_t field);

/* Set the name of a field to a given string. */
void rec_field_set_name (rec_field_t field, rec_field_name_t fname);

/* Return a pointer to the string containing the value of the
   field. */
char *rec_field_value (rec_field_t field);

/* Set the value of a given field to the given string. */
void rec_field_set_value (rec_field_t field, const char *value);

/* Return a copy of a given field. If there is not enough memory to
   perform the copy then NULL is returned. */
rec_field_t rec_field_dup (rec_field_t field);

/* Determine wether two given fields are equal (i.e. they have equal
   names and equal values). */
bool rec_field_equal_p (rec_field_t field1,
                        rec_field_t field2);

/* Others.. */
rec_comment_t rec_field_to_comment (rec_field_t field);

/*
 * RECORDS
 *
 * A record is an ordered set of one or more fields intermixed with
 * comment blocks.
 */

typedef struct rec_record_s *rec_record_t;

struct rec_record_elem_s
{
  struct rec_mset_elem_s *mset_elem;
};

typedef struct rec_record_elem_s rec_record_elem_t;

/* General.  */
rec_record_t rec_record_new (void);
void rec_record_destroy (rec_record_t record);

rec_record_t rec_record_dup (rec_record_t record);

bool rec_record_subset_p (rec_record_t record1,
                          rec_record_t record2);

bool rec_record_equal_p (rec_record_t record1,
                         rec_record_t record2);

/* Statistics.  */

int rec_record_num_elems (rec_record_t record);
int rec_record_num_fields (rec_record_t record);
int rec_record_num_comments (rec_record_t record);

/* Location properties.  */
char *rec_record_source (rec_record_t record);
void rec_record_set_source (rec_record_t record, char *source);

size_t rec_record_location (rec_record_t record);
char *rec_record_location_str (rec_record_t record);
void rec_record_set_location (rec_record_t record, size_t location);

/* Getting and setting elements.  */

rec_record_elem_t rec_record_get_elem (rec_record_t record, int position);
rec_record_elem_t rec_record_get_field (rec_record_t record, int position);
rec_record_elem_t rec_record_get_comment (rec_record_t record, int position);

bool rec_record_remove_at (rec_record_t record, int position);
void rec_record_insert_at (rec_record_t record, rec_record_elem_t elem, int position);
void rec_record_append (rec_record_t record, rec_record_elem_t elem);
void rec_record_append_field (rec_record_t record, rec_field_t field);
void rec_record_append_comment (rec_record_t record, rec_comment_t comment);

rec_record_elem_t rec_record_remove (rec_record_t record, rec_record_elem_t elem);
rec_record_elem_t rec_record_remove_field (rec_record_t record, rec_record_elem_t elem);
rec_record_elem_t rec_record_remove_comment (rec_record_t record, rec_record_elem_t elem);
void rec_record_insert_after (rec_record_t record,
                              rec_record_elem_t elem,
                              rec_record_elem_t new_elem);

/* Searching.  */
rec_record_elem_t rec_record_search_field (rec_record_t record,
                                           rec_field_t field);

int rec_record_get_field_index (rec_record_t record,
                                rec_field_t field);

/* Iterating.  */
rec_record_elem_t rec_record_first (rec_record_t record);
rec_record_elem_t rec_record_first_field (rec_record_t record);
rec_record_elem_t rec_record_first_comment (rec_record_t record);

rec_record_elem_t rec_record_next (rec_record_t record, rec_record_elem_t elem);
rec_record_elem_t rec_record_next_field (rec_record_t record, rec_record_elem_t elem);
rec_record_elem_t rec_record_next_comment (rec_record_t record, rec_record_elem_t elem);

/* By-name operations.  */

bool rec_record_field_p (rec_record_t record,
                         rec_field_name_t field_name);

int rec_record_get_num_fields_by_name (rec_record_t record,
                                       rec_field_name_t field_name);

rec_field_t rec_record_get_field_by_name (rec_record_t record,
                                          rec_field_name_t field_name,
                                          int n);

void rec_record_remove_field_by_name (rec_record_t record,
                                      rec_field_name_t field_name,
                                      int index);

int rec_record_get_field_index_by_name (rec_record_t record,
                                        rec_field_t field);

/* Elements.  */
rec_record_elem_t rec_record_null_elem (void);

rec_record_elem_t rec_record_elem_field_new (rec_record_t record,
                                             rec_field_t field);
rec_record_elem_t rec_record_elem_comment_new (rec_record_t record,
                                               rec_comment_t comment);

bool rec_record_elem_p (rec_record_elem_t elem);
bool rec_record_elem_field_p (rec_record_t record, rec_record_elem_t elem);
bool rec_record_elem_comment_p (rec_record_t record, rec_record_elem_t elem);

rec_field_t rec_record_elem_field (rec_record_elem_t elem);
rec_comment_t rec_record_elem_comment (rec_record_elem_t elem);

/* Others...  */

rec_comment_t rec_record_to_comment (rec_record_t record);

/*
 * RECORD SETS
 *
 * A record set is an ordered set of zero or more records and comments
 * maybe preceded by a record descriptor.
 */

typedef struct rec_rset_s *rec_rset_t;

struct rec_rset_elem_s
{
  struct rec_mset_elem_s *mset_elem;
};

typedef struct rec_rset_elem_s rec_rset_elem_t;

/* General.  */
rec_rset_t rec_rset_new (void);
void rec_rset_destroy (rec_rset_t rset);

rec_rset_t rec_rset_dup (rec_rset_t rset);

/* Statistics.  */

int rec_rset_num_elems (rec_rset_t rset);
int rec_rset_num_records (rec_rset_t rset);
int rec_rset_num_comments (rec_rset_t rset);

/* Getting and setting elements.  */

rec_rset_elem_t rec_rset_null_elem (void);

rec_rset_elem_t rec_rset_get_elem (rec_rset_t rset, int position);
rec_rset_elem_t rec_rset_get_record (rec_rset_t rset, int position);
rec_rset_elem_t rec_rset_get_comment (rec_rset_t rset, int position);

bool rec_rset_remove_at (rec_rset_t rset, int position);
void rec_rset_insert_at (rec_rset_t rset, rec_rset_elem_t elem, int position);
void rec_rset_append (rec_rset_t rset, rec_rset_elem_t elem);
void rec_rset_append_record (rec_rset_t rset, rec_record_t record);
void rec_rset_append_comment (rec_rset_t rset, rec_comment_t comment);

rec_rset_elem_t rec_rset_remove (rec_rset_t rset, rec_rset_elem_t elem);
rec_rset_elem_t rec_rset_remove_record (rec_rset_t rset, rec_rset_elem_t elem);
rec_rset_elem_t rec_rset_remove_comment (rec_rset_t rset, rec_rset_elem_t elem);
void rec_rset_insert_after (rec_rset_t rset,
                            rec_rset_elem_t elem,
                            rec_rset_elem_t new_elem);

/* Iterating.  */
rec_rset_elem_t rec_rset_first (rec_rset_t rset);
rec_rset_elem_t rec_rset_first_record (rec_rset_t rset);
rec_rset_elem_t rec_rset_first_comment (rec_rset_t rset);

rec_rset_elem_t rec_rset_next (rec_rset_t rset, rec_rset_elem_t elem);
rec_rset_elem_t rec_rset_next_record (rec_rset_t rset, rec_rset_elem_t elem);
rec_rset_elem_t rec_rset_next_comment (rec_rset_t rset, rec_rset_elem_t elem);

/* Elements.  */
rec_rset_elem_t rec_rset_elem_record_new (rec_rset_t rset, rec_record_t record);
rec_rset_elem_t rec_rset_elem_comment_new (rec_rset_t rset, rec_comment_t comment);

bool rec_rset_elem_p (rec_rset_elem_t elem);
bool rec_rset_elem_record_p (rec_rset_t rset, rec_rset_elem_t elem);
bool rec_rset_elem_comment_p (rec_rset_t rset, rec_rset_elem_t elem);

rec_record_t rec_rset_elem_record (rec_rset_elem_t elem);
rec_comment_t rec_rset_elem_comment (rec_rset_elem_t elem);

/* Record descriptor management.  */

rec_record_t rec_rset_descriptor (rec_rset_t rset);
void rec_rset_set_descriptor (rec_rset_t rset, rec_record_t record);

char *rec_rset_type (rec_rset_t rset);
void rec_rset_set_type (rec_rset_t rset, char *type);

/* Integrity.  */

int rec_rset_check (rec_rset_t rset, bool check_descriptor_p, FILE *errors);
int rec_rset_check_record (rec_rset_t rset, rec_record_t orig_rec,
                           rec_record_t rec, FILE *errors);
bool rec_rset_check_field_type (rec_rset_t rset, rec_field_t field, char **type);

/*
 * DATABASES
 *
 * A database is an ordered set of zero or more record sets.
 */

typedef struct rec_db_s *rec_db_t;

/* Create an empty database.  */
rec_db_t rec_db_new (void);

/* Destroy a database, freeing any used memory.
 *
 * This means that all the record sets contained in the database are
 * also destroyed.
 */
void rec_db_destroy (rec_db_t db);

/* Return the number of record sets contained in a given record
   set.  */
int rec_db_size (rec_db_t db);

/* Return a pointer to the record set at the given position.
 *
 * If no such record set is contained in the database then NULL is
 * returned.
 */
rec_rset_t rec_db_get_rset (rec_db_t db,
                            int position);

/* Insert the given record set into the given database at the given
 * position.
 *
 * - If POSITION >= rec_rset_size (DB), RSET is appended to the
 *   list of fields.
 *
 * - If POSITION < 0, RSET is prepended.
 *
 * - Otherwise RSET is inserted at the specified position.
 *
 * If the rset is inserted then 'true' is returned. If there is an
 * error then 'false' is returned.
 */
bool rec_db_insert_rset (rec_db_t db,
                         rec_rset_t rset,
                         int position);

/* Remove the record set contained in the given position into the
 * given database.
 *
 * - If POSITION >= rec_db_size (DB), the last record set is deleted.
 *
 * - If POSITION <= 0, the first record set is deleted.
 *
 * - Otherwise the record set occupying the specified position is
 *   deleted.
 *
 * If a record set has been removed then 'true' is returned.  If there
 * is an error or the database has no record sets 'false' is returned.
 */
bool rec_db_remove_rset (rec_db_t db, int position);

/* Determine whether an rset named TYPE exists in DB.  */
bool rec_db_type_p (rec_db_t db, char *type);

/* Get the rset with the given type from db.  */
rec_rset_t rec_db_get_rset_by_type (rec_db_t db, char *type);

/*
 * PARSER
 *
 * The rec parser provides functions to parse field, records and
 * entire record sets from a file stream.
 */

typedef struct rec_parser_s *rec_parser_t;

/* Create a parser associated with a given file stream.  If not enough
   memory, return NULL. */
rec_parser_t rec_parser_new (FILE *in, char *file_name);

/* Destroy a parser.
 *
 * Note that this call is not closing the associated file stream.
 */
void rec_parser_destroy (rec_parser_t parser);

/* Parsing routines.
 *
 * If a parse error (or EOF) occurs, the following functions return
 * NULL.
 */

rec_field_name_t rec_parse_field_name_str (char *str);
bool rec_parse_field_name (rec_parser_t parser, rec_field_name_t *fname);
bool rec_parse_field (rec_parser_t parser, rec_field_t *field);
bool rec_parse_record (rec_parser_t parser, rec_record_t *record);
bool rec_parse_rset (rec_parser_t parser, rec_rset_t *rset);
bool rec_parse_db (rec_parser_t parser, rec_db_t *db);

/* Getting information about the parser */
bool rec_parser_eof (rec_parser_t parser);
bool rec_parser_error (rec_parser_t parser);

/* Reset the error status and EOF of a parser. */
void rec_parser_reset (rec_parser_t parser);

/* Print a message with details on the last parser error.
 *
 * This function produces a message on the standard error output,
 * describing the last error encountered while parsing.  First, if FMT
 * is not NULL, it is printed along with any remaining argument.  Then
 * a colon and a space are printed, and finally an error message
 * describing what went wrong.
 */
void rec_parser_perror (rec_parser_t parser, char *fmt, ...);


/*
 * WRITER
 *
 * Writing routines.
 */

typedef struct rec_writer_s *rec_writer_t;

/* Create a writer associated with a given file stream.  If not enough
   memory, return NULL. */
rec_writer_t rec_writer_new (FILE *out);

/* Destroy a writer.
 *
 * Note that this call is not closing the associated file stream.
 */
void rec_writer_destroy (rec_writer_t writer);

/* Writing routines.
 *
 * If EOF occurs, the following functions return NULL.
 */

bool rec_write_comment (rec_writer_t writer, rec_comment_t comment);
bool rec_write_field_name (rec_writer_t writer, rec_field_name_t field_name);
bool rec_write_field (rec_writer_t writer, rec_field_t field);
bool rec_write_record (rec_writer_t writer, rec_record_t record);
bool rec_write_rset (rec_writer_t writer, rec_rset_t rset);
bool rec_write_db (rec_writer_t writer, rec_db_t db);

char *rec_write_field_name_str (rec_field_name_t field_name);
char *rec_write_field_str (rec_field_t field);
char *rec_write_comment_str (rec_comment_t comment);

/* Getting information about the writer */
bool rec_writer_eof (rec_writer_t writer);
int rec_writer_line (rec_writer_t writer);

/*
 * SELECTION EXPRESSIONS
 *
 * A selection expression is a written boolean expression that can be
 * applied on a record.
 */

typedef struct rec_sex_s *rec_sex_t;

/* Create a new selection expression and return it.  If there is not
   enough memory to create the sex, then return NULL.  */
rec_sex_t rec_sex_new (bool case_insensitive);

/* Destroy a sex.  */
void rec_sex_destroy (rec_sex_t sex);

/* Compile a sex.  If there is a parse error return false.  */
bool rec_sex_compile (rec_sex_t sex, char *expr);

/* Apply a sex expression to a record, setting RESULT in accordance.  */
bool rec_sex_eval (rec_sex_t sex, rec_record_t record, bool *status);

void rec_sex_print_ast (rec_sex_t sex);

/*
 * FIELD EXPRESSIONS
 *
 * A Field expression is composed by a sequence of "elements".  Each
 * element makes a reference to one or more fields in a record with a
 * given name:
 *
 *  - Name:  Field name.
 *  - Range: [min..max]
 *  - Prefix: One-character prefix.
 *
 * FEXs can be parsed from strings following this syntax:
 *
 *      ELEM_DESCRIPTOR1,...,ELEM_DESCRIPTORn
 *
 * Where each element descriptor is:
 *
 *      /?FIELD_NAME([N(..N)?])?
 */

typedef struct rec_fex_s *rec_fex_t;
typedef struct rec_fex_elem_s *rec_fex_elem_t;

enum rec_fex_kind_e
{
  REC_FEX_SIMPLE,
  REC_FEX_SUBSCRIPTS
};

rec_fex_t rec_fex_new (char *str,
                       enum rec_fex_kind_e kind);
void rec_fex_destroy (rec_fex_t fex);

bool prec_fex_check (char *str);

void rec_fex_sort (rec_fex_t fex);

int rec_fex_size (rec_fex_t fex);
rec_fex_elem_t rec_fex_get (rec_fex_t fex, int position);

char rec_fex_elem_prefix (rec_fex_elem_t elem);
rec_field_name_t rec_fex_elem_field_name (rec_fex_elem_t elem);
char *rec_fex_elem_field_name_str (rec_fex_elem_t elem);
int rec_fex_elem_min (rec_fex_elem_t elem);
int rec_fex_elem_max (rec_fex_elem_t elem);

/*
 * FIELD TYPES
 *
 */

enum rec_type_kind_e
  {
    /* Unrestricted.  */
    REC_TYPE_NONE = 0,
    /* An integer number.  */
    REC_TYPE_INT,
    /* A Boolean.  */
    REC_TYPE_BOOL,
    /* An integer number within a given range.  */
    REC_TYPE_RANGE,
    /* A real number.  */
    REC_TYPE_REAL,
    /* A string with a limitation on its size.  */
    REC_TYPE_SIZE,
    /* A line.  */
    REC_TYPE_LINE,
    /* A regexp.  */
    REC_TYPE_REGEXP,
    /* A date.  */
    REC_TYPE_DATE,
    /* An Enumeration.  */
    REC_TYPE_ENUM,
    /* A field name.  */
    REC_TYPE_FIELD,
    /* An email.  */
    REC_TYPE_EMAIL
  };

typedef struct rec_type_s *rec_type_t;

bool rec_type_descr_p (char *str);
rec_field_name_t rec_type_descr_field_name (char *str);

rec_type_t rec_type_new (char *str);
void rec_type_destroy (rec_type_t type);

enum rec_type_kind_e rec_type_kind (rec_type_t type);
char *rec_type_kind_str (rec_type_t type);

bool rec_type_check (rec_type_t type, char *str);

/*
 * TYPE REGISTRIES.
 *
 */

typedef struct rec_type_reg_s *rec_type_reg_t;

rec_type_reg_t rec_type_reg_new (void);
void rec_type_reg_destroy (rec_type_reg_t reg);

void rec_type_reg_register (rec_type_reg_t reg, rec_field_name_t name, rec_type_t value);
rec_type_t rec_type_reg_get (rec_type_reg_t reg, rec_field_name_t name);
                             
#endif /* !REC_H */

/* End of rec.h */
