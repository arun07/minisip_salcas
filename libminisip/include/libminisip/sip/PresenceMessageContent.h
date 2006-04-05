#ifndef _PRESENCEMESSAGECONTENT_H
#define _PRESENCEMESSAGECONTENT_H

#include<libminisip/libminisip_config.h>

#include<libmutil/MemObject.h>
#include<libmsip/SipMessageContent.h>
#include<libmsip/SipMessageContentFactory.h>

#include<string>


MRef<SipMessageContent*> presenceSipMessageContentFactory(const std::string &, const std::string &ContentType);

class LIBMINISIP_API PresenceMessageContent : public SipMessageContent{
	public:
		PresenceMessageContent(std::string from, std::string to, std::string onlineStatus, std::string onlineStatusDesc);
		PresenceMessageContent(const std::string &buildFrom);
		virtual std::string getMemObjectType(){return "PresenceMessageContent";}
		virtual std::string getString();
		virtual std::string getContentType(){return "application/xpidf+xml";}

	private:
		std::string fromUri;
		std::string toUri;
		std::string document;
		std::string onlineStatus;
		std::string onlineStatusDesc;
		std::string presentity;
};


#endif
