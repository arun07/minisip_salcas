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
 * 	SipSoftPhoneCallback.h
 * Author
 * 	Erik Eliasson, eliasson@it.kth.se
 * Purpose
 * 
*/


#ifndef SIPCALLBACK_H
#define SIPCALLBACK_H

#ifdef _MSC_VER
#ifdef LIBMSIP_EXPORTS
#define LIBMSIP_API __declspec(dllexport)
#else
#define LIBMSIP_API __declspec(dllimport)
#endif
#else
#define LIBMSIP_API
#endif

#include<libmutil/CommandString.h>

/**
 * Callback interface for the SipSoftPhoneCallback.
 * This class defines all callbacks that a instance of the 
 * SipStateMachine class does.
 * @author Erik Eliasson, eliasson@it.kth.se
 * @version 0.00
 * @see SipStateMachine
 */
class LIBMSIP_API SipCallback{
	public:
		/**
		 * A incoming call is available. This method is intended to signal
		 * that the SipStateMachine wants to be informed whether to accept
		 * or reject
		 * @param invite The invite message of the calling user agent.
		 */
		virtual void sipcb_handleCommand(CommandString &command)=0;
		virtual void sipcb_handleConfCommand(CommandString &command)=0;
		
};


#endif
