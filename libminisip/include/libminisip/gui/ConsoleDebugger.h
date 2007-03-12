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

#ifndef _CONSOLEDEBUGGER_H
#define _CONSOLEDEBUGGER_H

#include<libminisip/libminisip_config.h>

#include<libmutil/MemObject.h>
#include<libmutil/Thread.h>

#include<libminisip/signaling/sip/SipSoftPhoneConfiguration.h>
#include<libminisip/media/MediaHandler.h>

#include<string>

class LIBMINISIP_API ConsoleDebugger : public Runnable{
	public:
		ConsoleDebugger(MRef<SipSoftPhoneConfiguration*> conf);
		~ConsoleDebugger();
		
		std::string getMemObjectType() const {return "ConsoleDebugger";}
		
		void showHelp();
		void showMem();
		void showMemSummary();
		void showStat();
		void showConfig();

		void sendManagementCommand( std::string str );
		void sendCommandToMediaHandler( std::string str );
		
		MRef<Thread *> start();
		
		virtual void run();
		
		void stop();
		
		void join();
		
		void setMediaHandler( MRef<MediaHandler *> r ) {
			mediaHandler = r; 
			if (mediaHandler) 
				std::cerr << "EEEE: mediaHandler set!"<<std::endl; 
			else 
				std::cerr << "EEEE: mediaHandler set to NULL"<<std::endl;
		}

	private:
		MRef<SipStack*> sipStack;
		MRef<MediaHandler *> mediaHandler;
		
		MRef<Thread *> thread;
		bool keepRunning;
		MRef<SipSoftPhoneConfiguration*> config;
};

#endif
