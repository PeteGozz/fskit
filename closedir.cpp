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

#include "closedir.h"

// free a directory handle
static void fskit_dir_handle_destroy( struct fskit_dir_handle* dirh, void** app_handle_data ) {
   
   dirh->dent = NULL;
   
   if( app_handle_data ) {
      *app_handle_data = dirh->app_data;
   }
   
   if( dirh->path != NULL ) {
      free( dirh->path );
      dirh->path = NULL;
   }
   
   pthread_rwlock_destroy( &dirh->lock );
   
   memset( dirh, 0, sizeof(struct fskit_dir_handle) );
   
   free( dirh );
}


// close a directory handle, freeing it.  On success, its handle-specific app data will be put into app_handle_data.
// a directory may be unlinked on close, if this was the last handle to it, and its link count was zero.
// if this happens, then app_dir_data will contain the directory's app data, and the directory will be freed.
// return 0 on success
// return the following errors:
// * EBADF if the directory handle is invalid
// * EDEADLK if there is a bug in the lock handling
int fskit_closedir( struct fskit_dir_handle* dirh, void** app_handle_data, void** app_dir_data ) {
   
   int rc = 0;
   
   rc = fskit_dir_handle_wlock( dirh );
   if( rc != 0 ) {
      // indicates deadlock; shouldn't happen 
      errorf("BUG: fskit_dir_handle_wlock(%p) rc = %d\n", dirh, rc );
      return rc;
   }
   
   if( dirh->dent == NULL ) {
      
      fskit_dir_handle_unlock( dirh );
      return -EBADF;
   }
   
   rc = fskit_entry_wlock( dirh->dent );
   if( rc != 0 ) {
      
      // shouldn't happen; indicates deadlock 
      errorf("BUG: fskit_entry_wlock(%p) rc = %d\n", dirh->dent, rc );
      
      fskit_dir_handle_unlock( dirh );
      return rc;
   }
   
   // see if we can destroy this....
   rc = fskit_entry_try_destroy( dirh->dent, app_dir_data );
   if( rc > 0 ) {
      
      // dent was unlocked and destroyed
      free( dirh->dent );
      rc = 0;
   }
   else if( rc < 0 ) {
      
      // some error occurred 
      errorf("fskit_entry_try_destroy(%p) rc = %d\n", dirh->dent, rc );
      fskit_entry_unlock( dirh->dent );
      
      return rc;
   }
   else {
      // done with this directory
      fskit_entry_unlock( dirh->dent );
   }
   
   // give back the app data 
   if( app_handle_data != NULL ) {
      *app_handle_data = dirh->app_data;
   }

   // get rid of this handle
   fskit_dir_handle_unlock( dirh );
   fskit_dir_handle_destroy( dirh, app_handle_data );
   
   return 0;

}
