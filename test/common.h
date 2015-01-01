/*
   fskit: a library for creating multi-threaded in-RAM filesystems
   Copyright (C) 2014  Jude Nelson

   This program is dual-licensed: you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License version 3 or later as
   published by the Free Software Foundation. For the terms of this
   license, see LICENSE.LGPLv3+ or <http://www.gnu.org/licenses/>.

   You are free to use this program under the terms of the GNU Lesser General
   Public License, but WITHOUT ANY WARRANTY; without even the implied
   warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
   See the GNU Lesser General Public License for more details.

   Alternatively, you are free to use this program under the terms of the
   Internet Software Consortium License, but WITHOUT ANY WARRANTY; without
   even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
   For the terms of this license, see LICENSE.ISC or
   <http://www.isc.org/downloads/software-support-policy/isc-license/>.
*/

#ifndef _TEST_COMMON_H_
#define _TEST_COMMON_H_

#include "fskit.h"

void fskit_type_to_string( int type, char type_buf[10] );

int fskit_print_tree( FILE* out, struct fskit_entry* root );

int fskit_test_begin( struct fskit_core* core, void* test_data );
int fskit_test_end( struct fskit_core* core, void** test_data );

int fskit_test_mkdir_LR_recursive( struct fskit_core* core, char const* path, int depth );

#endif
