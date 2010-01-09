/* -*- mode: C -*- Time-stamp: ""
 *
 *       File:         rec.h
 *       Date:         Fri Feb 27 20:04:59 2009
 *
 *       GNU rec library - Main Header
 *
 */

/* Copyright (C) 2009 Jose E. Marchesi */

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

/* FIELD NAMES
 *
 * A field name is a list of name-parts:
 *
 *   namepart1:namepart2: ...
 *
 * Each name-part is finished by a colon (:) character.
 */

typedef struct rec_field_name_s *rec_field_name_t;

/* Return a newly created field.
 *
 * In the case of an error NULL is returned.
 */
rec_field_name_t rec_field_name_new ();

/* Destroy a field name */
void rec_field_name_destroy (rec_field_name_t fname);

/* Get a copy of a field name */
rec_field_name_t rec_field_name_dup (rec_field_name_t fname);

/* Comparation of field names.
 *
 * Two given field names are equal if and only if they contain the
 * same number of name parts and they are identical.
*/
bool rec_field_name_equal_p (rec_field_name_t fname1,
                             rec_field_name_t fname2);

/* Get the number of parts contained in a given field name */
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

/* Return a newly created field.
 *
 * In the case of an error NULL is returned.
 */
rec_field_t rec_field_new (const rec_field_name_t name,
                           const char *value);

/* Destroy a field, freeing any occupied memory. */
void rec_field_destroy (rec_field_t field);

/* Return a pointer to the string containing the field name. */
rec_field_name_t rec_field_name (rec_field_t field);

/* Set the name of a field to a given string. */
void rec_field_set_name (rec_field_t field, rec_field_name_t fname);

/* Return a pointer to the string containing the value of the
   field. */
const char *rec_field_value (rec_field_t field);

/* Set the value of a given field to the given string. */
void rec_field_set_value (rec_field_t field, const char *value);

/* Return a copy of a given field. If there is not enough memory to
   perform the copy then NULL is returned. */
rec_field_t rec_field_dup (rec_field_t field);

/* Determine wether two given fields are equal (i.e. they have equal
   names and equal values). */
bool rec_field_equal_p (rec_field_t field1,
                        rec_field_t field2);

/*
 * RECORDS
 *
 * A record is an ordered set of one or more fields.
 */

typedef struct rec_record_s *rec_record_t;

/* Create and return a new empty record. */
rec_record_t rec_record_new (void);

/* Destroy a record freeing any used resource.
 *
 * The contained fields are also destroyed.
 */
void rec_record_destroy (rec_record_t record);

/* Return the size of a given record. i.e. the number of fields
 * contained in the record.
 */
int rec_record_size (rec_record_t record);

/* Determine wether there is a field in the given record with the
 * given name.
 */
bool rec_record_field_p (rec_record_t record,
                         rec_field_name_t field_name);

/* Insert a field into a record at a given position.
 *
 * - If POSITION >= rec_record_size (RECORD), FIELD is appended to the
 *   list of fields.
 *
 * - If POSITION < 0, FIELD is prepended.
 *
 * - Otherwise FIELD is inserted at the specified position.
 *
 * If the field is inserted then 'true' is returned. If there is an
 * error then 'false' is returned.
 */
bool rec_record_insert_field (rec_record_t record,
                              rec_field_t field,
                              int position);

/* Remove the field contained at the given POSITION in RECORD.
 *
 * - If POSITION >= rec_record_size (RECORD), the last field is
 *   deleted.
 *
 * - If POSITION <= 0, the first field is deleted.
 *
 * - Otherwise the field occupying the specified position is deleted.
 *
 * If a field has been removed then 'true' is returned.  If there is
 * an error or the record has no fields 'false' is returned.
 */
bool rec_record_remove_field (rec_record_t record,
                              int position);

/* Return a pointer to the field at the given position.
 * 
 * - If POSITOIN >= rec_record_size (RECORD), the last field is
 *   returned.
 *
 * - If POSITION <= 0, the first field is returned.
 *
 * - Otherwise the field occupying the specified position is returned.
 * 
 */
rec_field_t rec_record_get_field (rec_record_t record,
                                  int position);

