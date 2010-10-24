/* -*- mode: C -*- Time-stamp: "2010-10-24 20:15:55 jemarch"
 *
 *       File:         rec.c
 *       Date:         Sun Oct 24 19:47:16 2010
 *
 *       GNU recutils - Initialization and finalization routines.
 *
 */

#include <config.h>

#include <libintl.h>

#include <rec.h>

void
rec_init (void)
{
  bindtextdomain (PACKAGE, LOCALEDIR);
}

void
rec_fini (void)
{

}

/* End of rec.c */
