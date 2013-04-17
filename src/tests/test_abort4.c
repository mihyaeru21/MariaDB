/* -*- mode: C; c-basic-offset: 4 -*- */
#ident "Copyright (c) 2007 Tokutek Inc.  All rights reserved."
#include "test.h"

//Verify aborting transactions works properly when transaction 
//starts with an empty db and a table lock.
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <db.h>
#include <memory.h>
#include <stdio.h>


// ENVDIR is defined in the Makefile

DB_ENV *env;
DB *db;
DB_TXN *null_txn = NULL;
DB_TXN *txn;
u_int32_t find_num;

static void
init(u_int32_t dup_flags) {
    int r;
    system("rm -rf " ENVDIR);
    r=toku_os_mkdir(ENVDIR, S_IRWXU+S_IRWXG+S_IRWXO); CKERR(r);
    r=db_env_create(&env, 0); CKERR(r);
    r=env->open(env, ENVDIR, DB_INIT_LOCK|DB_INIT_LOG|DB_INIT_MPOOL|DB_INIT_TXN|DB_PRIVATE|DB_CREATE, S_IRWXU+S_IRWXG+S_IRWXO); CKERR(r);
    r=db_create(&db, env, 0); CKERR(r);
    if (dup_flags) {
        r = db->set_flags(db, dup_flags);
        CKERR(r);
    }
    r=db->open(db, null_txn, "foo.db", 0, DB_BTREE, DB_CREATE|DB_EXCL, S_IRWXU|S_IRWXG|S_IRWXO);
    CKERR(r);
    r=db->close(db, 0); CKERR(r);
    r=db_create(&db, env, 0); CKERR(r);
    r=db->open(db, null_txn, "foo.db", 0, DB_BTREE, 0, S_IRWXU|S_IRWXG|S_IRWXO);
    CKERR(r);
    r=env->txn_begin(env, 0, &txn, 0); CKERR(r);
    r=db->pre_acquire_table_lock(db, txn); CKERR(r);
}

static void
tear_down(void) {
    int r;
    r = db->close(db, 0); CKERR(r);
    r = env->close(env, 0); CKERR(r);
}

static void
abort_txn(void) {
    find_num = 0;
    int r = txn->abort(txn); CKERR(r);
}

#ifndef DB_YESOVERWRITE
#define DB_YESOVERWRITE 0
#endif

static void
put(u_int32_t k, u_int32_t v) {
    int r;
    DBT key,val;

    dbt_init(&key, &k, sizeof(k));
    dbt_init(&val, &v, sizeof(v));
    r = db->put(db, txn, &key, &val, DB_YESOVERWRITE); CKERR(r);
}

static void
test_insert_and_abort(u_int32_t num_to_insert) {
    find_num = 0;
    
    u_int32_t k;
    u_int32_t v;

    u_int32_t i;
    for (i=0; i < num_to_insert; i++) {
        k = htonl(i);
        v = htonl(i+num_to_insert);
        put(k, v);
    }
    abort_txn();
}

static void
test_insert_and_abort_and_insert(u_int32_t num_to_insert) {
    test_insert_and_abort(num_to_insert); 
    find_num = num_to_insert / 2;
    u_int32_t k, v;
    u_int32_t i;
    for (i=0; i < find_num; i++) {
        k = htonl(i);
        v = htonl(i+5);
        put(k, v);
    }
}

#define bit0 (1<<0)
#define bit1 (1<<1)

static int
do_nothing(DBT const *UU(a), DBT  const *UU(b), void *UU(c)) {
    return 0;
}

static void
verify_and_tear_down(int close_first) {
    toku_struct_stat temp;
    int r;
    r = toku_stat(ENVDIR "/foo.db", &temp);
    CKERR(r);
    if (close_first) {
        r=db->close(db, 0); CKERR(r);
        r=db_create(&db, env, 0); CKERR(r);
        r=db->open(db, null_txn, "foo.db", 0, DB_BTREE, 0, S_IRWXU|S_IRWXG|S_IRWXO);
        CKERR(r);
    }
    DBC *cursor;
    r=env->txn_begin(env, 0, &txn, 0); CKERR(r);
    r = db->cursor(db, txn, &cursor, 0); CKERR(r);
    u_int32_t found = 0;
    do {
        r = cursor->c_getf_next(cursor, 0, do_nothing, NULL);
        if (r==0) found++;
    } while (r==0);
    CKERR2(r, DB_NOTFOUND);
    cursor->c_close(cursor);
    txn->commit(txn, 0);
    assert(found==find_num);
    tear_down();
}

static void
runtests(u_int32_t dup_flags) {
    int close_first;
    for (close_first = 0; close_first < 2; close_first++) {
        init(dup_flags);
        abort_txn();
        verify_and_tear_down(close_first);
        u_int32_t n;
        for (n = 1; n < 1<<20; n*=2) {
            init(dup_flags);
            test_insert_and_abort(n);
            verify_and_tear_down(close_first);

            init(dup_flags);
            test_insert_and_abort_and_insert(n);
            verify_and_tear_down(close_first);
        }
    }
}

int
test_main (int UU(argc), char UU(*argv[])) {
    runtests(0);
    runtests(DB_DUPSORT|DB_DUP);
    return 0;
}

