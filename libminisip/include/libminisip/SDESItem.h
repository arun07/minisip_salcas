/*
  Copyright (C) 2005, 2004 Erik Eliasson, Johan Bilien
  
  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/


/* Copyright (C) 2004 
 *
 * Authors: Erik Eliasson <eliasson@it.kth.se>
 *          Johan Bilien <jobi@via.ecp.fr>
*/

#ifndef SDESITEM_H
#define SDESITEM_H

#ifdef _MSC_VER
#ifdef LIBMINISIP_EXPORTS
#define LIBMINISIP_API __declspec(dllexport)
#else
#define LIBMINISIP_API __declspec(dllimport)
#endif
#else
#define LIBMINISIP_API
#endif


#include<config.h>

#include<vector>

#define CNAME 1
#define NAME 2
#define EMAIL 3
#define PHONE 4
#define LOC 5
#define TOOL 6
#define NOTE 7


class LIBMINISIP_API SDESItem{
	public:
//		virtual vector<unsigned char> get_bytes()=0;
		virtual int size()=0;
		static SDESItem *build_from(void *from,int max_length);

#ifdef DEBUG_OUTPUT
		virtual void debug_print()=0;
#endif
	protected:
//		unsigned type;
		unsigned length;
};

#endif
