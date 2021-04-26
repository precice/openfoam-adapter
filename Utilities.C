#include "Utilities.H"

using namespace Foam;

void adapterInfo(const std::string message, const std::string level)
{
    if (level.compare("info") == 0)
    {
        // Prepend the message with a string
        Info << INFO_STR_ADAPTER
             << message.c_str()
             << nl;
    }
    else if (level.compare("warning") == 0)
    {
        // Produce a warning message with cyan header
        WarningInFunction
            << "\033[36m" // cyan color
            << "Warning in the preCICE adapter: "
            << "\033[0m" // restore color
            << nl
            << message.c_str()
            << nl
            << nl;
    }
    else if (level.compare("error") == 0)
    {
        // Produce an error message with red header
        // and exit the functionObject.
        // It will also exit the simulation, unless it
        // is called inside the functionObject's read().
        FatalErrorInFunction
            << "\033[31m" // red color
            << "Error in the preCICE adapter: "
            << "\033[0m" // restore color
            << nl
            << message.c_str()
            << nl
            << exit(FatalError);
    }
    else if (level.compare("error-deferred") == 0)
    {
        // Produce an warning message with red header.
        // OpenFOAM degrades errors inside read()
        // to warnings, stops the function object, but does
        // not exit. We throw a warning which is described
        // as an error, so that OpenFOAM does not exit,
        // but the user still sees that this is the actual
        // problem. We catch these errors and exit later.
        WarningInFunction
            << "\033[31m" // red color
            << "Error (deferred - will exit later) in the preCICE adapter: "
            << "\033[0m" // restore color
            << nl
            << message.c_str()
            << nl
            << nl;
    }
    else if (level.compare("debug") == 0)
    {
        Info << INFO_STR_ADAPTER
             << "[DEBUG] "
             << message.c_str()
             << nl;
    }
    else if (level.compare("dev") == 0)
    {
        Info << "\033[35m" // cyan color
             << INFO_STR_ADAPTER
             << "[under development] "
             << "\033[0m " // restore color
             << message.c_str()
             << nl;
    }
    else
    {
        Info << INFO_STR_ADAPTER
             << "[unknown info level] "
             << message.c_str()
             << nl;
    }

    return;
}
