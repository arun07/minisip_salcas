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

/* Copyright (C) 2004, 2006
 *
 * Authors: Erik Eliasson <eliasson@it.kth.se>
 *          Johan Bilien <jobi@via.ecp.fr>
 *          Mikael Magnusson <mikma@users.sourceforge.net> 
*/

#include"OpenGLDisplay.h"
#include<config.h>
#include<sys/time.h>
#include<SDL/SDL_syswm.h>

#include<iostream>

using namespace std;

#define NB_IMAGES 3


static std::list<std::string> pluginList;
static MRef<MPlugin *> plugin;
static bool initialized;


extern "C" LIBMINISIP_API
std::list<std::string> *mopengl_LTX_listPlugins( MRef<Library*> lib ){
	if( !initialized ){
		pluginList.push_back("getPlugin");
		initialized = true;
	}

	return &pluginList;
}

extern "C" LIBMINISIP_API
MPlugin * mopengl_LTX_getPlugin( MRef<Library*> lib ){
	return new OpenGLPlugin( lib );
}

/**
 * A single OpenGl widow is used to display all video data. It displays
 * video from multiple OpenGLDisplay objects.
 */



class OpenGLWindow : public Runnable {
	public:
		OpenGLWindow(int width, int height, bool fullscreen);

		void init();

		void start();
		void stop();
		virtual void run();

		void display(MImage* image, uint64_t source);
		
		string getMemObjectType(){return "OpenGLWindow";}

		static MRef<OpenGLWindow*> getWindow();
	private:
		void drawSurface();
		void sdlQuit();
		void windowResized(int w, int h);
		bool isFullscreen();
		void toggleFullscreen();
		

		//Counts how many times displays have called "start()"
		int runCount;
		MRef<Semaphore*> startWaitSem;

		static MRef<OpenGLWindow*> globalWindowObj;
		void initSdl();
		void initSurface();

		bool initialized;

		SDL_Surface* gDrawSurface;
		bool startFullscreen;


		int windowed_width;
		int windowed_height;
		int native_width;
		int native_height;
		int cur_width;
		int cur_height;

		int sdlFlags;
		int bpp;


		bool doStop;
		Mutex lock;
		MRef<Thread*> thread;

		Mutex queueLock;
		//Semaphore queueSem;
		std::list<MImage *> displayQueue;

		GLuint texture;
};

MRef<OpenGLWindow*> OpenGLWindow::globalWindowObj=NULL;

OpenGLWindow::OpenGLWindow(int w, int h, bool fullscreen){
	texture=-1;
	runCount=0;
	doStop=false;
	initialized=false;
	windowed_width=w;
	windowed_height=h;
	native_width=0;
	native_height=0;
	startFullscreen=fullscreen;

	gDrawSurface=NULL;

	bpp=0;

}

void OpenGLWindow::display(MImage* image, uint64_t sourceid){
	queueLock.lock();
	displayQueue.push_back(image);


	queueLock.unlock();

}

#define REPORT_N 500

#define SCALE (10.0/512.0)


