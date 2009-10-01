/* -*- mode: C -*- Time-stamp: "09/10/01 13:44:07 jemarch"
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

/*
 * GR Fields
 *
 * A field is a name-value pair.
 */

enum rec_field_type_e
  {
    REC_FIELD_TYPE_TEXT,
    REC_FIELD_TYPE_INTEGER,
    REC_FIELD_TYPE_REAL
  };

typedef struct rec_field_s *rec_field_t;

/* Return a newly created field. In the case of an error NULL is
   returned */
rec_field_t rec_field_new (const char *name,
                           const char *value);

/* Destroy a field, freeing any occupied memory */
void rec_field_destroy (rec_field_t field);

/* Return the type of a field */
enum rec_field_type_e rec_field_get_type (rec_field_t field);

/* Set the type of a field */
void rec_field_set_type (rec_field_t field, enum rec_field_type_e type);

/* Return a string containing the field name */
const char *rec_field_get_name (rec_field_t field);

/* Set the name of a field to a given string */
void rec_field_set_name (rec_field_t field, const char *name);

/* Return the value of a given field */
const char *rec_field_get_value (rec_field_t field);

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
 * GR Records
 *
 * A record is an ordered set of one or more fields.
 */
typedef struct rec_record_s *rec_record_t;

/* Create and return a new empty record */
rec_record_t rec_record_new (void);

/* Destroy a record freeing any used resource. The contained fields,
   if any, are also destroyed */
void rec_record_destroy (rec_record_t record);

/* Return the size of a given record. i.e. the number of fields
   contained in the record */
int rec_record_size (rec_record_t record);

/* Determine wether there is a field in the given record with the
   given name */
bool rec_record_field_p (rec_record_t record,
                         const char *field_name);

/* Insert the given field into the given record. If a field with that
   name is already contained by the record then 'false' is returned
   and no field is inserted. If the field is inserted then 'true' is
   returned */
bool rec_record_insert_field (rec_record_t record,
                              rec_field_t field);

/* Remove a field with the given name from the given record. It
   returns 'true' if the field was found and removed. 'false' if the
   field was not found in the record. */
bool rec_record_remove_field (rec_record_t record,
                              const char *field_name);

/* Return a pointer to the field with the given name. If no such field
   is contained in the record then NULL is returned */
rec_field_t rec_record_get_field (rec_record_t record,
                                  const char *field_name);

/* Return a pointer to the field at the given position (zero
   based). If no such field is contained in the record at that
   position then NULL is returned */
rec_field_t rec_record_get_field_at (rec_record_t record,
                                     int position);

/* Insert the given field into the given record at the given
   position. If the position is larger than the current number of
   records then the record is appended. If the position is 0 or
   negative then the record is prepended. If the field is inserted
   then 'true' is returned. If there is an error then 'false' is
   returned. */
bool rec_record_insert_field_at (rec_record_t record,
                                 rec_field_t field,
                                 int position);

/* Determine wether two given records are equal (i.e. they contain
   exactly the same fields with same values). */
bool rec_record_equal_p (rec_record_t record1,
                         rec_record_t record2);

/* Determine wether a record is a subset of another record (i.e. any
   field defined in the first record is defined in the second
   containing the same value). */
bool rec_record_subset_p (rec_record_t record1,
                          rec_record_t record2);

/* Return a copy of a given record. If there is not enough memory to
   perform the copy then NULL is returned. */
rec_record_t rec_record_dup (rec_record_t record);

/*
 * GR Record Sets
 *
 * A record set is a set of zero or more non-special records preceded
 * by a special record. The order of the records is significant.
 */
typedef struct rec_rset_s *rec_rset_t;

/* Create a new, empty record set. The special record of the new
   record set is initialized with a grf: field. */
rec_rset_t rec_rset_new (void);

/* Destroy a record set, freeing any used memory. This means that all
   the records contained in the record set are also destroyed. */
void rec_rset_destroy (rec_rset_t rset);

/* Return the number of records contained in a given record set. The
   special record is not included in the count. */
int rec_rset_size (rec_rset_t rset);

/* Return a pointer to the record at the given position (zero
   based). If no such record is contained in the record set at that
   position then NULL is returned. */
rec_record_t rec_rset_get_record_at (rec_rset_t rset,
                                     int position);

/* Insert the given record into the given record set at the given
   position. If the position is larger than the current number of
   records then the record is appended. If the position is 0 or
   negative then the record is prepended. If the record is inserted
   then 'true' is returned. If there is an error then 'false' is
   returned. */
bool rec_rset_insert_record_at (rec_rset_t rset,
                                rec_record_t record,
                                int position);

/* Insert the given record into the given record set. If the record is
   inserted then 'true' is returned. If there is an error then 'false'
   is returned. */
bool rec_rset_insert_record (rec_rset_t rset,
                             rec_record_t record);

/* Remove the record contained in the given position into the given
   record set. A boolean value is returned telling if the record was
   found and deleted. */
bool rec_rset_remove_record_at (rec_rset_t rset, int position);

/* Return a pointer to the special record contained in the given
   record set. */
rec_record_t rec_rset_get_special_record (rec_rset_t rset);

/* Replace the special record of a record set with a given record. The
   previous special record in the record set is destroyed. */
void rec_rset_set_special_record (rec_rset_t rset, rec_record_t record);

/* 
 * GR Databases
 *
 * A database is an unordered sequence of one or more record sets.
 *
 */
typedef struct rec_db_s *rec_db_t;

#endif /* !REC_H */

/* End of rec.h */
