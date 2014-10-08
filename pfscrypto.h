/* Copyright (c) 2014 Anton Titov.
 * Copyright (c) 2014 pCloud Ltd.
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of pCloud Ltd nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL pCloud Ltd BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _PSYNC_FSCRYPTO_H
#define _PSYNC_FSCRYPTO_H

#include "pfs.h"

#define PSYNC_CRYPTO_MAX_SECTORID (UINT32_MAX-1)
#define PSYNC_CRYPTO_INVALID_SECTORID (UINT32_MAX)

typedef uint32_t psync_crypto_sectorid_t;
typedef int32_t psync_crypto_sectorid_diff_t;

typedef struct {
  psync_tree tree;
  psync_crypto_sectorid_t sectorid;
  uint32_t logoffset;
  psync_crypto_sector_auth_t auth;
} psync_sector_inlog_t;

int psync_fs_crypto_init_log(psync_openfile_t *of);
int psync_fs_crypto_read_newfile_locked(psync_openfile_t *of, char *buf, uint64_t size, uint64_t offset);
int psync_fs_crypto_write_newfile_locked(psync_openfile_t *of, const char *buf, uint64_t size, uint64_t offset);
int psync_fs_crypto_flush_file(psync_openfile_t *of);

uint64_t psync_fs_crypto_plain_size(uint64_t cryptosize);

#endif