void OpenGLWindow::drawSurface(){
	cerr << "EEEE: doing OpenGLWindow::drawSurface()"<<endl;
        static struct timeval lasttime;
        static int i=0;
        i++;
        if (i%REPORT_N==1){
                struct timeval now;
                gettimeofday(&now, NULL);
                int diffms = (now.tv_sec-lasttime.tv_sec)*1000+(now.tv_usec-lasttime.tv_usec)/1000;
                float sec = (float)diffms/1000.0f;
                printf("%d frames in %fs\n", REPORT_N, sec);
                printf("FPS: %f\n", (float)REPORT_N/(float)sec );
                lasttime=now;
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ); //EE: not use depth?

///        if (!d_login->hidden())
///                d_login->draw();

//      SDL_Color green = {0,255,0};
//        SDL_Color black= {0,0,0};

//      text->draw2D(100,100,"Hello world", 10, green);
        //text->draw2D(100,100,"Hello world", 30,green, black);
	

	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();

	glEnable( GL_TEXTURE_2D );

	glPushMatrix();

	glTranslatef( 0, 0.0f, -20);



	queueLock.lock();
	if (displayQueue.size()>0 && texture==-1){
		cerr <<"EEEE: UPLOADING TEXTURE TO MEMORY"<<endl;
		MImage* image = *displayQueue.begin();
		glGenTextures( 1, &texture);
		cerr << "EEEE: generated texture with id "<< texture << endl;
		glBindTexture( GL_TEXTURE_2D, texture);
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		cerr << "EEEE: texture dimensions: "<< image->width<<"x"<< image->height<<endl;

//		for (int i=0; i<256*256; i++)
//			image->data[0][i]=rand()%255;
		glTexImage2D( GL_TEXTURE_2D, 0, 3, /*image->width*/256, /*image->height*/256, 0,
				GL_BGR, GL_UNSIGNED_BYTE, image->data[0] );
		
		displayQueue.pop_front();

	}else{
		
	}


	
	queueLock.unlock(); // unlock before uploading...

	if (texture>0){
		cerr << "EEEE: ++++++++++++++ drawing texture +++++++++++++"<<endl;
		glColor4f(1.0,1.0,1.0, 1.0 );
		glBindTexture( GL_TEXTURE_2D, texture);
		glBegin( GL_QUADS );

		glTexCoord2i( 0, 1 );
		glVertex3f( /*-5.0*/ -(512.0/2.0)*SCALE, -5.0, 0.0f );

		glTexCoord2i( 1, 1 );
		glVertex3f( 5.0, -5.0, 0.0f );

		glTexCoord2i( 1, 0 );
		glVertex3f( 5.0, 5.0, 0.0f );

		glTexCoord2i( 0, 0 );
		glVertex3f( -5.0, 5.0, 0.0f );
		glEnd();
	}


	glPopMatrix();


	glFlush();
	SDL_GL_SwapBuffers();
}


void OpenGLWindow::sdlQuit(){
        cerr << "EEEE: sdlQuit called"<<endl;
        if (isFullscreen() ){
                toggleFullscreen();
        }
        SDL_Quit();
}



void OpenGLWindow::toggleFullscreen(){
        printf("Toggle fullscreent\n");
        sdlFlags ^= SDL_FULLSCREEN;
        //SDL_WM_ToggleFullScreen(gDrawSurface);
        initSurface();

}


bool OpenGLWindow::isFullscreen(){
        if (sdlFlags&SDL_FULLSCREEN)
                return true;
        else
                return false; 
}



void OpenGLWindow::windowResized(int w, int h){
        windowed_width=w;
        windowed_height=h;
#if 0
        SDL_SetVideoMode(w, h, 16, SDL_OPENGL | SDL_RESIZABLE);
        glViewport(0, 0, w, h);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluPerspective(45,(double)w/(double)h,1.0,200.0);
#endif
        initSurface();

/*      glOrtho(0, 1, 1, 0, -1, 1);

        glClear(GL_COLOR_BUFFER_BIT);
        glColor3f(1.0, 1.0, 1.0);
        glBegin(GL_LINES);
        glVertex2i(0, 0); glVertex2i(1, 1);
        glVertex2i(1, 0); glVertex2i(0, 1);
        glEnd();
*/
        SDL_GL_SwapBuffers();
}


void OpenGLWindow::run(){
	cerr << "EEEE: OpenGLWindow::run start"<<endl;
#ifdef DEBUG_OUTPUT
	setThreadName("OpenGLWindow");
#endif

	if (!initialized)
		init();

	startWaitSem->inc();

	SDL_Event event;
	while(!doStop)
	{
		cerr << "EEEE: OpenGLWindow::run loop"<<endl;
		while( SDL_PollEvent( &event ))
		{
			string url;
			switch( event.type )
			{
			case SDL_MOUSEBUTTONDOWN:
				printf("PRESS: %d,%d\n",event.button.x, event.button.y);
#if 0
				url=d_login->clickedUrl(event.button.x, event.button.y);
				if (url!=""){
					gui->doRegister(url);
					d_login->hide();
				}
#endif

				break;
			case SDL_MOUSEMOTION:
				//printf("mouse %d,%d\n",event.motion.x, event.motion.y);
				//			d_login->mouseMove(event.motion.x, event.motion.y);
				break;

			case SDL_QUIT:
				sdlQuit();
				return;
			case SDL_VIDEORESIZE:
				printf("EE: RESIZE!\n");
				windowResized(event.resize.w, event.resize.h);
				break;
			case SDL_KEYDOWN:

				//trap quit and fullscreen events. Forward everything else

				if (event.key.keysym.sym == SDLK_ESCAPE ||
						event.key.keysym.sym == SDLK_q){
					sdlQuit();
					return;
				}


				if (event.key.keysym.sym == SDLK_RETURN &&
						event.key.keysym.mod & KMOD_ALT){
					toggleFullscreen();
				}else{
					char key = event.key.keysym.sym;
					if (event.key.keysym.mod == KMOD_SHIFT){        //if shift, make upper case
						printf("Shift detected\n");
						key-='a'-'A';
					}
					//gui->keyPressed(key);

				}

				break;
			}

		} // -- while event in queue
		drawSurface();
//		Thread::msleep(500);
	}

	cerr <<"----------------EEEE: OpenGl thread quitting"<<endl;
} 


