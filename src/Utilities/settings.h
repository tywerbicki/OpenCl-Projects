#ifndef UTILITIES_SETTINGS_H
#define UTILITIES_SETTINGS_H


namespace settings
{
    // TODO: Read in settings from YAML at runtime.
    //       Settings should be able to be toggled without requiring recompilation.

#ifdef _DEBUG
    inline extern const bool displayPlatformInfo          = true;
    inline extern const bool displayGeneralDeviceInfo     = false;
    inline extern const bool enableProgramBinaryCaching   = true;
    inline extern const bool forceCreateProgramFromSource = true;
#elif defined(_RELEASE)
    inline extern const bool displayPlatformInfo          = false;
    inline extern const bool displayGeneralDeviceInfo     = false;
    inline extern const bool enableProgramBinaryCaching   = true;
    inline extern const bool forceCreateProgramFromSource = false;
#endif // _RELEASE
}


#endif // UTILITIES_SETTINGS_H