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

/*
 * Authors: Erik Eliasson <eliasson@it.kth.se>
 *          Johan Bilien <jobi@via.ecp.fr>
*/


/* Name
 * 	SipHeaderValueTo.cxx
 * Author
 * 	Erik Eliasson, eliasson@it.kth.se
 * Purpose
 * 
*/

#include<config.h>

#include<libmsip/SipHeaderTo.h>
#include<libmutil/trim.h>

MRef<SipHeaderValue *> toFactory(const string &build_from){
	                return new SipHeaderValueTo(build_from);
}

SipHeaderFactoryFuncPtr sipHeaderToFactory=toFactory;


const string sipHeaderValueToTypeStr = "To";
		
SipHeaderValueTo::SipHeaderValueTo()
		: SipHeaderValue(SIP_HEADER_TYPE_TO,sipHeaderValueToTypeStr)
{
}
		

SipHeaderValueTo::SipHeaderValueTo(const string &build_from) 
		: SipHeaderValue(SIP_HEADER_TYPE_TO,sipHeaderValueToTypeStr)
{
	uri = SipURI( build_from );
}


SipHeaderValueTo::SipHeaderValueTo(const string &username, const string &ip)
		: SipHeaderValue(SIP_HEADER_TYPE_TO,sipHeaderValueToTypeStr)
{
	uri.setParams(username, ip,"",0);
}

SipHeaderValueTo::~SipHeaderValueTo(){

}

string SipHeaderValueTo::getString(){
	//cerr << "To::get_string" << endl;
	string ret = /*"To: "+*/ uri.getString();
//	if (tag.length()>0)
//		ret=ret+";tag="+tag;
	
	return ret;
} 

SipURI &SipHeaderValueTo::getUri(){
	return uri;
}

void SipHeaderValueTo::setUri(const SipURI &uri){
	this->uri=uri;
}
		
/*
void SipHeaderValueTo::setTag(const string &tag){
	this->tag=tag;
}

string SipHeaderValueTo::getTag(){
	return tag;
}
*/