void OpenGLWindow::start(){
	cerr << "EEEE: doing OpenGLWindow::start()"<<endl;
	lock.lock();
	startWaitSem = new Semaphore();
	doStop=false;
	if (runCount==0){
		massert(!thread);
		thread = new Thread(this);
	}
	runCount++;

	lock.unlock();
	startWaitSem->dec();

	cerr << "EEEE: after OpenGLWindow::start() runCount="<<runCount<<endl;
}

void OpenGLWindow::stop(){
	cerr << "EEEE: doing OpenGLWindow::stop()"<<endl;
	lock.lock();
	runCount--;
	if (runCount==0){
		doStop=true;
		thread->join();
		thread=NULL;
	}
	lock.unlock();

	cerr << "EEEE: after OpenGLWindow::stop() runCount="<<runCount<<endl;
}

MRef<OpenGLWindow*> OpenGLWindow::getWindow(){
	if (!globalWindowObj)
		globalWindowObj = new OpenGLWindow(800,  600, false);
	return globalWindowObj;

}

void OpenGLWindow::init(){
	if (!initialized){
		initialized=true;
		initSdl();
	}

}

void OpenGLWindow::initSdl(){
	// init video system
	if( SDL_Init(SDL_INIT_VIDEO) < 0 )
	{
		fprintf(stderr,"Failed to initialize SDL Video!\n");
		exit(1);
	}

#if 0
	if(TTF_Init()){
		fprintf(stderr,"Failed to initialize SDL_TTF!\n");
		exit(1);
	}

	text = new Text("share/FreeSans.ttf");
#endif


	// tell system which funciton to process when exit() call is made
	atexit(SDL_Quit);

	// get optimal video settings
	const SDL_VideoInfo* vidinfo = SDL_GetVideoInfo();
	if(!vidinfo)
	{
		fprintf(stderr,"Coudn't get video information!\n%s\n", SDL_GetError());
		exit(1);
	}
	native_width=vidinfo->current_w;
	native_height=vidinfo->current_h;

	// set opengl attributes
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE,        5);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,      5);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,       5);
#ifdef __APPLE__
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,      32);
#else
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,      16);
#endif

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER,    1);

	SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL,    1);


	sdlFlags = SDL_OPENGL | SDL_RESIZABLE;

	if (startFullscreen)
		sdlFlags = sdlFlags | SDL_FULLSCREEN;

	bpp = vidinfo->vfmt->BitsPerPixel; //attribute used by initSurface

	initSurface( );
}

void OpenGLWindow::initSurface(){

	printf("initSurface running\n");

	// get a framebuffer
	int w,h;
	if (sdlFlags & SDL_FULLSCREEN){
		w=native_width;
		h=native_height;
		cerr << "EEEE: initializing to fullscreen dimensions "<< w <<"x"<<h<<endl;
	}else{
		w=windowed_width;
		h=windowed_height;
		cerr << "EEEE: initializing to windowed dimensions "<< w <<"x"<<h<<endl;
	}
	cur_width=w;
	cur_height=h;

	cerr << "EEEE: setting video mode to " << w << "x" << h << endl;
	gDrawSurface = SDL_SetVideoMode(w,h, bpp, sdlFlags);

	if( !gDrawSurface )
	{
		fprintf(stderr,"Couldn't set video mode!\n%s\n", SDL_GetError());
		exit(1);
	}

	GLint texSize;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &texSize);
	printf("INFO: max texture size: %dx%d\n",texSize,texSize);


	glShadeModel(GL_SMOOTH);
	glClearColor(0x04/255.0F,0x01/255.0F,0x16/255.0F,0);
	// set opengl viewport and perspective view
	glViewport(0,0,cur_width,cur_height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0, (GLfloat) cur_width/(GLfloat) cur_height, 1.0, 200.0);

	//glMatrixMode(GL_MODELVIEW);
	//glLoadIdentity();

	//        loadTexture();

	///	massert(d_login);
	///	d_login->initGl(text);

	///	if (text)
	///		text->restartGl();


	SDL_GL_SetAttribute(SDL_GL_RED_SIZE,        5);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,      5);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,       5);
