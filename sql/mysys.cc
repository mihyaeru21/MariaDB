/* Copyright (c) 2021, MariaDB Corporation.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1335  USA */

/*
  Convenience wrappers for mysys functionality
*/


#include "mariadb.h"
#include "sql_priv.h"
#include "sql_class.h"
#include "mysys_charset.h"


void Charset_loader_server::raise_unknown_collation_error(const char *name) const
{
  ErrConvString err(name, &my_charset_utf8mb4_general_ci);
  my_error(ER_UNKNOWN_COLLATION, MYF(0), err.ptr());
  if (error[0])
    push_warning_printf(current_thd,
                        Sql_condition::WARN_LEVEL_WARN,
                        ER_UNKNOWN_COLLATION, "%s", error);
}


void Charset_loader_server::raise_not_applicable_error(const char *cs,
                                                       const char *cl) const
{
  my_error(ER_COLLATION_CHARSET_MISMATCH, MYF(0), cl, cs);
}
