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



#include<libminisip/Minisip.h>

#include<config.h>

#include<libmsip/SipCallback.h>
#include<libmsip/SipCommandString.h>
#include<libmnetutil/NetworkFunctions.h>
#include<libmnetutil/NetUtil.h>
#include<libmnetutil/NetworkException.h>
#include<libmikey/keyagreement_dh.h>
#include<libminisip/Sip.h>
#include<libminisip/LogEntry.h>
#include<libminisip/ContactDb.h>
#include<libminisip/MediaHandler.h>
#include<libminisip/ConferenceControl.h>
#include<libminisip/ConfCallback.h>
#include<libminisip/MessageRouter.h>

#include<libmsip/SipUtils.h>
#include<exception>
#include<libmutil/Timestamp.h>
#include<libmutil/termmanip.h>
#include<stdlib.h>

#ifndef WIN32
#ifdef DEBUG_OUTPUT
#include<signal.h>
#include<libmutil/Timestamp.h>
#endif
#endif

#ifndef WIN32
#ifdef DEBUG_OUTPUT
static void signal_handler( int signal ){
        if( signal == SIGUSR1 ){
                merr << "Minisip was stopped" << end;
                ts.print();
        }
	exit( 1 );
}
#endif
#endif


Minisip::Minisip(MRef<Gui*> g, string configFile )
	: gui(g)
{
	if (!g){
		cerr << "Minisip::Minisip: Invalid argument: Gui is NULL"<< endl;
		::exit(1);
	}

	if( configFile.size()>0){
		conffile = configFile;
	}
	else{
		char *home = getenv("HOME");
		if (home==NULL){
			merr << "WARNING: Could not determine home directory"<<end;

#ifdef WIN32
                        conffile=string("c:\\minisip.conf"); 
#else
                        conffile = string("/.minisip.conf");
#endif
                }else{
                        conffile = string(home)+ string("/.minisip.conf");
                }
        }

        srand(time(0));

#ifndef WIN32
#ifdef DEBUG_OUTPUT
	signal( SIGUSR1, signal_handler );
#endif
#endif
	

	mout << "Initializing NetUtil"<<end;

        if ( ! NetUtil::init()){
                printf("Could not initialize Netutil package\n");
                merr << "ERROR: Could not initialize NetUtil package"<<end;
                exit();
        }
	cerr << "Creating SipSoftPhoneConfiguration..."<< endl;
	phoneConf =  new SipSoftPhoneConfiguration();


	phoneConf->sip=NULL;

#ifdef MINISIP_AUTOCALL
	if (argc==3){
		phoneConf->autoCall = string(argv[2]);
	}
#endif

#ifdef DEBUG_OUTPUT
	mout << BOLD << "init 1/9: Creating ContactDb" << PLAIN << end;
#endif

        /* Create the global contacts database */
        ContactDb *contactDb = new ContactDb();
        ContactEntry::setDb(contactDb);
	
	//FIXME: move all this in a Gui::create()

#ifdef DEBUG_OUTPUT
        mout << BOLD << "init 2/9: Setting contact db" << PLAIN << end;
#endif

	if (gui){
		cerr << "Setting contact db"<< endl;
		gui->setContactDb(contactDb);
	}
}

Minisip::~Minisip(){

}

/*
void Minisip::setGui(MRef<Gui *> gui){

}
*/

void Minisip::exit(){
	//TODO
	mout << BOLD << "CESC: Minisip::exit()!!!" << PLAIN << end;
	/* End on-going calls the best we can */

	/* Unregister */

	/* End threads */

	/* delete things */
}

