#pragma once

namespace BuildInfo
{
inline constexpr const char *version()
{
    return "1.0.1.20260505";
}

inline constexpr int versionMajor()
{
    return 1;
}

inline constexpr int versionMinor()
{
    return 0;
}

inline constexpr int versionPatch()
{
    return 1;
}

inline constexpr int versionDate()
{
    return 20260505;
}
} // namespace BuildInfo