#ifdef __APPLE__
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,      32);
#else
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,      16);
#endif

	SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL,    1);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER,    1);



	glDisable(GL_DEPTH_TEST);

}



//////////////////////////////////////////////////////////////////////////////////////



OpenGLDisplay::OpenGLDisplay( uint32_t width, uint32_t height):VideoDisplay(){
	cerr << "EEEE: OpenGLDisplay::OpenGLDisplay("<< width<<","<<height<<") running"<<endl;
	this->width = width;
	this->height = height;
	fullscreen = false;
#if 0
	baseWindowWidth = this->width;
	baseWindowHeight = this->height;
#endif
	window = * (OpenGLWindow::getWindow() );
	massert(window);
}

void OpenGLDisplay::start(){
	cerr <<"EEEE: doing OpenGLDisplay::start"<<endl;
	massert(window);
	window->start();
}

void OpenGLDisplay::stop(){
	cerr <<"EEEE: doing OpenGLDisplay::stop"<<endl;
	window->stop();
}

void OpenGLDisplay::openDisplay(){
	cerr <<"EEEE: doing OpenGLDisplay::openDisplay"<<endl;

}

void OpenGLDisplay::init( uint32_t width, uint32_t height ){
	cerr <<"EEEE: doing OpenGLDisplay::init("<<width<<","<<height<<")"<<endl;
	this->width = width;
	this->height=height;
	//	window->init();
#if 0
	baseWindowWidth = this->width = width;
	baseWindowHeight = this->height = height;
#endif
}

void OpenGLDisplay::createWindow(){
	cerr <<"EEEE: doing OpenGLDisplay::createWindow"<<endl;
#if 0	
	if( OpenGL_Init( OpenGL_INIT_VIDEO | OpenGL_INIT_EVENTTHREAD |OpenGL_INIT_NOPARACHUTE) < 0 ){
		fprintf( stderr, "Could not initialize OpenGL: %s\n",
				OpenGL_GetError() );
		exit( 1 );
	}

	//        uint32_t flags;
	//        int bpp;

	flags = OpenGL_ANYFORMAT | OpenGL_HWPALETTE | OpenGL_HWSURFACE |
		OpenGL_DOUBLEBUF | OpenGL_RESIZABLE;

	bpp = OpenGL_VideoModeOK( baseWindowWidth, baseWindowHeight, 16, flags );

	if( bpp == 0 ){
		fprintf( stderr, "Could not find an OpenGL video mode\n" );
		exit( 1 );
	}

	surface = OpenGL_SetVideoMode( baseWindowWidth, baseWindowHeight, bpp, flags );

	if( surface == NULL ){
		fprintf( stderr, "Could not set OpenGL video mode\n" );
		exit( 1 );
	}

	initWm();


	OpenGL_WM_SetCaption( "minisip video", "minisip video" );


	OpenGL_LockSurface( surface );
#endif
}

#if 0
void OpenGLDisplay::initWm(){
#ifdef IPAQ
	if( !fullscreen ){
		int ret;
		OpenGL_SysWMinfo wmInfo;

		OpenGL_VERSION( &wmInfo.version );
		ret = OpenGL_GetWMInfo( &wmInfo );

		if( ret > 0 ){
			XClientMessageEvent event;
			display = wmInfo.info.x11.display;
			window = wmInfo.info.x11.wmwindow;
			Atom type = XInternAtom( display, "_NET_WM_WINDOW_TYPE", False );
			Atom typeDialog = XInternAtom(display, "_NET_WM_WINDOW_TYPE_DIALOG", False );

			XChangeProperty( display, window, type,
					XA_ATOM, 32, PropModeReplace,
					(unsigned char *) &typeDialog, 1);



			XUnmapWindow( display, window );
			XResizeWindow( display, window, width, height );
			XMapWindow( display, window );
		}
		else{
			fprintf( stderr, "OpenGL Error when getting WM info: %s\n",
					OpenGL_GetError() );
		}
	}
#endif
}
#endif

void OpenGLDisplay::resize(int w, int h){
	cerr << "EEEE: doing OpenGLDisplay::resize("<<w<<","<<h<<")"<<endl;
	this->width=w;
	this->height=h;
//	stop();
//	init(w,h);
//	start();

}