void Minisip::run(){
	cerr << "Thread 2 running - doing initParseConfig"<< endl;
	initParseConfig();
	cerr << "Creating MessageRouter"<< endl;

	try{
		MessageRouter *messageRouter=  new MessageRouter();
//		phoneConf->timeoutProvider = timeoutprovider;

#ifdef DEBUG_OUTPUT
                mout << BOLD << "init 4/9: Creating IP provider" << PLAIN << end;
#endif
                MRef<IpProvider *> ipProvider =
                        IpProvider::create( phoneConf, *gui ); //TODO: remove second argument - not needed
//#ifdef DEBUG_OUTPUT
//                mout << BOLD << "init 5/9: Creating SIP transport layer" << PLAIN << end;
//#endif
                string localIpString;
                string externalContactIP;

                // FIXME: This should be done more often
                localIpString = externalContactIP = ipProvider->getExternalIp();                
		
		MRef<UDPSocket*> udpSocket = new UDPSocket( false, phoneConf->inherited->localUdpPort );                
		
		phoneConf->inherited->localUdpPort =
                        ipProvider->getExternalPort( udpSocket );
                phoneConf->inherited->localIpString = externalContactIP;
                phoneConf->inherited->externalContactIP = externalContactIP;
                udpSocket=NULL;

#ifdef DEBUG_OUTPUT
                mout << BOLD << "init 5/9: Creating MediaHandler" << PLAIN << end;
#endif
                MRef<MediaHandler *> mediaHandler = new MediaHandler( phoneConf, ipProvider );
		messageRouter->setMediaHandler( mediaHandler );
                Session::registry = *mediaHandler;
                /* Hack: precompute a KeyAgreementDH */
                Session::precomputedKa = new KeyAgreementDH( phoneConf->securityConfig.cert, phoneConf->securityConfig.cert_db, DH_GROUP_OAKLEY5 );

#ifdef DEBUG_OUTPUT
                mout << BOLD << "init 6/9: Creating MSip SIP stack" << PLAIN << end;
#endif
		MRef<Sip*> sip=new Sip(phoneConf,mediaHandler,
					localIpString,
					externalContactIP,
					phoneConf->inherited->localUdpPort,
					phoneConf->inherited->localTcpPort,
					phoneConf->inherited->externalContactUdpPort,
					phoneConf->inherited->transport,
					phoneConf->inherited->localTlsPort,
					phoneConf->securityConfig.cert,    //The certificate chain is used by TLS
					//TODO: TLS should use the whole chain instead of only the f$                                MRef<ca_db *> cert_db = NULL
					phoneConf->securityConfig.cert_db
					);
		//sip->init();

                phoneConf->sip = sip;

                sip->getSipStack()->setCallback(messageRouter);

                messageRouter->setSip(sip);

#ifdef DEBUG_OUTPUT
                mout << BOLD << "init 7/9: Connecting GUI to SIP logic" << PLAIN << end;
#endif
                gui->setSipSoftPhoneConfiguration(phoneConf);
                messageRouter->setGui(gui);

//                Thread t(*sip);

		try{
                if (phoneConf->tcp_server){
#ifdef DEBUG_OUTPUT
                        mout << BOLD << "init 8.2/9: Starting TCP transport worker thread" << PLAIN << end;
#endif
			
			sip->getSipStack()->getSipTransportLayer()->startTcpServer();
//                        Thread::createThread(tcp_server_thread, *(/*phoneConf->inherited.sipTransport*/ sip->getSipStack()->getSipTransportLayer() ));

                }

                if (phoneConf->tls_server){
                        if( phoneConf->securityConfig.cert.isNull() ){
                                merr << "Certificate needed for TLS server" << end;
                        }
                        else{
#ifdef DEBUG_OUTPUT
                                mout << BOLD << "init 8.3/9: Starting TLS transport worker thread" << PLAIN << end;
#endif
				sip->getSipStack()->getSipTransportLayer()->startTlsServer();
//                                Thread::createThread(tls_server_thread, *(/*phoneConf->inherited.sipTransport*/ sip->getSipStack()->getSipTransportLayer()));
                        }
                }
		}
		catch( NetworkException * exc ){
			cerr << "Exception thrown when creating TCP/TLS servers." << endl;
			cerr << exc->errorDescription() << endl;
			delete exc;
		}

#ifdef DEBUG_OUTPUT
                mout << BOLD << "init 9/9: Initiating register to proxy commands (if any)" << PLAIN << end;
#endif

                for (list<MRef<SipIdentity*> >::iterator i=phoneConf->identities.begin() ; i!=phoneConf->identities.end(); i++){
                        if ( (*i)->registerToProxy  ){
//                              cerr << "Registering user "<< (*i)->getSipUri() << " to proxy " << (*i)->sipProxy.sipProxyAddressString<< ", requesting domain " << (*i)->sipDomain << endl;
                                CommandString reg("",SipCommandString::proxy_register);
                                reg["proxy_domain"] = (*i)->sipDomain;
                                SipSMCommand sipcmd(reg, SipSMCommand::remote, SipSMCommand::TU);
                                sip->getSipStack()->handleCommand(sipcmd);

                        }
                }
/*
		mdbg << "Starting presence server"<< end;
		CommandString subscribeserver("", SipCommandString::start_presence_server);
		SipSMCommand sipcmdss(subscribeserver, SipSMCommand::remote, SipSMCommand::TU);
		sip->handleCommand(sipcmdss);
*/

/*
		cerr << "Minisip: starting presence client for johan@bilien.org"<< endl;
		
		CommandString subscribe("", SipCommandString::start_presence_client,"johan@bilien.org");
		SipSMCommand sipcmd2(subscribe, SipSMCommand::remote, SipSMCommand::TU);
		sip->getSipStack()->handleCommand(sipcmd2);
*/
		
                gui->setCallback(messageRouter);
//		sleep(5);
		
//		CommandString pupd("", SipCommandString::remote_presence_update,"someone@ssvl.kth.se","online","Working hard");
//		gui->handleCommand(pupd);

#ifdef TEXT_UI
		MinisipTextUI * textui = dynamic_cast<MinisipTextUI *>(gui);
		if (textui){
			textui->displayMessage("");
			textui->displayMessage("To auto-complete, press <tab>. For a list of commands, press <tab>.", MinisipTextUI::bold);
		textui->displayMessage("");
		}
#endif
                sip->run();

        }catch(XMLElementNotFound *e){
                //FIXME: Display message in GUI
#ifdef DEBUG_OUTPUT
                merr << "Error: The following element could not be parsed: "<<e->what()<< "(corrupt config file?)"<< end;
#endif
	}
        catch(exception &exc){
                //FIXME: Display message in GUI
#ifdef DEBUG_OUTPUT
                merr << "Minisip caught an exception. Quitting."<< end;
		merr << exc.what() << end;
#endif
        }
        catch(...){
                //FIXME: Display message in GUI
#ifdef DEBUG_OUTPUT
                merr << "Minisip caught an unknown exception. Quitting."<< end;
#endif

        };

}

