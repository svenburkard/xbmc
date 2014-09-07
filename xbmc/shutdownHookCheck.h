// shutdownHookCheck.h
//
// @AUTHOR: Sven Burkard <dev@sven-burkard.de>
// @DESC..: checks for user defined xbmc-shutdown-hooks in a predefined hook directory.
// @DESC..: if one of the hook scripts returns an exit code not 0,
// @DESC..: the xbmc shutdown will be stopped and the user will be informed via xbmc-notifications
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// system includes
#include <cstdlib>    // system / getenv
//#include <iostream>   // cout
#include <dirent.h>   // dir...
#include <string.h>   // strcmp
#include <sys/stat.h> // stat

// xbmc includes
#include "dialogs/GUIDialogKaiToast.h"
#include "filesystem/SpecialProtocol.h"


using namespace std;


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
  string    dirHome;
  string    dirHooks;

/*
//////////////////////////////////////////////////
  create a path-variable of the hook scripts dir
//////////////////////////////////////////////////
*/
  dirHome   = CSpecialProtocol::TranslatePath("special://home/").c_str();

  CLog::Log(LOGDEBUG,"[shutdownHookCheck] dirHome:>>%s<<", dirHome.c_str());


  dirHooks  += dirHome;
  dirHooks  += "../xbmcShutdownHooks/hooks-enabled/";

  CLog::Log(LOGDEBUG,"[shutdownHookCheck] dirHooks:>>%s<<", dirHooks.c_str());
/*
//////////////////////////////////////////////////
*/

  dir                 = opendir(dirHooks.c_str());

  if(!dir)
  {
    CGUIDialogKaiToast::QueueNotification(CGUIDialogKaiToast::Warning, "shutdown-hook: " + dirHooks, "hooks directory couldn't be opened, shutdown anyway");
    CLog::Log(LOGWARNING,"[shutdownHookCheck] hooks directory (dirHooks: %s) couldn't be opened, shutdown anyway", dirHooks.c_str());
    return true;    // return true because: a missing shutdown hook dir should't be a reason to abort exit/shutdown
  }

  while((pdir = readdir(dir)) != NULL)
  {

    CLog::Log(LOGDEBUG,"[shutdownHookCheck] shutdownStopped:>>%i<<", shutdownStopped);

    if(!strcmp(pdir->d_name, "."))          continue;
    if(!strcmp(pdir->d_name, ".."))         continue;
    if(!strcmp(pdir->d_name, ".gitignore")) continue;
    if(!strcmp(pdir->d_name, "README"))     continue;
    if(shutdownStopped == 1)                continue;


    string scriptName   = pdir->d_name;
    string combinedCmd  = dirHooks+scriptName;

    CLog::Log(LOGDEBUG,"[shutdownHookCheck] scriptName:>>%s<<", scriptName.c_str());

    if(!fileIsExecutable(combinedCmd))
    {
      CGUIDialogKaiToast::QueueNotification(CGUIDialogKaiToast::Error, "shutdown-hook-check: ", scriptName + ": can NOT be executed");
      CLog::Log(LOGERROR,"[shutdownHookCheck] %s : can NOT be executed", scriptName.c_str());
      return false;
    }


    int exitCode = system(combinedCmd.c_str());

    if(exitCode == 0)
    {
      CGUIDialogKaiToast::QueueNotification(CGUIDialogKaiToast::Info, "shutdown-hook-check: ", scriptName + ": ok");
      CLog::Log(LOGINFO,"[shutdownHookCheck] %s: ok", scriptName.c_str());
    }
    else
    {
      shutdownStopped = 1;
      CLog::Log(LOGDEBUG,"[shutdownHookCheck] setting shutdownStopped to 1 !!!");

      CGUIDialogKaiToast::QueueNotification(CGUIDialogKaiToast::Info, "shutdown-hook-check: ", "shutdown stopped by " + scriptName);
      CLog::Log(LOGINFO,"[shutdownHookCheck] shutdown stopped by %s", scriptName.c_str());

      closedir(dir);
      return false;
    }

  }
  closedir(dir);


  return true;
}
