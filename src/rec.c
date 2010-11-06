/* -*- mode: C -*- Time-stamp: ""
 *
 *       File:         rec.c
 *       Date:         Sun Oct 24 19:47:16 2010
 *
 *       GNU recutils - Initialization and finalization routines.
 *
 */

#include <config.h>

#if defined REMOTE_DESCRIPTORS
#   include <curl/curl.h>
#endif

#include <libintl.h>

#include <rec.h>

void
rec_init (void)
{
  bindtextdomain (PACKAGE, LOCALEDIR);
#if defined REMOTE_DESCRIPTORS
  curl_global_init (CURL_GLOBAL_ALL);
#endif
}

void
rec_fini (void)
{
#if defined REMOTE_DESCRIPTORS
  curl_global_cleanup ();
#endif
}

/* End of rec.c */
