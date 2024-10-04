#pragma once

#ifndef FLUF_UI_API
    #ifdef FLUF_UI
        #define FLUF_UI_API __declspec(dllexport)
    #else
        #define FLUF_UI_API __declspec(dllimport)
    #endif
#endif