void Minisip::initParseConfig(){

        bool done=false;
        do{
                try{
#ifdef DEBUG_OUTPUT
                        mout << BOLD << "init 3/9: Parsing configuration file ("<< conffile<<")" << PLAIN << end;
#endif
                        string ret = phoneConf->load( conffile );

                        cerr << "Identities: "<<endl;
                        for (list<MRef<SipIdentity*> >::iterator i=phoneConf->identities.begin() ; i!=phoneConf->identities.end(); i++){
                                cerr<< "\t"<< (*i)->getDebugString()<< endl;
                        }

                        if (ret.length()>0){
                                //bool ok;
                                merr << ret << end;
				/*
                                ok = gui->configDialog( phoneConf );
                                if( !ok ){
                                        exit();
                                }
                                done=false;
				*/
				done=true;
                        }else
                                done=true;
                }catch(XMLElementNotFound *enf){
#ifdef DEBUG_OUTPUT
                        merr << FG_ERROR << "Element not found: "<< enf->what()<< PLAIN << end;
#endif
                        merr << "Could not parse configuration item: "+enf->what() << end;
                        gui->configDialog( phoneConf );
                        done=false;
                }
        }while(!done);
}

void Minisip::startSip(){
	//cerr << "Creating thread"<< endl;
	Thread( this );
}


