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
 *	    Joachim Orrblad <joachim@orrblad.com>
*/

#include<config.h>


#include"Session.h"
#include"MediaStream.h"

#ifndef NO_SECURITY

#include<libmutil/Timestamp.h>
#include<libmutil/dbg.h>
#include"../sip/SipDialogSecurityConfig.h"

#include<libmikey/keyagreement.h>
#include<libmikey/keyagreement_dh.h>
#include<libmikey/keyagreement_psk.h>
#include<libmikey/MikeyException.h>

#define MIKEY_PROTO_SRTP	0


using namespace std;


bool Session::responderAuthenticate( string message ){
	
	bool authenticated;
	
	if(message.substr(0,6) == "mikey "){

		string b64Message = message.substr(6, message.length()-6);

		if( message == "" )
			throw MikeyException( "No MIKEY message received" );
		else {
			try{
				MikeyMessage * init_mes = new MikeyMessage(b64Message);
				
//				MikeyMessage * resp_mes = NULL;
				switch( init_mes->type() ){
					case MIKEY_TYPE_DH_INIT:

						if( securityConfig.cert.isNull() ){
							merr << "No certificate available" << end;
						//	throw new MikeyExceptionUnacceptable(
						//			"Cannot handle DH key agreement, no certificate" );
							securityConfig.secured = false;
							securityConfig.ka_type = KEY_MGMT_METHOD_NULL;
							return false;
						}
							

						if( !securityConfig.dh_enabled ){
							merr << "Cannot handle DH key agreement" << end;
							//throw new MikeyExceptionUnacceptable(
							//		"Cannot handle DH key agreement" );
							securityConfig.secured = false;
							securityConfig.ka_type = KEY_MGMT_METHOD_NULL;
							return false;
						}

						if( !ka ){
							ka = new KeyAgreementDH( securityConfig.cert, securityConfig.cert_db, DH_GROUP_OAKLEY5 );
						}
						ka->setInitiatorData( init_mes );

#ifndef _MSC_VER
						ts.save( AUTH_START );
#endif
						if( init_mes->authenticate( ((KeyAgreementDH *)*ka) ) ){
							merr << "Authentication of the DH init message failed" << end;
//							throw new MikeyExceptionAuthentication(
//								"Authentication of the DH init message failed" );
							merr << ka->authError() << end;
							securityConfig.secured = false;
							securityConfig.ka_type = KEY_MGMT_METHOD_NULL;
							return false;
						}

						merr << "Authentication successful, controling the certificate" << end;

#ifndef _MSC_VER
						ts.save( TMP );
#endif
						if( securityConfig.check_cert ){
							if( ((KeyAgreementDH *)*ka)->controlPeerCertificate() == 0){
#ifdef DEBUG_OUTPUT
								merr << "Certificate check failed in the incoming MIKEY message" << end;
#endif
								securityConfig.secured = false;
								securityConfig.ka_type = KEY_MGMT_METHOD_NULL;
								return false;
							}
						}
#ifndef _MSC_VER
						ts.save( AUTH_END );
#endif

						securityConfig.ka_type = KEY_MGMT_METHOD_MIKEY_DH;

						break;
					case MIKEY_TYPE_PSK_INIT:
						if( !securityConfig.psk_enabled ){
							//throw new MikeyExceptionUnacceptable(
							//		"Cannot handle PSK key agreement" );

							securityConfig.secured = false;
							securityConfig.ka_type = KEY_MGMT_METHOD_NULL;
							return false;
						}

						ka = new KeyAgreementPSK( securityConfig.psk, securityConfig.psk_length );
						ka->setInitiatorData( init_mes );
						
#ifndef _MSC_VER
						ts.save( AUTH_START );
#endif

						if( init_mes->authenticate( ((KeyAgreementPSK *)*ka) ) ){
//							throw new MikeyExceptionAuthentication(
//								"Authentication of the PSK init message failed" );
							securityConfig.secured = false;
							securityConfig.ka_type = KEY_MGMT_METHOD_NULL;
							return false;
						}
						
#ifndef _MSC_VER
						ts.save( AUTH_END );
#endif

						securityConfig.ka_type = KEY_MGMT_METHOD_MIKEY_PSK;
						break;
					case MIKEY_TYPE_PK_INIT:
						//throw new MikeyExceptionUnimplemented(
						//	"Public Key key agreement not implemented" );
						securityConfig.secured = false;
						securityConfig.ka_type = KEY_MGMT_METHOD_NULL;
						return false;
					default:
						merr << "Unexpected type of message in INVITE" << end;
						securityConfig.secured = false;
						securityConfig.ka_type = KEY_MGMT_METHOD_NULL;
						return false;
				}

				securityConfig.secured = true;
				authenticated = true;
			}
			catch( certificate_exception *exc ){
				// TODO: Tell the GUI
				merr << "Could not open certificate" <<end;
				securityConfig.ka_type = KEY_MGMT_METHOD_NULL;
				securityConfig.secured = false;
				authenticated = false;
				delete exc;
			}
			catch( MikeyExceptionUnacceptable *exc ){
				merr << "MikeyException caught: "<<exc->message()<<end;
				//FIXME! send SIP Unacceptable with Mikey Error message
				securityConfig.ka_type = KEY_MGMT_METHOD_NULL;
				securityConfig.secured = false;
				authenticated = false;
				delete exc;
			}
			// Authentication failed
			catch( MikeyExceptionAuthentication *exc ){
				merr << "MikeyExceptionAuthentication caught: "<<exc->message()<<end;
				//FIXME! send SIP Authorization failed with Mikey Error message
				securityConfig.ka_type = KEY_MGMT_METHOD_NULL;
				securityConfig.secured = false;
				authenticated = false;
				delete exc;
			}
			// Message was invalid
			catch( MikeyExceptionMessageContent *exc ){
				MikeyMessage * error_mes;
				merr << "MikeyExceptionMesageContent caught: " << exc->message() << end;
				if( ( error_mes = exc->errorMessage() ) != NULL ){
					//FIXME: send the error message!
				}
				securityConfig.ka_type = KEY_MGMT_METHOD_NULL;
				securityConfig.secured = false;
				authenticated = false;
				delete exc;
			}
			catch( MikeyException * exc ){
				merr << "MikeyException caught: " << exc->message() << end;
				securityConfig.ka_type = KEY_MGMT_METHOD_NULL;
				securityConfig.secured = false;
				authenticated = false;
				delete exc;
			}
		
		}
	}
	else {
		merr << "Unknown type of key agreement" << end;
		securityConfig.ka_type = KEY_MGMT_METHOD_NULL;
		securityConfig.secured = false;
		authenticated = true;
	}
	return authenticated;
}

