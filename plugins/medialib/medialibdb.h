/*
    DeaDBeeF -- the music player
    Copyright (C) 2009-2021 Oleksiy Yakovenko and other contributors

    This software is provided 'as-is', without any express or implied
    warranty.  In no event will the authors be held liable for any damages
    arising from the use of this software.

    Permission is granted to anyone to use this software for any purpose,
    including commercial applications, and to alter it and redistribute it
    freely, subject to the following restrictions:

    1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.

    2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.

    3. This notice may not be removed or altered from any source distribution.
*/

#ifndef medialibdb_h
#define medialibdb_h

#include <stdint.h>
#include "../../deadbeef.h"
#include "medialibstate.h"

typedef struct ml_collection_item_s {
    uint64_t row_id;

    ddb_playItem_t *it;
    struct ml_collection_item_s *next; // next item in the same collection (albums, artists, ...)
} ml_collection_item_t;

// A string with associated "collection_items", stored as a hash and as a list.
// Basically, this is the "tree node".
typedef struct ml_string_s {
    uint64_t row_id; // a unique ID of the item, which is valid only during single session (will be different after deadbeef restarts)

    const char *text;

    ml_collection_item_t *items; // list of all leaf items (tracks) in this node (e.g. all tracks in an album)
    ml_collection_item_t *items_tail; // tail, for fast append
    int items_count; // count of items

    struct ml_string_s *bucket_next; // next node in a hash bucket
    struct ml_string_s *next; // next node in a list

    // to support tree hierarchy
    struct ml_string_s *children;
    struct ml_string_s *children_tail;

    // Note: this is "temporary state" stuff, used when processing a tree query
    struct ml_tree_item_s *coll_item; // The item associated with collection string
    struct ml_tree_item_s *coll_item_tail; // Tail of the children list of coll_item
} ml_string_t;

#define ML_HASH_SIZE 4096

// This is the collection "container" -- that is, item tree with a hash table.
typedef struct {
    ml_string_t *hash[ML_HASH_SIZE]; // hash table, for quick lookup by name (pointer-based hash)
    ml_string_t root;
} ml_collection_t;

typedef struct ml_entry_s {
    const char *file;
    const char *title;
    int subtrack;
    ml_string_t *artist;
    ml_string_t *album;
    ml_string_t *genre;
    ml_string_t *folder;
    ml_string_t *track_uri;
    struct ml_entry_s *next;
    struct ml_entry_s *bucket_next;
} ml_entry_t;

typedef struct ml_cached_string_s {
    const char *s;
    struct ml_cached_string_s *next;
} ml_cached_string_t;

typedef struct {
    // Plain list of all tracks in the entire collection
    // The purpose is to hold references to all metadata strings, used by the DB
    ml_entry_t *tracks;

    // hash formed by filename pointer
    // this hash purpose is to quickly check whether the filename is in the library already
    // NOTE: this hash doesn't contain all of the tracks from the `tracks` list, because of subtracks
    ml_entry_t *filename_hash[ML_HASH_SIZE];

    // plain lists for each index
    ml_collection_t albums;
    ml_collection_t artists;
    ml_collection_t genres;

    // for the folders, a tree structure is used
    ml_collection_t folders_tree;

    // This hash is formed from track_uri ([%:TRACKNUM%#]%:URI%), and supposed to have all tracks from the `tracks` list
    // Main purpose is to find a library instance of a track for given track pointer
    ml_collection_t track_uris;

    // list of all strings which are not referenced by tracks
    ml_cached_string_t *cached_strings;

    /// Selected / expanded state
    ml_collection_state_t state;

    uint64_t row_id; // increment for each new ml_collection_item_t
} ml_db_t;

uint32_t
hash_for_ptr (void *ptr);

ml_string_t *
hash_find_for_hashkey (ml_string_t **hash, const char *val, uint32_t h);

ml_string_t *
hash_find (ml_string_t **hash, const char *val);

void
ml_free_col (ml_db_t *db, ml_collection_t *coll);

ml_string_t *
ml_reg_col (ml_db_t *db, ml_collection_t *coll, const char /* nonnull */ *c, ddb_playItem_t *it, uint64_t coll_row_id, uint64_t item_row_id);

void
_reuse_row_ids (ml_collection_t *coll, const char *coll_name, ddb_playItem_t *item, ml_collection_state_t *state, ml_collection_state_t *saved_state, uint64_t *coll_rowid, uint64_t *item_rowid);

void
ml_reg_item_in_folder (ml_db_t *db, ml_string_t *node, const char *path, ddb_playItem_t *it, uint64_t use_this_row_id);

void
ml_db_free (ml_db_t *db);

void
ml_db_init (DB_functions_t *_deadbeef);

#endif /* medialibdb_h */

