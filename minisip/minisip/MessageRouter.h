/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

/* Copyright (C) 2004 
 *
 * Authors: Erik Eliasson <eliasson@it.kth.se>
 *          Johan Bilien <jobi@via.ecp.fr>
*/

#ifndef MESSAGE_ROUTER_H
#define MESSAGE_ROUTER_H

#include<libmsip/SipCallback.h>
#include<libmsip/SipInvite.h>
#include<libmsip/SipResponse.h>
//#include"SoundSender.h"
//#include"SoundReceiver.h"
#include"gui/GuiCallback.h"
//#include"sip/SipStateMachine.h"
#include"../sip/Sip.h"
#include"gui/Gui.h"
#include"../sip/SipSoftPhoneConfiguration.h"

#include<config.h>


class MessageRouter: public SipCallback, public GuiCallback{
	public:
		MessageRouter(MRef<SipSoftPhoneConfiguration *>phoneconf);
		virtual ~MessageRouter(){}
		
		void setSipStateMachine(MRef<Sip*> ssp);
		void setGui(Gui *guiptr){gui = guiptr;};
		void setMediaHandler(MRef<MediaHandler *> mediaHandler){
			this->mediaHandler = mediaHandler;}

		virtual void sipcb_handleCommand(CommandString &command);

		virtual void guicb_handleCommand(CommandString &command);
		virtual void guicb_handleMediaCommand(CommandString &command);

		virtual string guicb_doInvite(string sip_url);
			
	private:
		
		Gui *gui;
		MRef<Sip*> sip_machine;
		MRef<MediaHandler *> mediaHandler;
};

#endif