string Session::responderParse(){
	
	if( ! ( securityConfig.ka_type & KEY_MGMT_METHOD_MIKEY ) ){
		merr << "Unknown type of key agreement" << end;
		securityConfig.secured = false;
		return "";
	}
	
	MikeyMessage * responseMessage = NULL;
	MikeyMessage * initMessage = (MikeyMessage *)ka->initiatorData();

	if( initMessage == NULL ){
		merr << "Uninitialized message, this is a bug" << end;
		securityConfig.secured = false;
		return "";
	}
	
	try{
		switch( securityConfig.ka_type ){
			case KEY_MGMT_METHOD_MIKEY_DH:
#ifndef _MSC_VER
				ts.save( MIKEY_PARSE_START );
#endif

				addStreamsToKa( false );
				responseMessage = initMessage->buildResponse((KeyAgreementDH *)*ka);
#ifndef _MSC_VER
				ts.save( MIKEY_PARSE_END );
#endif
				break;
			case KEY_MGMT_METHOD_MIKEY_PSK:
#ifndef _MSC_VER
				ts.save( MIKEY_PARSE_START );
#endif
				
				addStreamsToKa( false );

				responseMessage = initMessage->buildResponse((KeyAgreementPSK *)*ka);
#ifndef _MSC_VER
				ts.save( MIKEY_PARSE_END );
#endif
				break;
			case KEY_MGMT_METHOD_MIKEY_PK:
				/* Should not happen at that point */
				throw new MikeyExceptionUnimplemented(
						"Public Key key agreement not implemented" );
				break;
			default:
				throw new MikeyExceptionMessageContent(
						"Unexpected type of message in INVITE" );
		}
	}
	catch( certificate_exception *exc ){
		// TODO: Tell the GUI
		merr << "Could not open certificate" <<end;
		securityConfig.ka_type = KEY_MGMT_METHOD_NULL;
		securityConfig.secured = false;
		delete exc;
	}
	catch( MikeyExceptionUnacceptable *exc ){
		merr << "MikeyException caught: "<<exc->message()<<end;
		//FIXME! send SIP Unacceptable with Mikey Error message
		securityConfig.ka_type = KEY_MGMT_METHOD_NULL;
		securityConfig.secured = false;
		delete exc;
	}
	// Message was invalid
	catch( MikeyExceptionMessageContent *exc ){
		MikeyMessage * error_mes;
		merr << "MikeyExceptionMesageContent caught: " << exc->message() << end;
		if( ( error_mes = exc->errorMessage() ) != NULL ){
			responseMessage = error_mes;
		}
		securityConfig.ka_type = KEY_MGMT_METHOD_NULL;
		securityConfig.secured = false;
		delete exc;
	}
	catch( MikeyException * exc ){
		merr << "MikeyException caught: " << exc->message() << end;
		securityConfig.ka_type = KEY_MGMT_METHOD_NULL;
		securityConfig.secured = false;
		delete exc;
	}

	if( responseMessage != NULL ){
		//merr << "Created response message" << responseMessage->get_string() << end;
		return responseMessage->b64Message();
	}
	else{
		//merr << "No response message" << end;
		return string("");
	}


}


