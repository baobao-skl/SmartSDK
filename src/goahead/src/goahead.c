/*
    goahead.c -- Main program for GoAhead

    Usage: goahead [options] [documents] [IP][:port] ...
        Options:
        --auth authFile        # User and role configuration
        --background           # Run as a Linux daemon
        --home directory       # Change to directory to run
        --log logFile:level    # Log to file file at verbosity level
        --route routeFile      # Route configuration file
        --verbose              # Same as --log stderr:2
        --version              # Output version information

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************* Includes ***********************************/

#include "goahead.h"
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>

/********************************* Defines ************************************/

static int finished = 0;

/********************************* Forwards ***********************************/

static void initPlatform();
static void logHeader();
static void usage();

#if WINDOWS
static void windowsClose();
static int windowsInit();
static LRESULT CALLBACK websWindProc(HWND hwnd, UINT msg, UINT wp, LPARAM lp);
#endif

#if BIT_UNIX_LIKE
static void sigHandler(int signo);
#endif

static int GetSessionUsername(int jid, Webs *wp, int argc, char **argv);
static void web_data_register(void);
static void GetData(Webs *wp);
static int GetMusicList(int jid, Webs *wp, int argc, char **argv);
static void MusicWebAction(Webs *wp);
static void web_music_register(void);
static int SendCommand(char *command);

/*********************************** Code *************************************/

MAIN(goahead, int argc, char **argv, char **envp)
{
    char  *argp, *home, *documents, *endpoints, *endpoint, *route, *auth, *tok;
    int     argind;

#if WINDOWS
    if (windowsInit() < 0) {
        return 0;
    }
#endif
    route = "route.txt";
    auth = "auth.txt";

    for (argind = 1; argind < argc; argind++) {
        argp = argv[argind];
        if (*argp != '-') {
            break;

        } else if (smatch(argp, "--auth") || smatch(argp, "-a")) {
            auth = argv[++argind];

        } else if (smatch(argp, "--background") || smatch(argp, "-b")) {
            websSetBackground(1);

        } else if (smatch(argp, "--debugger") || smatch(argp, "-d") || smatch(argp, "-D")) {
            websSetDebug(1);

        } else if (smatch(argp, "--home")) {
            if (argind >= argc) usage();
            home = argv[++argind];
            if (chdir(home) < 0) {
                error("Can't change directory to %s", home);
                exit(-1);
            }
        } else if (smatch(argp, "--log") || smatch(argp, "-l")) {
            if (argind >= argc) usage();
            logSetPath(argv[++argind]);

        } else if (smatch(argp, "--verbose") || smatch(argp, "-v")) {
            logSetPath("stdout:2");

        } else if (smatch(argp, "--route") || smatch(argp, "-r")) {
            route = argv[++argind];

        } else if (smatch(argp, "--version") || smatch(argp, "-V")) {
            printf("%s-%s\n", BIT_VERSION, BIT_BUILD_NUMBER);
            exit(0);

        } else {
            usage();
        }
    }
    documents = BIT_GOAHEAD_DOCUMENTS;
    if (argc > argind) {
        documents = argv[argind++];
    }
    initPlatform();
    if (websOpen(documents, route) < 0) {
        error("Can't initialize server. Exiting.");
        return -1;
    }
    if (websLoad(auth) < 0) {
        error("Can't load %s", auth);
        return -1;
    }
    logHeader();
    if (argind < argc) {
        while (argind < argc) {
            endpoint = argv[argind++];
            if (websListen(endpoint) < 0) {
                return -1;
            }
        }
    } else {
        endpoints = sclone(BIT_GOAHEAD_LISTEN);
        for (endpoint = stok(endpoints, ", \t", &tok); endpoint; endpoint = stok(NULL, ", \t,", &tok)) {
#if !BIT_PACK_SSL
            if (strstr(endpoint, "https")) continue;
#endif
            if (websListen(endpoint) < 0) {
                return -1;
            }
        }
        wfree(endpoints);
    }
#if BIT_ROM && KEEP
    /*
        If not using a route/auth config files, then manually create the routes like this:
        If custom matching is required, use websSetRouteMatch. If authentication is required, use websSetRouteAuth.
     */
    websAddRoute("/", "file", 0);
#endif
#if BIT_UNIX_LIKE && !MACOSX
    /*
        Service events till terminated
     */
    if (websGetBackground()) {
        if (daemon(0, 0) < 0) {
            error("Can't run as daemon");
            return -1;
        }
    }
#endif
    websJstOpen();
    websDefineJst("GetSessionUsername", GetSessionUsername);
    web_music_register();
    web_data_register();
    websServiceEvents(&finished);
    logmsg(1, "Instructed to exit");
    websClose();
#if WINDOWS
    windowsClose();
#endif
    return 0;
}


