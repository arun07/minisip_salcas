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

#include<config.h>

#include"MatroxGrabber.h"
#include<libminisip/media/video/ImageHandler.h>
#include<libminisip/media/video/VideoMedia.h>
#include<libminisip/media/video/VideoException.h>
#include<stdio.h>
#include<errno.h>
#include<libmutil/mtime.h>
#include<libmutil/stringutils.h>
#include<libmutil/merror.h>

using namespace std;

static std::list<std::string> pluginList;
static bool initialized;

extern "C" LIBMINISIP_API
std::list<std::string> *matrox_LTX_listPlugins( MRef<Library*> lib ){
	if( !initialized ){
		pluginList.push_back("getPlugin");
		initialized = true;
	}

	return &pluginList;
}

extern "C" LIBMINISIP_API
MPlugin * matrox_LTX_getPlugin( MRef<Library*> lib ){
	return new MatroxPlugin( lib );
}

#if 0
MIL_INT MFTYPE GrabStart(MIL_INT HookType, MIL_ID EventId, void MPTYPE *UserStructPtr)
{
	UserDataStruct *UserPtr=(UserDataStruct*)UserStructPtr;
	UserPtr->NbGrabStart++;

	/* Queue a new grab or stop the timer at the end. */
	cerr << "EEEE: doing grabstarts MdigGrab..."<<endl;
	MdigGrab(UserPtr->MilDigitizer, UserPtr->MilImage[ UserPtr->NbGrabStart%NB_GRAB_MAX] );
	cerr << "EEEE: done doing grab"<<endl;
	return 0; 
}
#endif

MIL_INT MFTYPE GrabEnd(MIL_INT HookType, MIL_ID EventId, void MPTYPE *UserStructPtr)
{
	UserDataStruct *UserPtr=(UserDataStruct*)UserStructPtr;

	/* Signal the end of grab event to the main thread waiting. */
	MthrControl(UserPtr->GrabEndEvent, M_EVENT_SET, M_SIGNALED);

	return 0;
}




MatroxGrabber::MatroxGrabber( string device ){
	width=height=-1;
        if (M_NULL==MappAlloc(M_DEFAULT, &UserStruct.MilApplication)){
                cerr << "ERROR: failed to allocate MIL application!"<<endl;
                return;
        }
        if (M_NULL==MsysAlloc(M_SYSTEM_VIO, M_DEV0, M_DEFAULT, &UserStruct.MilSystem)){
                cerr << "ERROR: failed to allocate MIL system!"<<endl;
                return;
        }

        //if (M_NULL==MdigAlloc(UserStruct.MilSystem, M_DEV0, MIL_TEXT("/opt/matrox_imaging/drivers/vio/dcf/standard/hd_sdi_1280x720P_60Hz.dcf"), M_DEFAULT, &UserStruct.MilDigitizer)){
        if (M_NULL==MdigAlloc(UserStruct.MilSystem, M_DEV0, MIL_TEXT("/opt/matrox_imaging/drivers/vio/dcf/standard/hd_sdi_1920x1080i_30Hz.dcf"), M_DEFAULT, &UserStruct.MilDigitizer)){
    //    if (M_NULL==MdigAlloc(UserStruct.MilSystem, M_DEV0, MIL_TEXT("/opt/matrox_imaging/drivers/vio/dcf/standard/hd_rgb_1280x720p_50Hz.dcf"), M_DEFAULT, &UserStruct.MilDigitizer)){
	//if (M_NULL==MdigAlloc(UserStruct.MilSystem, M_DEV0, MIL_TEXT("/opt/matrox_imaging/drivers/vio/dcf/standard/ntsc.dcf"), M_DEFAULT, &UserStruct.MilDigitizer)){
                cerr << "ERROR: could not allocate digitizer"<<endl;
                return;
        }

	/* Allocate and clear sequence and display images. */
	MappControl(M_ERROR, M_PRINT_DISABLE);
	for (UserStruct.NbFrames=0; UserStruct.NbFrames<NB_GRAB_MAX+2; UserStruct.NbFrames++)
	{

		MbufAllocColor(UserStruct.MilSystem, 
					MdigInquire(UserStruct.MilDigitizer, M_SIZE_BAND, M_NULL), 
					MdigInquire(UserStruct.MilDigitizer, M_SIZE_X, M_NULL), 
					MdigInquire(UserStruct.MilDigitizer, M_SIZE_Y, M_NULL), 
					8L+M_UNSIGNED, 
					M_IMAGE + M_GRAB + M_NON_PAGED + M_BGR32 + M_HOST_MEMORY, 
					&MilImage[UserStruct.NbFrames]); //Allocate grab buffer with three bands for RGB24 capture

		if (MilImage[UserStruct.NbFrames])
		{
			MbufClear(MilImage[UserStruct.NbFrames], 0xFF);
		}
		else{
			cerr << "EEEE: ERROR: could not allocate image buffer"<<endl;
			break;
		}	
	}


	MappControl(M_ERROR, M_PRINT_ENABLE);

	/* Free buffers to leave space for possible temporary buffers. */
	int n;
	for (n=0; n<2 && UserStruct.NbFrames; n++)
	{
		UserStruct.NbFrames--;
		MbufFree(MilImage[UserStruct.NbFrames]);
	}

	/* MIL event allocation for grab end hook. */
	MthrAlloc(UserStruct.MilSystem, M_EVENT, M_DEFAULT, M_NULL, M_NULL, &UserStruct.GrabEndEvent);


}