string Session::initiatorCreate(){
	MikeyMessage * message;
	
	
	try{
		switch( securityConfig.ka_type ){
			case KEY_MGMT_METHOD_MIKEY_DH:
				if( !securityConfig.cert || securityConfig.cert->is_empty() ){
					throw new MikeyException( "No certificate provided for DH key agreement" );
				}
#ifndef _MSC_VER
				ts.save( DH_PRECOMPUTE_START );
#endif

				if( ka && ka->type() != KEY_AGREEMENT_TYPE_DH ){
					ka = NULL;
				}
				if( !ka ){
					ka = new KeyAgreementDH( securityConfig.cert, securityConfig.cert_db, DH_GROUP_OAKLEY5 );
				}
				addStreamsToKa();
#ifndef _MSC_VER
				ts.save( DH_PRECOMPUTE_END );
#endif
				message = new MikeyMessage( ((KeyAgreementDH *)*ka) );
#ifndef _MSC_VER
				ts.save( MIKEY_CREATE_END );
#endif
				break;
			case KEY_MGMT_METHOD_MIKEY_PSK:
#ifndef _MSC_VER
				ts.save( DH_PRECOMPUTE_START );
#endif
				ka = new KeyAgreementPSK( securityConfig.psk, securityConfig.psk_length );
				addStreamsToKa();
#ifndef _MSC_VER
				ts.save( DH_PRECOMPUTE_END );
#endif
				((KeyAgreementPSK *)*ka)->generateTgk();
#ifndef _MSC_VER
				ts.save( MIKEY_CREATE_START );
#endif
				fprintf( stderr, "Before new MikeyMessage\n"); // Debug
				message = new MikeyMessage( ((KeyAgreementPSK *)*ka) );
				fprintf( stderr, "After new MikeyMessage\n"); // Debug
#ifndef _MSC_VER
				ts.save( MIKEY_CREATE_END );
#endif
				break;
			case KEY_MGMT_METHOD_MIKEY_PK:
				throw new MikeyExceptionUnimplemented(
						"PK KA type not implemented" );
			default:
				throw new MikeyException( "Invalid type of KA" );
		}
		
		string b64Message = message->b64Message();
		delete message;
		return "mikey "+b64Message;
	}
	catch( certificate_exception * exc ){
		// FIXME: tell the GUI
		merr << "Could not open certificate" <<end;
		securityConfig.ka_type = KEY_MGMT_METHOD_NULL;
		securityConfig.secured = false;
		return "";
	}
	catch( MikeyException * exc ){
		merr << "MikeyException caught: " << exc->message() << end;
		securityConfig.ka_type = KEY_MGMT_METHOD_NULL;
		securityConfig.secured=false;
		return "";
	}
}
			
