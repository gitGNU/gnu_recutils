/* -*- mode: C -*- Time-stamp: "2011-08-28 19:43:08 jemarch"
 *
 *       File:         rec-crypt.c
 *       Date:         Fri Aug 26 19:50:51 2011
 *
 *       GNU recutils - Encryption routines
 *
 */

/* Copyright (C) 2011 Jose E. Marchesi */

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

#include <gcrypt.h>

#include <rec.h>
#include <rec-utils.h>

/* Size of a block in AES128 */
#define AESV2_BLKSIZE 16
#define AESV2_KEYSIZE 16

bool
rec_encrypt (char   *in,
             size_t  in_size,
             char   *password,
             char  **out,
             size_t *out_size)
{
  gcry_cipher_hd_t handler;
  size_t i;
  size_t password_size;
  char key[AESV2_KEYSIZE];
  char iv[AESV2_BLKSIZE];
  size_t padding;

  /* The size of the input buffer must be bigger than AESV2_BLKSIZE,
     and must contain an entire number of blocks.  We assure that by
     padding the buffer with \0 characters.  */

  if ((in_size % AESV2_BLKSIZE) != 0)
    {
      padding = AESV2_BLKSIZE - (in_size % AESV2_BLKSIZE);
    }
  else
    {
      padding = 0;
    }

  if (padding != 0)
    {
      in_size = in_size + padding;
      in = realloc (in, in_size);

      for (i = 0; i < padding; i++)
        {
          in[in_size - i - 1] = '\0';
        }
    }  

  /* Create the handler.  */
  if (gcry_cipher_open (&handler,
                        GCRY_CIPHER_AES128,
                        GCRY_CIPHER_MODE_CBC,
                        0) != GPG_ERR_NO_ERROR)
    {
      return false;
    }

  /* Set the key of the cypher.  */
  password_size = strlen (password);
  for (i = 0; i < AESV2_KEYSIZE; i++)
    {
      key[i] = password[i % password_size];
    }

  /* Set both the key and the IV vector.  */
  if (gcry_cipher_setkey (handler, key, AESV2_KEYSIZE)
      != GPG_ERR_NO_ERROR)
    {
      return false;
    }

  for (i = 0; i < AESV2_BLKSIZE; i++)
    {
      iv[i] = i;
    }
  gcry_cipher_setiv (handler, iv, AESV2_BLKSIZE);

  /* Encrypt the data.  */
  *out_size = in_size;
  *out = malloc (*out_size);
  if (gcry_cipher_encrypt (handler,
                           *out,
                           *out_size,
                           in,
                           in_size) != 0)
    {
      /* Error.  */
      return false;
    }

  /* Close the handler.  */
  gcry_cipher_close (handler);

  return true;
}

bool
rec_decrypt (char   *in,
             size_t  in_size,
             char   *password,
             char  **out,
             size_t *out_size)
{
  gcry_cipher_hd_t handler;
  size_t i;
  size_t password_size;
  char key[AESV2_KEYSIZE];
  char iv[AESV2_BLKSIZE];
  
  if ((in_size % AESV2_BLKSIZE) != 0)
    {
      return false;
    }

  /* Create the handler.  */
  if (gcry_cipher_open (&handler,
                        GCRY_CIPHER_AES128,
                        GCRY_CIPHER_MODE_CBC,
                        0) != GPG_ERR_NO_ERROR)
    {
      return false;
    }

  /* Set the key of the cypher.  */
  password_size = strlen (password);
  for (i = 0; i < AESV2_KEYSIZE; i++)
    {
      key[i] = password[i % password_size];
    }

  /* Set both the key and the IV vector.  */
  if (gcry_cipher_setkey (handler, key, AESV2_KEYSIZE)
      != GPG_ERR_NO_ERROR)
    {
      printf ("error setting key\n");
      return false;
    }

  for (i = 0; i < AESV2_BLKSIZE; i++)
    {
      iv[i] = i;
    }
  gcry_cipher_setiv (handler, iv, AESV2_BLKSIZE);

  /* Decrypt the data.  */
  *out_size = in_size;
  *out = malloc (*out_size);
  if (gcry_cipher_decrypt (handler,
                           *out,
                           *out_size,
                           in,
                           in_size) != 0)
    {
      /* Error.  */
      return false;
    }

  /* Make sure the decrypted data is ok by checking the CRC at the end
     of the sequence.  */
  //  if ((*out_size < 4)
  //      || (strcmp ((*out)[(*out_size)-REC_CRYPT_CTRL_SEQ_SIZE],
  //                  REC_CRYPT_CTRL_SEQ) != 0))
  //    {
  //      return false;
  //    }

  /* Close the handler.  */
  gcry_cipher_close (handler);

  return true;
}

/* End of rec-crypt.c */