void MatroxGrabber::open(){

	/* Initialize hook structure. */
	UserStruct.MilImage     = MilImage;
	UserStruct.NbGrabStart  = 0;

//	MdigHookFunction(UserStruct.MilDigitizer, M_GRAB_START, GrabStart, (void *)(&UserStruct));
	MdigHookFunction(UserStruct.MilDigitizer, M_GRAB_END,   GrabEnd,   (void *)(&UserStruct));

}

void MatroxGrabber::getCapabilities(){
}

void MatroxGrabber::printCapabilities(){

}

void MatroxGrabber::getImageFormat(){
	width = MdigInquire(UserStruct.MilDigitizer, M_SIZE_X, M_NULL);
	height= MdigInquire(UserStruct.MilDigitizer, M_SIZE_Y, M_NULL);
	cerr << "MatroxGrabber: Info: grabber dimensions are "<< width<<"x"<<height<<endl;
}

bool MatroxGrabber::setImageChroma( uint32_t chroma ){
	cerr << "Warning: ignoring setImageChroma request for chroma "<< chroma <<endl;
	return true;
}

void MatroxGrabber::printImageFormat(){
}

void MatroxGrabber::stop(){
	UserStruct.stopped = true;
}

/*loop that reads from the card (and calls handler->handle()) until stop() is called*/
void MatroxGrabber::run(){
	UserStruct.stopped = false;
	read( handler );
}

void MatroxGrabber::setHandler( ImageHandler * handler ){
	grabberLock.lock();
	this->handler = handler;
	grabberLock.unlock();
}

void MatroxGrabber::read( ImageHandler * handler ){

	/* Put digitizer in asynchronous mode. */
	MdigControl(UserStruct.MilDigitizer, M_GRAB_MODE, M_ASYNCHRONOUS);
	/* Start sequence with a grab in the first buffer. */
	MdigGrab(UserStruct.MilDigitizer, UserStruct.MilImage[0]);

	MImage *frame = new MImage;

	frame->data[0] = new uint8_t[ width * height * 4 ];
	frame->data[1] = NULL;
	frame->data[2] = NULL;
	frame->linesize[0] = width*4;
	frame->linesize[1] = 0;
	frame->linesize[2] = 0;
	frame->chroma = M_CHROMA_RV32;


	while (!UserStruct.stopped)
	{
		/* Wait end of grab event */
		MthrWait(UserStruct.GrabEndEvent, M_EVENT_WAIT, M_NULL);
		
		int frameindex = UserStruct.NbGrabStart;

		UserStruct.NbGrabStart = (UserStruct.NbGrabStart+1) % NB_GRAB_MAX;

		MdigGrab(UserStruct.MilDigitizer, UserStruct.MilImage[ UserStruct.NbGrabStart] );

		MIL_INT line_delta=-33;
		uint8_t *rgb32ptr;

		MbufInquire( UserStruct.MilImage[ frameindex ], M_PITCH_BYTE, &line_delta ); //FIXME: copy from correct index
		MbufInquire(UserStruct.MilImage[ frameindex ], M_HOST_ADDRESS, &rgb32ptr);
		memcpy(frame->data[0], rgb32ptr, width * height * 4 );
		frame->mTime = mtime();
		handler->handle( frame );
	}

	/* Wait for end of last grab. */
	MdigGrabWait(UserStruct.MilDigitizer, M_GRAB_END);


	/* Unhook functions. */
//	MdigHookFunction(UserStruct.MilDigitizer, M_GRAB_START+M_UNHOOK, GrabStart, (void *)(&UserStruct));
	MdigHookFunction(UserStruct.MilDigitizer, M_GRAB_END+M_UNHOOK, GrabEnd, (void *)(&UserStruct));

	/* Free event. */
	MthrFree(UserStruct.GrabEndEvent);

	/* Free allocations. */
	int n;
	for (n=0; n<UserStruct.NbFrames; n++)
	{
		MbufFree(UserStruct.MilImage[n]);
	}
	//XXX: TODO: free resources
//	MappFreeDefault(UserStruct.MilApplication, MilSystem, MilDisplay, UserStruct.MilDigitizer, MilImageDisp);

}

void MatroxGrabber::close(){
	stop();
}


MRef<Grabber *> MatroxPlugin::create( const std::string &device ) const{
	return new MatroxGrabber( device );
}