bool Session::initiatorAuthenticate( string message ){

		
	if (message.substr(0,6) == "mikey ")
	{

		// get rid of the "mikey "
		message = message.substr(6,message.length()-6);
		if(message == ""){
			merr << "No MIKEY message received" << end;
			securityConfig.secured = false;
			return false;
		} else {
			try{
				MikeyMessage * resp_mes = new MikeyMessage( message );
				ka->setResponderData( resp_mes );

				switch( securityConfig.ka_type ){
					case KEY_MGMT_METHOD_MIKEY_DH:
						
#ifndef _MSC_VER
						ts.save( AUTH_START );
#endif
						if( resp_mes->authenticate( ((KeyAgreementDH *)*ka) ) ){
							throw new MikeyExceptionAuthentication(
							  "Authentication of the DH response message failed" );
						}
						
#ifndef _MSC_VER
						ts.save( TMP );
#endif
						if( securityConfig.check_cert ){
							if( ((KeyAgreementDH *)*ka)->controlPeerCertificate() == 0)
								throw new MikeyExceptionAuthentication(
									"Certificate control failed" );
						}
#ifndef _MSC_VER
						ts.save( AUTH_END );
#endif
						securityConfig.secured = true;
						return true;

						/*
						if( resp_mes->get_type() == MIKEY_TYPE_DH_RESP )
							((MikeyMessageDH*)resp_mes)->parse_response((KeyAgreementDH *)(key_agreement));
						else
							throw new MikeyExceptionMessageContent(
								"Unexpected MIKEY Message type" );
						
						((KeyAgreementDH *)key_agreement)->compute_tgk();*/
						
					case KEY_MGMT_METHOD_MIKEY_PSK:

#ifndef _MSC_VER
						ts.save( AUTH_START );
#endif
						if( resp_mes->authenticate( ((KeyAgreementPSK *)*ka) ) ){
							throw new MikeyExceptionAuthentication(
							"Authentication of the PSK verification message failed" );
						}
#ifndef _MSC_VER
						ts.save( AUTH_END );
#endif
					/*	
						if( resp_mes->get_type() == MIKEY_TYPE_PSK_RESP )
							((MikeyMessagePSK*)resp_mes)->parse_response((KeyAgreementPSK *)(key_agreement));
						else
							throw new MikeyExceptionMessageContent(
								"Unexpected MIKEY Message type" );
						
						break;*/
						securityConfig.secured = true;
						return true;

					case KEY_MGMT_METHOD_MIKEY_PK:
						throw new MikeyExceptionUnimplemented(
								"PK type of KA unimplemented" );
					default:
						throw new MikeyException(
								"Invalid type of KA" );
				}

				//transii->getDialog()->getPhone()->log(LOG_INFO, "Negociated the TGK: " + print_hex( key_agreement->get_tgk(), key_agreement->get_tgk_length() ) );
			}
			catch(MikeyExceptionAuthentication *exc){
				merr << "MikeyException caught: " << exc->message() << end;
				//FIXME! send SIP Authorization failed with Mikey Error message
				securityConfig.ka_type = KEY_MGMT_METHOD_NULL;
				securityConfig.secured=false;
				return false;
			}
			catch(MikeyExceptionMessageContent *exc){
				MikeyMessage * error_mes;
				merr << "MikeyExceptionMessageContent caught: " << exc->message() << end;
				if( ( error_mes = exc->errorMessage() ) != NULL ){
					//FIXME: send the error message!
				}
				securityConfig.ka_type = KEY_MGMT_METHOD_NULL;
				securityConfig.secured=false;
				return false;
			}
				
			catch(MikeyException *exc){
				merr << "MikeyException caught: " << exc->message() << end;
				securityConfig.ka_type = KEY_MGMT_METHOD_NULL;
				securityConfig.secured=false;
				return false;
			}
		}
	}
	else{
		merr << "Unknown key management method" << end;
		securityConfig.secured = false;
		return false;
	}

}

