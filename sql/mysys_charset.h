#ifndef MYSYS_CHARSET
#define MYSYS_CHARSET

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


#include "my_sys.h"


/*
  An extention for Charset_loader_mysys,
  with server error and warning support.
*/
class Charset_loader_server: public Charset_loader_mysys
{
public:
  using Charset_loader_mysys::Charset_loader_mysys;
  void raise_unknown_collation_error(const char *name) const;
  void raise_not_applicable_error(const char *cs, const char *cl) const;

  /*
    Find a CHARSET_INFO by a character set and a
    contextually typed collation name.
    @param cs              - the character set
    @param context_cl_name - the context name, e.g. "uca1400_cs_ci"
    @returns               - a NULL pointer in case of failure, or
                             a CHARSET_INFO pointer on success.
  */
  CHARSET_INFO *get_exact_collation_by_context_name_or_error(CHARSET_INFO *cs,
                                                             const char *name,
                                                             myf my_flags)
  {
    CHARSET_INFO *ci= get_exact_collation_by_context_name(cs, name, my_flags);
    if (!ci)
      raise_not_applicable_error(cs->cs_name.str, name);
    return ci;
  }

  CHARSET_INFO *get_context_collation_or_error(const char *collation_name,
                                               myf my_flags)
  {
    CHARSET_INFO *ci= get_context_collation(collation_name, my_flags);
    if (!ci)
      raise_unknown_collation_error(collation_name);
    return ci;
  }

  CHARSET_INFO *get_bin_collation_or_error(CHARSET_INFO *cs,
                                           myf my_flags= MYF(0))
  {
    const char *cs_name= cs->cs_name.str;
    if (!(cs= get_bin_collation(cs, my_flags)))
    {
      char tmp[65];
      strxnmov(tmp, sizeof(tmp)-1, cs_name, "_bin", NULL);
      raise_unknown_collation_error(tmp);
    }
    return cs;
  }

  CHARSET_INFO *get_default_collation(CHARSET_INFO *cs,
                                      myf my_flags= MYF(0))
  {
    return Charset_loader_mysys::get_default_collation(cs, my_flags);
  }
};


#endif // MYSYS_CHARSET

