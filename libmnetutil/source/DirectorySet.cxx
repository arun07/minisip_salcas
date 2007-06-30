/*
  Copyright (C) 2007, Mikael Svensson

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
 * Authors:	Mikael Svensson
 *              Erik Eliasson <eliasson@it.kth.se>
 *          	Johan Bilien <jobi@via.ecp.fr>
 *          	Mikael Magnusson <mikma@users.sourceforge.net>
 */

#include <libmnetutil/libmnetutil_config.h>
#include <libmutil/MemObject.h>

#include <libmnetutil/DirectorySet.h>
#include <libmutil/stringutils.h>

DirectorySet::DirectorySet(){
	itemsIndex = items.begin();
}

DirectorySet::~DirectorySet(){
	std::list<MRef<DirectorySetItem*> >::iterator i;
	std::list<MRef<DirectorySetItem*> >::iterator last = items.end();

	items.clear();
}

DirectorySet* DirectorySet::clone(){
	DirectorySet * db = create();

	lock();
	std::list<MRef<DirectorySetItem*> >::iterator i;
	std::list<MRef<DirectorySetItem*> >::iterator last = items.end();

	for( i = items.begin(); i != last; i++ ){
		db->addItem( *i );
	}

	unlock();
	return db;
}

DirectorySet* DirectorySet::create() {
	return new DirectorySet();
}

void DirectorySet::lock(){
	mLock.lock();
}

void DirectorySet::unlock(){
	mLock.unlock();
}

void DirectorySet::addItem( MRef<DirectorySetItem*> item ){
	items.push_back( item );
	itemsIndex = items.begin();
}

MRef<DirectorySetItem*> DirectorySet::createItemLdap (std::string url, std::string subTree){
	MRef<DirectorySetItem*> item = new DirectorySetItem(url, subTree);

	return item;
}

void DirectorySet::addLdap (std::string url, std::string subTree) {
	MRef<DirectorySetItem*> item = createItemLdap(url, subTree);
	addItem(item);
}

void DirectorySet::remove( MRef<DirectorySetItem*> removedItem ){
	initIndex();

	while( itemsIndex != items.end() ){
		if( **(*itemsIndex) == **removedItem ){
			items.erase( itemsIndex );
			initIndex();
			return;
		}
		itemsIndex ++;
	}
	initIndex();
}

std::list<MRef<DirectorySetItem*> > &DirectorySet::getItems(){
	return items;
}
std::list<MRef<DirectorySetItem*> > DirectorySet::findItems(const std::string subTree, bool endsWithIsEnough) {
	std::list<MRef<DirectorySetItem*> > res;
	initIndex();

	while( itemsIndex != items.end() ){
		MRef<DirectorySetItem*> item = *itemsIndex;
		bool addThis = false;

		if ((endsWithIsEnough && stringEndsWith(item->getSubTree(), subTree)) || item->getSubTree() == subTree)
			res.push_back(item);

		itemsIndex++;
	}
	initIndex();
	return res;
}


void DirectorySet::initIndex(){
	itemsIndex = items.begin();
}

MRef<DirectorySetItem*> DirectorySet::getNext(){
	MRef<DirectorySetItem*> tmp;

	if( itemsIndex == items.end() ){
		itemsIndex = items.begin();
		return NULL;
	}

	tmp = *itemsIndex;
	itemsIndex++;
	return tmp;
}