string Session::initiatorParse(){


	if( ! ( securityConfig.ka_type & KEY_MGMT_METHOD_MIKEY ) ){
		merr << "Unknown type of key agreement" << end;
		securityConfig.secured = false;
		return "";
	}
	
	MikeyMessage * responseMessage = NULL;
	
	try{
		MikeyMessage * initMessage = (MikeyMessage *)ka->responderData();

		if( initMessage == NULL ){
			merr << "Uninitialized MIKEY init message, this is a bug" << end;
			securityConfig.ka_type = KEY_MGMT_METHOD_NULL;
			securityConfig.secured = false;
			return "";
		}
			
		switch( securityConfig.ka_type ){
			case KEY_MGMT_METHOD_MIKEY_DH:
#ifndef _MSC_VER
				ts.save( MIKEY_PARSE_START );
#endif
				responseMessage = initMessage->parseResponse((KeyAgreementDH *)*ka);
#ifndef _MSC_VER
				ts.save( MIKEY_PARSE_END );
#endif
				break;
			case KEY_MGMT_METHOD_MIKEY_PSK:
#ifndef _MSC_VER
				ts.save( MIKEY_PARSE_START );
#endif
				responseMessage = initMessage->parseResponse((KeyAgreementPSK *)*ka);
#ifndef _MSC_VER
				ts.save( MIKEY_PARSE_END );
#endif

				break;
			case KEY_MGMT_METHOD_MIKEY_PK:
				/* Should not happen at that point */
				throw new MikeyExceptionUnimplemented(
						"Public Key key agreement not implemented" );
				break;
			default:
				throw new MikeyExceptionMessageContent(
						"Unexpected type of message in INVITE" );
		}
	}
	catch( certificate_exception *exc ){
		// TODO: Tell the GUI
		merr << "Could not open certificate" <<end;
		securityConfig.ka_type = KEY_MGMT_METHOD_NULL;
		securityConfig.secured = false;
		delete exc;
	}
	catch( MikeyExceptionUnacceptable *exc ){
		merr << "MikeyException caught: "<<exc->message()<<end;
		//FIXME! send SIP Unacceptable with Mikey Error message
		securityConfig.ka_type = KEY_MGMT_METHOD_NULL;
		securityConfig.secured = false;
		delete exc;
	}
	// Message was invalid
	catch( MikeyExceptionMessageContent *exc ){
		MikeyMessage * error_mes;
		merr << "MikeyExceptionMesageContent caught: " << exc->message() << end;
		if( ( error_mes = exc->errorMessage() ) != NULL ){
			responseMessage = error_mes;
		}
		securityConfig.ka_type = KEY_MGMT_METHOD_NULL;
		securityConfig.secured = false;
		delete exc;
	}
	catch( MikeyException * exc ){
		merr << "MikeyException caught: " << exc->message() << end;
		securityConfig.ka_type = KEY_MGMT_METHOD_NULL;
		securityConfig.secured = false;
		delete exc;
	}

	if( responseMessage != NULL )
		return responseMessage->b64Message();
	else
		return string("");

}

void Session::addStreamsToKa( bool initiating ){
	list< MRef<MediaStream *> >::iterator iSender;
	ka->setCsIdMapType(HDR_CS_ID_MAP_TYPE_SRTP_ID);
	uint8_t j = 1;
	for( iSender = mediaStreamSenders.begin(); 
	     iSender != mediaStreamSenders.end();
	     iSender ++, j++ ){

		if( initiating ){ 
			uint8_t policyNo = ka->setdefaultPolicy( MIKEY_PROTO_SRTP );
			ka->addSrtpStream( (*iSender)->getSsrc(), 0/*ROC*/, 
					policyNo );
			/* Placeholder for the receiver to place his SSRC */
			ka->addSrtpStream( 0, 0/*ROC*/, 
					policyNo );
		}
		else{
			ka->setSrtpStreamSsrc( (*iSender)->getSsrc(), 2*j );
			ka->setSrtpStreamRoc ( 0, 2*j );
		}

	}
}

void Session::setMikeyOffer(){
	MikeyMessage * initMessage = (MikeyMessage *)ka->initiatorData();
	switch( securityConfig.ka_type ){
		case KEY_MGMT_METHOD_MIKEY_DH:
			initMessage->setOffer((KeyAgreementDH *)*ka);
			break;
		case KEY_MGMT_METHOD_MIKEY_PSK:
			initMessage->setOffer((KeyAgreementPSK *)*ka);
			break;
		case KEY_MGMT_METHOD_MIKEY_PK:
		/* Should not happen at that point */
			throw new MikeyExceptionUnimplemented("Public Key key agreement not implemented" );
			break;
		default:
			throw new MikeyExceptionMessageContent("Unexpected type of message in INVITE" );
		}
}


#endif //NO_SECURITY
