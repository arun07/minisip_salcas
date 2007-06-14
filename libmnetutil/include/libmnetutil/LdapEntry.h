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

#ifndef LDAPENTRY_H_
#define LDAPENTRY_H_

#include<libmnetutil/libmnetutil_config.h>

#include <ldap.h>
#include <lber.h>
#include <libmnetutil/LdapException.h>
#include <libmutil/MemObject.h>
#include <libmutil/stringutils.h>

#include <string>
#include <map>
#include <vector>

class certificate_pair {}; //Dummy entry

/**
 * Class used internally by LdapEntry
 *
 * @author	Mikael Svensson
 */
class LdapEntryBinaryValue : public MObject {
	public:
		LdapEntryBinaryValue(char* v, int l) : length(l) {
			value = new char[length];
			memcpy(value, v, length);
		}
		~LdapEntryBinaryValue() {
			delete[] value;
		}
		char* value;
		int length;
};

/**
 * Represents one object in the LDAP directory, including its attribute values.
 *
 * When the class instance is created it read an entire entry from an LDAP repsonse message
 * and stores all information internally. This means that once all the LdapEntry objects
 * have been created it is safe to free the memory allocated by the response message itself.
 *
 * The class differentiates between binary and non-binary attribute values (anything that is
 * not binary is stored as strings). The separation is made based on the returned attribute
 * names as it is assumed that all binary attributes have name which end with ";binary".
 *
 * Binary attribute values are stored in LdapEntryBinaryValue object whereas all other values
 * are stored as strings.
 *
 * @author	Mikael Svensson
 */
class LdapEntry : public MObject {
	public:
		LdapEntry(LDAP* ld, LDAPMessage* entry);
		//~LdapEntry();

		/**
		 * Return THE FIRST string value for a given attribute.
		 */
		std::string getAttrValueString(std::string attr) throw (LdapAttributeNotFoundException);

		/**
		 * Return all binary values for a given attribute.
		 */

		std::vector< MRef<LdapEntryBinaryValue*> > getAttrValuesBinary(std::string attr) throw (LdapAttributeNotFoundException);
		/**
		 * Return ALL string values for a given attribute.
		 */
		std::vector<std::string> getAttrValuesStrings(std::string attr) throw (LdapAttributeNotFoundException);

		/**
		 * Specialized method that returns all certificate pair attribute values (given a specific attribute name).
		 */
		std::vector<MRef<certificate_pair*> > getAttrValuesCertificatePairs(std::string attr) throw (LdapAttributeNotFoundException);

		//std::string getDn();

		/**
		 * Returns list of all attribute names.
		 */
		std::vector<std::string> getAttrNames();

		/**
		 * Tests if the directory entry/object has a specific attribute.
		 */
		bool hasAttribute(std::string attr);
	private:
		std::map<std::string, std::vector<MRef<LdapEntryBinaryValue*> > > valuesBinary;
		std::map<std::string, std::vector<std::string> > valuesStrings;
};

#endif
