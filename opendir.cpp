/*
   fskit: a library for creating multi-threaded in-RAM filesystems
   Copyright (C) 2014  Jude Nelson

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "opendir.h"
#include "path.h"

// create a directory handle from an fskit_entry
static struct fskit_dir_handle* fskit_dir_handle_create( struct fskit_entry* dir, char const* path, void* app_handle_data ) {
   
   struct fskit_dir_handle* dirh = CALLOC_LIST( struct fskit_dir_handle, 1 );
   
   dirh->dent = dir;
   dirh->path = strdup( path );
   dirh->file_id = dir->file_id;
   dirh->app_data = app_handle_data;
   
   pthread_rwlock_init( &dirh->lock, NULL );
   
   return dirh;
}

 
// open a directory.
// On success, return an fskit_dir_handle, which will have its app_data field initialized to the given app_handle_data
// Return any error code via the *err argument (which will be non-zero if there was an error).
// on error, return NULL, and set *err appropriately:
// * -ENAMETOOLONG if _path is too long
// * -EACCES if some part of _path is in accessible to the given user and group
// * -ENOTDIR if the entry referred to by _path isn't a directory
// * -ENOENT if the entry doesn't exist
struct fskit_dir_handle* fskit_opendir( struct fskit_core* core, char const* _path, uint64_t user, uint64_t group, void* app_handle_data, int* err ) {
   
   if( strlen(_path) >= PATH_MAX ) {
      // too long 
      *err = -ENAMETOOLONG;
      return NULL;
   }
   
   // ensure path ends in /
   char path[PATH_MAX];
   memset( path, 0, PATH_MAX );
   strncpy( path, _path, PATH_MAX - 1 );

   fskit_sanitize_path( path );
   
   struct fskit_entry* dir = fskit_entry_resolve_path( core, path, user, group, true, err );
   if( dir == NULL ) {
      // resolution error; err is set appropriately
      return NULL;
   }
   
   // make sure it's a directory
   if( dir->type != FSKIT_ENTRY_TYPE_DIR ) {
      *err = -ENOTDIR;
      fskit_entry_unlock( dir );
      return NULL;
   }

   // open this directory
   dir->open_count++;
   
   // make a handle to it
   struct fskit_dir_handle* dirh = fskit_dir_handle_create( dir, path, app_handle_data );
   
   // release the directory
   fskit_entry_unlock( dir );
   
   return dirh;
}