void OpenGLDisplay::destroyWindow(){
	cerr << "EEEE: doing OpenGLDisplay::destroyWindow()"<<endl;
#if 0
	OpenGL_UnlockSurface( surface );

	OpenGL_FreeSurface( surface );

	OpenGL_QuitSubSystem( OpenGL_INIT_VIDEO );
#endif
}

MImage * OpenGLDisplay::provideImage(){
	cerr <<"EEEE: WARNING: INTENTIONAL MEMORY LEAK"<<endl;
	return allocateImage();
}

MImage * OpenGLDisplay::allocateImage(){
	cerr << "EEEE: doing OpenGLDisplay::allocateImage"<<endl;
	MImage * mimage = new MImage;

	mimage->data[0] = /*overlay->pixels[0]*/ (uint8_t*)calloc(1,width*height+1);
	mimage->data[1] = /*overlay->pixels[2]*/ (uint8_t*)calloc(1,width*height+1);
	mimage->data[2] = /*overlay->pixels[1]*/ (uint8_t*)calloc(1,width*height+1);

	mimage->linesize[0] =/* overlay->pitches[0]*/ width;
	mimage->linesize[1] =/* overlay->pitches[2]*/ width;
	mimage->linesize[2] =/* overlay->pitches[1]*/ width;
	mimage->width=width;
	mimage->height=height;

	mimage->privateData = NULL;

	return mimage;
}

void OpenGLDisplay::deallocateImage( MImage * mimage ){
	cerr << "EEEE: doing OpenGLDisplay::deallocateImage()"<<endl;
	free(mimage->data[0]);
	free(mimage->data[1]);
	free(mimage->data[2]);
	delete mimage;
}

bool OpenGLDisplay::handlesChroma( uint32_t chroma ){
	cerr << "EEEE: doing OpenGLDisplay::handlesChroma "<<chroma<<endl;
	return chroma == M_CHROMA_RV24;
}

void OpenGLDisplay::displayImage( MImage * mimage ){
	cerr <<"EEEE: doing OpenGLDisplay::displayImage"<<endl;

	window->display(mimage, (uint64_t)this);

}

void OpenGLDisplay::handleEvents(){
	cerr <<"EEEE: doing OpenGLDisplay::handleEvents"<<endl;
#if 0
	OpenGL_Event event;

	while( OpenGL_PollEvent( &event ) ){
		if( event.type == OpenGL_KEYDOWN && event.key.keysym.sym == OpenGLK_f || event.type == OpenGL_MOUSEBUTTONDOWN && event.button.button == OpenGL_BUTTON_LEFT ){
#ifdef IPAQ
			if( fullscreen ){
				cerr << "not fullscreen" << endl;
				baseWindowWidth = width;
				baseWindowHeight = height;
				fullscreen = false;
			}
			else{
				cerr << "fullscreen" << endl;
				baseWindowWidth = 240;
				baseWindowHeight = 180;
				fullscreen = true;
			}

			//XResizeWindow( display, window, baseWindowWidth, baseWindowHeight );
			//XSync( display, false );
			OpenGL_UnlockSurface( surface );

			OpenGL_FreeSurface( surface );
			surface = OpenGL_SetVideoMode( baseWindowWidth, 
					baseWindowHeight, bpp, flags);
			initWm();
			OpenGL_LockSurface( surface );	
			//			destroyWindow();
			//			createWindow();


#endif
			if( fullscreen ){
				OpenGL_WM_ToggleFullScreen( surface );
			}
			break;
		}
		else if( event.type == OpenGL_VIDEORESIZE ){
			//XSync( display, false );
			baseWindowWidth = event.resize.w;
			baseWindowHeight = event.resize.h;

			OpenGL_UnlockSurface( surface );
			OpenGL_FreeSurface( surface );
			surface = OpenGL_SetVideoMode( baseWindowWidth, 
					baseWindowHeight, bpp, flags);
			//			initWm();
			OpenGL_LockSurface( surface );	
		}

	}
#endif
}

void OpenGLDisplay::handle( MImage * mimage ){
	cerr <<"EEEE: doing OpenGLDisplay::handle"<<endl;

	window->display(mimage, (uint64_t)this);
}


OpenGLPlugin::OpenGLPlugin( MRef<Library *> lib ): VideoDisplayPlugin( lib ){
}

OpenGLPlugin::~OpenGLPlugin(){
}
