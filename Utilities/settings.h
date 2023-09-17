#pragma once


namespace settings
{

#ifdef _DEBUG

inline extern const bool displayPlatformInfo          = true;
inline extern const bool displayGeneralDeviceInfo     = true;
inline extern const bool enableProgramBinaryCaching   = true;
inline extern const bool forceCreateProgramFromSource = true;

#else

inline extern const bool displayPlatformInfo          = false;
inline extern const bool displayGeneralDeviceInfo     = false;
inline extern const bool enableProgramBinaryCaching   = true;
inline extern const bool forceCreateProgramFromSource = false;

#endif // _DEBUG

} // settings