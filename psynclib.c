/* Copyright (c) 2013 Anton Titov.
 * Copyright (c) 2013 pCloud Ltd.
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

#include <string.h>
#include "psynclib.h"
#include "pcompat.h"
#include "plibs.h"
#include "pcallbacks.h"
#include "pdatabase.h"
#include "pstatus.h"
#include "pdiff.h"
#include "pssl.h"

psync_malloc_t psync_malloc=malloc;
psync_realloc_t psync_realloc=realloc;
psync_free_t psync_free=free;

const char *psync_database=NULL;

PSYNC_THREAD uint32_t psync_error=0;

#define return_error(err) do {psync_error=err; return -1;} while (0)

uint32_t psync_get_last_error(){
  return psync_error;
}

void psync_set_database_path(const char *databasepath){
  psync_database=psync_strdup(databasepath);
}

void psync_set_alloc(psync_malloc_t malloc_call, psync_realloc_t realloc_call, psync_free_t free_call){
  psync_malloc=malloc_call;
  psync_realloc=realloc_call;
  psync_free=free_call;
}

int psync_init(){
  if (psync_ssl_init())
    return_error(PERROR_SSL_INIT_FAILED);
  if (!psync_database){
    psync_database=psync_get_default_database_path();
    if (!psync_database)
      return_error(PERROR_NO_HOMEDIR);
  }
  if (psync_sql_connect(psync_database) || psync_sql_statement(PSYNC_DATABASE_STRUCTURE))
    return_error(PERROR_DATABASE_OPEN);
  psync_status_init();
  return 0;
}

void psync_start_sync(pstatus_change_callback_t status_callback, pevent_callback_t event_callback){
  psync_run_thread(psync_diff_thread);
  if (status_callback)
    psync_set_status_callback(status_callback);
  if (event_callback)
    psync_set_event_callback(event_callback);
}

uint32_t psync_download_state(){
  return 0;
}

void psync_destroy(){
  psync_do_run=0;
  psync_send_status_update();
  psync_milisleep(20);
  psync_sql_lock();
  psync_sql_close();
}

char *psync_get_username(){
  return psync_sql_cellstr("SELECT value FROM setting WHERE id='user'");
}

static void clear_db(int save){
  char *sql;
  psync_sql_statement("DELETE FROM setting WHERE id IN ('pass', 'auth')");
  if (save)
    sql="REPLACE INTO setting (id, value) VALUES ('saveauth', 1)";
  else
    sql="REPLACE INTO setting (id, value) VALUES ('saveauth', 0)";
  psync_sql_statement(sql);
}

static void save_to_db(const char *key, const char *val){
  psync_sql_res *q;
  q=psync_sql_prep_statement("REPLACE INTO setting (id, value) VALUES (?, ?)");
  if (q){
    psync_sql_bind_string(q, 1, key);
    psync_sql_bind_string(q, 2, val);
    psync_sql_run(q);
    psync_sql_free_result(q);
  }
}

void psync_set_user_pass(const char *username, const char *password, int save){
  clear_db(save);
  if (save){
    save_to_db("user", username);
    save_to_db("pass", password);
  }
  else{
    pthread_mutex_lock(&psync_my_auth_mutex);
    psync_free(psync_my_user);
    psync_my_user=psync_strdup(username);
    psync_free(psync_my_pass);
    psync_my_pass=psync_strdup(password);
    pthread_mutex_unlock(&psync_my_auth_mutex);
  }
  psync_set_status(PSTATUS_TYPE_AUTH, PSTATUS_AUTH_PROVIDED);
}

void psync_set_pass(const char *password, int save){
  clear_db(save);
  if (save)
    save_to_db("pass", password);
  else{
    pthread_mutex_lock(&psync_my_auth_mutex);
    psync_free(psync_my_pass);
    psync_my_pass=psync_strdup(password);
    pthread_mutex_unlock(&psync_my_auth_mutex);
  }
  psync_set_status(PSTATUS_TYPE_AUTH, PSTATUS_AUTH_PROVIDED);
}

void psync_set_auth(const char *auth, int save){
  clear_db(save);
  if (save)
    save_to_db("auth", auth);
  else{
    pthread_mutex_lock(&psync_my_auth_mutex);
    psync_free(psync_my_auth);
    psync_my_pass=psync_strdup(auth);
    pthread_mutex_unlock(&psync_my_auth_mutex);
  }
  psync_set_status(PSTATUS_TYPE_AUTH, PSTATUS_AUTH_PROVIDED);
}

void psync_unlink(){
}