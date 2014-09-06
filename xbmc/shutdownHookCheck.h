// shutdownHookCheck.h
//
// @AUTHOR: Sven Burkard <dev@sven-burkard.de>
// @DESC..: checks for user defined xbmc-shutdown-hooks in a predefined hook directory.
// @DESC..: if one of the hook scripts returns an exit code not 0,
// @DESC..: the xbmc shutdown will be stopped and the user will be informed via xbmc-notifications
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// system includes
#include <cstdlib>    // system / getenv
#include <iostream>   // cout
#include <dirent.h>   // dir...
#include <string.h>   // strcmp
#include <sys/stat.h> // stat

// xbmc includes
#include "dialogs/GUIDialogKaiToast.h"

using namespace std;


////////////////////////////////////////////////////////
char* getHomeDirectory()
{
////////////////////////////////////////////////////////
  char *h=getenv("HOME");

  return h;
}


////////////////////////////////////////////////////////
bool fileIsExecutable(const string file)
{
////////////////////////////////////////////////////////

  struct stat  st;

  if(stat(file.c_str(), &st) < 0)
  {
    return false;
  }
  else if((st.st_mode & S_IEXEC) != 0)
  {
    return true;
  }

  return false;
}


////////////////////////////////////////////////////////
bool shutdownHookCheck()
{
////////////////////////////////////////////////////////

  DIR*      dir;
  dirent*   pdir;
  int       shutdownStopped = 0;
  string    dirHooks;

/*
//////////////////////////////////////////////////
  create a path-variable of the hook scripts dir
//////////////////////////////////////////////////
*/
  char* homeDirectory = getHomeDirectory();

  if(homeDirectory != NULL)
  {
    dirHooks    = homeDirectory;
  }
  else
  {
    dirHooks    = "/home/xbmc";
    cout << "ERROR: homeDirectory var is undef => using default homeDirectory!!!\n";
  }

  dirHooks    +=  "/xbmcShutdownHooks/hooks-enabled/";

  //cout << "DEBUG: >>>" << dirHooks << "<<<<<\n";
/*
//////////////////////////////////////////////////
*/

  dir                 = opendir(dirHooks.c_str());

  if(!dir)
  {
    CGUIDialogKaiToast::QueueNotification(CGUIDialogKaiToast::Warning, "shutdown-hook: " + dirHooks, "failed to open hooks directory, shutdown anyway");
    return true;    // return true because: a missing shutdown hook dir should't be a reason to abort exit/shutdown
  }

  while((pdir = readdir(dir)) != NULL)
  {

//    cout << "shutdownStopped: >" << shutdownStopped << "<\n";   // debug

    if(!strcmp(pdir->d_name, "."))          continue;
    if(!strcmp(pdir->d_name, ".."))         continue;
    if(!strcmp(pdir->d_name, ".gitignore")) continue;
    if(!strcmp(pdir->d_name, "README"))     continue;
    if(shutdownStopped == 1)                continue;


    string scriptName   = pdir->d_name;
    string combinedCmd  = dirHooks+scriptName;

//    cout << "scriptName: " << pdir->d_name << "\n";   // debug

    if(!fileIsExecutable(combinedCmd))
    {
      CGUIDialogKaiToast::QueueNotification(CGUIDialogKaiToast::Error, "shutdown-hook-check: ", scriptName + ": can NOT be executed");
      return false;
    }


    int exitCode = system(combinedCmd.c_str());

    if(exitCode == 0)
    {
      CGUIDialogKaiToast::QueueNotification(CGUIDialogKaiToast::Info, "shutdown-hook-check: ", scriptName + ": ok");
    }
    else
    {
      shutdownStopped = 1;
//      cout << "setting shutdownStopped to 1 !!!\n";   // debug

      CGUIDialogKaiToast::QueueNotification(CGUIDialogKaiToast::Info, "shutdown-hook-check: ", "shutdown stopped by " + scriptName);

      closedir(dir);
      return false;
    }

  }
  closedir(dir);


  return true;
}
