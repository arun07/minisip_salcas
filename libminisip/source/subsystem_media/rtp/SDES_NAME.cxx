/*
 Copyright (C) 2004-2006 the Minisip Team
 
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
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 */

/* Copyright (C) 2004 
 *
 * Authors: Erik Eliasson <eliasson@it.kth.se>
 *          Johan Bilien <jobi@via.ecp.fr>
*/

#include <config.h>

#include<libmutil/massert.h>
#include<libminisip/media/rtp/SDES_NAME.h>

#ifdef DEBUG_OUTPUT
#include<iostream>
#endif

using namespace std;

SDES_NAME::SDES_NAME(void *buildfrom, int /*max_length*/){
	unsigned char *lengthptr = (unsigned char *)buildfrom;
	lengthptr++;
	length=*lengthptr;
	
	char *cptr = (char *)buildfrom;
	massert(*cptr == NAME);
//	type=*cptr;
	name="";
	
	cptr+=2;
	for (int i=0 ; i<*lengthptr; i++){
		name+=*cptr;
		cptr++;
	}	
#ifdef DEBUG_OUTPUT
	cerr << "In SDES_NAME: Parsed string name to <"<< name << ">"<< endl;
#endif
}

int SDES_NAME::size(){
//	cerr << "WARNING: returning unaligned size - FIX"<< endl;
//	int npad = 4-((2+cname.length())%4);
//	if (npad==4)
//		npad=0;
	return 2+(int)name.length()/*+npad*/;
}

#ifdef DEBUG_OUTPUT
void SDES_NAME::debug_print(){
	cerr << "SDES NAME:"<< endl;
	cerr << "\tlength: "<< length << endl;
	cerr << "\tname: "<< name << endl;
}
#endif
