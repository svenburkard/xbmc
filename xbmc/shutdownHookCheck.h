// shutdownHookCheck.h
//
// @AUTHOR: Sven Burkard <dev@sven-burkard.de>
// @DESC..: checks for user defined xbmc-shutdown-hooks in a predefined hook directory.
// @DESC..: if one of the hook scripts returns an exit code not 0,
// @DESC..: the xbmc shutdown will be stopped and the user will be informed via xbmc-notifications
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////


#include <cstdlib>    // system / getenv
#include <iostream>   // cout
#include <dirent.h>   // dir...
#include <string.h>   // strcmp
#include <sys/stat.h> // stat


using namespace std;



////////////////////////////////////////////////////////
char* getUserName()
{
////////////////////////////////////////////////////////
    char *u=getenv("USER");

    return u;
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
bool sendMSGtoXBMC(string title, string message)
{
////////////////////////////////////////////////////////

  string curl         = "/usr/bin/curl";

  if(!fileIsExecutable(curl))
  {
    cout << "ERROR: file (" + curl + ") can NOT be executed\n";
    return false;
  }

  string xbmcUser     = "xbmc";
  string xbmcPassword = "xbmc";
  string xbmcHost     = "localhost";
  string xbmcPort     = "8080";

  string cmd;

  cmd +=  curl;
  cmd +=  " ";
  cmd +=  "-s ";
  cmd +=  "-u \"" + xbmcUser + ":" + xbmcPassword + "\" ";
  cmd +=  "-H \"Content-type: application/json\" ";
  cmd +=  "-X POST ";
  cmd +=  "-d '{\"id\":1,\"jsonrpc\":\"2.0\",\"method\":\"GUI.ShowNotification\",\"params\":{\"title\":\"" + title + "\",\"message\":\"" + message + "\"}}' ";
  cmd +=  "http://" + xbmcHost + ":" + xbmcPort + "/jsonrpc ";
  cmd +=  "1>/dev/null ";


/*
//////////////////////////////////////////////////
  debug
//////////////////////////////////////////////////
*/
//  cout << "  xbmcHost:    " << xbmcHost << "\n";
//  cout << "  xbmcPort:    " << xbmcPort << "\n";
//  cout << "  cmd:         " << cmd << "\n";
//  cout << "  title:       " << title << "\n";
//  cout << "  message:     " << message << "\n";
/*
//////////////////////////////////////////////////
*/


  int exitCode = system(cmd.c_str());

  if(exitCode != 0)
  {
    cout << "ERROR: no valid exitCode @ sendMSGtoXBMC()\n";
  }

  return true;
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
  dirHooks    +=  "/home/";

  char* userName = getUserName();

  if(userName != NULL)
  {
    dirHooks  +=  userName;
  }
  else
  {
    string userNameDefault  = "xbmc";

    cout << "ERROR: userName var is undef => using default userName (" << userNameDefault << ")!!!\n";
    dirHooks  +=  userNameDefault;
  }

  dirHooks    +=  "/xbmcShutdownHooks/hooks-enabled/";
/*
//////////////////////////////////////////////////
*/

  dir                 = opendir(dirHooks.c_str());

  if(!dir)
  {
    if(!sendMSGtoXBMC("shutdown-hook: " + dirHooks, "failed to open hooks directory, shutdown anyway"))
    {
      cout << "ERROR: sendMSGtoXBMC failed!!!!!!!\n";
    }

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
      sendMSGtoXBMC("shutdown-hook-check: ",  scriptName + ": can NOT be executed");
      return false;
    }


    int exitCode = system(combinedCmd.c_str());

    if(exitCode == 0)
    {

      if(sendMSGtoXBMC("shutdown-hook-check: ",  scriptName + ": ok"))
      {
        cout << "sendMSGtoXBMC was successfull\n";
      }
      else
      {
        cout << "ERROR: sendMSGtoXBMC failed!!!!!!!\n";
      }

    }
    else
    {

      shutdownStopped = 1;
//      cout << "setting shutdownStopped to 1 !!!\n";   // debug

      if(sendMSGtoXBMC("shutdown-hook-check: ", "shutdown stopped by " + scriptName))
      {
        cout << "sendMSGtoXBMC was successfull\n";
      }
      else
      {
        cout << "ERROR: sendMSGtoXBMC failed!!!!!!!\n";
      }


      closedir(dir);
      return false;

    }

  }
  closedir(dir);


  return true;
}
