// shutdownHookCheck.h
//
// @AUTHOR: Sven Burkard <dev@sven-burkard.de>
// @DESC..: checks for user defined xbmc-shutdown-hooks in a predefined hook directory.
// @DESC..: if one of the hook scripts returns an exit code not 0,
// @DESC..: the xbmc shutdown will be stopped and the user will be informed via xbmc-notifications
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////


class CshutdownHookCheck
{
  public:
    bool shutdownHookCheck();
};