/* Return a pointer to the first field with the given name in RECORD.
 *
 * If such a record is not found then return NULL.
 */
rec_field_t rec_record_get_field_name (rec_record_t record,
                                       const char *name);

/* Determine wether two given records are equal (i.e. they contain
 * exactly the same fields with same values).
 */
bool rec_record_equal_p (rec_record_t record1,
                         rec_record_t record2);

/* Determine wether a record is a subset of another record.
 *
 * A record is contained in another record if all fields defined in
 * the first record are defined in the second record having the same
 * value.
 */
bool rec_record_subset_p (rec_record_t record1,
                          rec_record_t record2);

/* Return a copy of a given record.
 *
 * If there is not enough memory to perform the copy then NULL is
 * returned. */
rec_record_t rec_record_dup (rec_record_t record);

/*
 * RECORD SETS
 *
 * A record set is an ordered set of zero or more records maybe
 * preceded by a record descriptor.
 */
typedef struct rec_rset_s *rec_rset_t;

/* Create an empty record set. */
rec_rset_t rec_rset_new (void);

/* Destroy a record set, freeing any used memory.
 *
 * This means that all the records contained in the record set are
 * also destroyed.
 */
void rec_rset_destroy (rec_rset_t rset);

/* Return the number of records contained in a given record set.
 *
 * The record descriptor is not included in the count.
 */
int rec_rset_size (rec_rset_t rset);

/* Return a pointer to the record at the given position.
 *
 * If no such record is contained in the record set at that position
 * then NULL is returned.
 */
rec_record_t rec_rset_get_record (rec_rset_t rset,
                                  int position);

/* Insert the given record into the given record set at the given
 * position.
 *
 * - If POSITION >= rec_rset_size (RSET), RECORD is appended to the
 *   list of fields.
 *
 * - If POSITION < 0, RECORD is prepended.
 *
 * - Otherwise RECORD is inserted at the specified position.
 *
 * If the record is inserted then 'true' is returned. If there is an
 * error then 'false' is returned.
 */
bool rec_rset_insert_record (rec_rset_t rset,
                             rec_record_t record,
                             int position);

/* Remove the record contained in the given position into the given
 * record set.
 *
 * - If POSITION >= rec_rset_size (RSET), the last record is
 *   deleted.
 *
 * - If POSITION <= 0, the first record is deleted.
 *
 * - Otherwise the record occupying the specified position is deleted.
 *
 * If a record has been removed then 'true' is returned.  If there is
 * an error or the record set has no records 'false' is returned.
 */
bool rec_rset_remove_record (rec_rset_t rset, int position);


/* Return a pointer to the record descriptor contained in the given
 * record set.
 */
rec_record_t rec_rset_descriptor (rec_rset_t rset);

/* Replace the record descriptor of a record set with a given
 * record.
 *
 * The previous descriptor in the record set is destroyed.
 */
void rec_rset_set_descriptor (rec_rset_t rset, rec_record_t record);

/*
 * PARSER
 *
 * The rec parser provides functions to parse field, records and
 * entire record sets from a file stream.
 */

typedef struct rec_parser_s *rec_parser_t;

/* Create a parser associated with a given file stream.  If not enough
   memory, return NULL. */
rec_parser_t rec_parser_new (FILE *in);

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

bool rec_parse_field_name (rec_parser_t parser, rec_field_name_t *fname);
bool rec_parse_field (rec_parser_t parser, rec_field_t *field);
bool rec_parse_record (rec_parser_t parser, rec_record_t *record);
bool rec_parse_rset (rec_parser_t parser, rec_rset_t *rset);

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

bool rec_write_field (rec_writer_t writer, rec_field_t field);
bool rec_write_record (rec_writer_t writer, rec_record_t record);
bool rec_write_rset (rec_writer_t writer, rec_rset_t rset);

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
rec_sex_t rec_sex_new ();

/* Destroy a sex.  */
void rec_sex_destroy (rec_sex_t sex);

/* Apply a sex expression to a record.  */
bool rec_sex_apply (rec_sex_t sex, char *expr, rec_record_t record);

#endif /* !REC_H */

/* End of rec.h */