static int GetSessionUsername(int jid, Webs *wp, int argc, char **argv)
{
	websWrite(wp, "%s",websGetSessionVar(wp,WEBS_SESSION_USERNAME,0));
	return 1;
}


static void logHeader()
{
    char    home[BIT_GOAHEAD_LIMIT_STRING];

    getcwd(home, sizeof(home));
    logmsg(2, "Configuration for %s", BIT_TITLE);
    logmsg(2, "---------------------------------------------");
    logmsg(2, "Version:            %s-%s", BIT_VERSION, BIT_BUILD_NUMBER);
    logmsg(2, "BuildType:          %s", BIT_DEBUG ? "Debug" : "Release");
    logmsg(2, "CPU:                %s", BIT_CPU);
    logmsg(2, "OS:                 %s", BIT_OS);
    logmsg(2, "Host:               %s", websGetServer());
    logmsg(2, "Directory:          %s", home);
    logmsg(2, "Documents:          %s", websGetDocuments());
    logmsg(2, "Configure:          %s", BIT_CONFIG_CMD);
    logmsg(2, "---------------------------------------------");
}


static void usage() {
    fprintf(stderr, "\n%s Usage:\n\n"
        "  %s [options] [documents] [[IPaddress][:port] ...]\n\n"
        "  Options:\n"
        "    --auth authFile        # User and role configuration\n"
#if BIT_WIN_LIKE && !MACOSX
        "    --background           # Run as a Unix daemon\n"
#endif
        "    --debugger             # Run in debug mode\n"
        "    --home directory       # Change to directory to run\n"
        "    --log logFile:level    # Log to file file at verbosity level\n"
        "    --route routeFile      # Route configuration file\n"
        "    --verbose              # Same as --log stderr:2\n"
        "    --version              # Output version information\n\n",
        BIT_TITLE, BIT_PRODUCT);
    exit(-1);
}


static void initPlatform() 
{
#if BIT_UNIX_LIKE
    signal(SIGTERM, sigHandler);
    signal(SIGKILL, sigHandler);
    #ifdef SIGPIPE
        signal(SIGPIPE, SIG_IGN);
    #endif
#elif BIT_WIN_LIKE
    _fmode=_O_BINARY;
#endif
}


#if BIT_UNIX_LIKE
static void sigHandler(int signo)
{
    finished = 1;
}
#endif


#if WINDOWS
/*
    Create a taskbar entry. Register the window class and create a window
 */
static int windowsInit()
{
    HINSTANCE   inst;
    WNDCLASS    wc;                     /* Window class */
    HMENU       hSysMenu;
    HWND        hwnd;

    inst = websGetInst();
    wc.style         = CS_HREDRAW | CS_VREDRAW;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = inst;
    wc.hIcon         = NULL;
    wc.lpfnWndProc   = (WNDPROC) websWindProc;
    wc.lpszMenuName  = wc.lpszClassName = BIT_PRODUCT;
    if (! RegisterClass(&wc)) {
        return -1;
    }
    /*
        Create a window just so we can have a taskbar to close this web server
     */
    hwnd = CreateWindow(BIT_PRODUCT, BIT_TITLE, WS_MINIMIZE | WS_POPUPWINDOW, CW_USEDEFAULT, 
        0, 0, 0, NULL, NULL, inst, NULL);
    if (hwnd == NULL) {
        return -1;
    }

    /*
        Add the about box menu item to the system menu
     */
    hSysMenu = GetSystemMenu(hwnd, FALSE);
    if (hSysMenu != NULL) {
        AppendMenu(hSysMenu, MF_SEPARATOR, 0, NULL);
    }
    ShowWindow(hwnd, SW_SHOWNORMAL);
    UpdateWindow(hwnd);
    return 0;
}


static void windowsClose()
{
    HINSTANCE   inst;

    inst = websGetInst();
    UnregisterClass(BIT_PRODUCT, inst);
}


/*
    Main menu window message handler.
 */
static LRESULT CALLBACK websWindProc(HWND hwnd, UINT msg, UINT wp, LPARAM lp)
{
    switch (msg) {
        case WM_DESTROY:
            PostQuitMessage(0);
            finished++;
            return 0;

        case WM_SYSCOMMAND:
            break;
    }
    return DefWindowProc(hwnd, msg, wp, lp);
}


/*
    Check for Windows Messages
 */
