#pragma once

#include "Delegate.hpp"

inline const constinit char8* GEngineName{"LiteEngine"};
inline const constinit uint32 GEngineVersion{1};

inline const constinit char8* GProjectName{"TriangleProject"};
inline const constinit uint32 GProjectVersion{1};

inline bool GIsEngineExitRequested{false};

inline TDelegate<void> GOnEngineShutdown{};

