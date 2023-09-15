#pragma once


namespace settings
{

#ifdef _DEBUG

inline extern const bool enableProgramBinaryCaching   = true;
inline extern const bool forceCreateProgramFromSource = true;

#else

inline extern const bool enableProgramBinaryCaching   = true;
inline extern const bool forceCreateProgramFromSource = false;

#endif // _DEBUG

} // settings