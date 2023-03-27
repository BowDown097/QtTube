#ifndef OSUTILITIES_H
#define OSUTILITIES_H

#ifdef Q_OS_MACOS
#include <IOKit/pwr_mgt/IOPMLib.h>
#endif

class OSUtilities
{
public:
    static void toggleIdleSleep(bool toggle);
#ifdef Q_OS_MACOS
private:
    inline static IOPMAssertionID sleepAssert;
#endif
};

#endif // OSUTILITIES_H
