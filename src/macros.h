#pragma once

#ifdef VWBAR_EXPORTS
  #define VWBAR_API __declspec(dllexport)
#else
  #define VWBAR_API __declspec(dllimport)
#endif