WPARAM checkWindowsMsgLoop()
{
    MSG     msg;

    if (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE)) {
        if (!GetMessage(&msg, NULL, 0, 0) || msg.message == WM_QUIT) {
            return msg.wParam;
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}


/*
    Windows message handler
 */
static LRESULT CALLBACK websAboutProc(HWND hwndDlg, uint msg, uint wp, long lp)
{
    LRESULT    lResult;

    lResult = DefWindowProc(hwndDlg, msg, wp, lp);

    switch (msg) {
        case WM_CREATE:
            break;
        case WM_DESTROY:
            break;
        case WM_COMMAND:
            break;
    }
    return lResult;
}

#endif

/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2013. All Rights Reserved.

    This software is distributed under commercial and open source licenses.
    You may use the Embedthis GoAhead open source license or you may acquire 
    a commercial license from Embedthis Software. You agree to be fully bound
    by the terms of either license. Consult the LICENSE.md distributed with
    this software for full details and other copyrights.

    Local variables:
    tab-width: 4
    c-basic-offset: 4
    End:
    vim: sw=4 ts=4 expandtab

    @end
 */


void web_music_register(void)
{
	websDefineJst("GetMusicList", GetMusicList);
	websDefineAction("music", MusicWebAction);
}

static void MusicWebAction(Webs *wp)
{
	 char *cmd = websGetVar(wp, "action", "abc");
	 printf("cmd:%s\n",cmd);
	 if(strstr(cmd,"PLAY_START+")){
		SendCommand(cmd);
	 }
	 else	 if(strstr(cmd,"PLAY_PAUSE")){
		SendCommand(cmd);
	 }
	 else	 if(strstr(cmd,"PLAY_CONTINUE")){
		SendCommand(cmd);
	 }
	 else	 if(strstr(cmd,"PLAY_STOP")){
		SendCommand(cmd);
	 }
	 websWriteHeaders(wp, -1, 0);
    	 websWriteEndHeaders(wp);
	 websWrite(wp, "OK");   
	 websDone(wp); 
}

static int GetMusicList(int jid, Webs *wp, int argc, char **argv)
{
	unsigned char src[1024] = {0};
	unsigned char buffer[1024] = {0};int len = 1024,i;
	struct dirent *file_t;
	DIR * dir = opendir("/usr/ftp/music/");
	if(dir==NULL) return -1;
	websWrite(wp,"<table class=\"music_table\">");
	while((file_t = readdir(dir))!=NULL){
		if(strcmp(file_t->d_name,".")==0 || strcmp(file_t->d_name,"..")==0)
			continue;
		memset(src,0,sizeof(src));
		sprintf(src,"AAftp://192.168.0.10/music/%sZZ",file_t->d_name);
		memset(buffer,0,sizeof(buffer));
		i = base64_encode(buffer,&len,src,strlen(src));
		websWrite(wp,"<tr><td class=\"music_td\"><a href=\"javascript:void(0)\" onclick=\"control('PLAY_START%%2B'+this.innerText)\">%s</a></td><td class=\"music_td\"><a href=\"thunder://%s\">download</a></td></tr>",file_t->d_name,buffer);
		len = 1024;
	}
	websWrite(wp,"</table>");
	closedir(dir);
	return 1;
}

//thunder://QUFmdHA6Ly8xOTIuMTY4LjAuMTAvbXVzaWMv5pe26Ze06YO95Y675ZOq5YS/5LqG5Y675ZOq5LqG5ZOI5ZOI5ZOI5ZOI5ZOI5ZWK5ZWK5ZWK5ZWK5ZWK6Zi/6L+q546LZHNk6YW4ZHNk6YW455qELm1wM1pa



static int SendCommand(char *command)
{
	int socket_fd = 0;
	unsigned int on = 1;
	struct sockaddr_in server_addr;

	if((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
		return -1;
	}
	setsockopt(socket_fd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on));

	server_addr.sin_family 				=		AF_INET;
	server_addr.sin_port				= 		htons(2001);
	server_addr.sin_addr.s_addr			=		inet_addr("127.0.0.1");

	bzero(&(server_addr.sin_zero),8);

	if(connect(socket_fd,(struct sockaddr *)&server_addr,sizeof(struct sockaddr)) == -1) goto error;
	if(write(socket_fd,command,strlen(command)) < 0) goto error;
  	close(socket_fd);  
	return 0;
error:
	close(socket_fd);
	return -1;
}


static void web_data_register(void)
{
	websDefineAction("GetData", GetData);
}

static void GetData(Webs *wp)
{
	static int count = 1;
	if(count++>9999)count=1;

	websWriteHeaders(wp, -1, 0);
    	websWriteEndHeaders(wp);
	websWrite(wp, "%d,%d",count,count-1);   
	websDone(wp); 
}
