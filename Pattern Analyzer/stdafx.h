// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#define VW_LOG_FILENAME "VideoWorkbench.log"

#define VW_PROCESSOR			0
#define VW_ANALYZER				1

#define VW_PROCESS_DEINTERLACE	0
#define VW_PROCESS_INTERLACE	1

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>
#include <string.h>
#include <cmath>
#include <queue>

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <windows.h>

// TODO: reference additional headers your program requires here

// additional C/C++ includes
#include <time.h>

// OpenCV 2.4.9 includes
#include <cv.h>
#include <cxcore.h>
#include <highgui.h>
#include <cvaux.h>

// include files from classes
#include "VideoProcessor.h"
#include "VideoAnalyzer.h"
#include "Logger.h"
#include "YUVFileHandling.h"
#include "CommonToolBox.h"
#include "ColorToolBox.h"
#include "ParameterHandling.h"
#include "DPXAnalyzer.h"
#include "MXFAnalyzer_Types.h"
#include "MXFAnalyzer_KLV.h"
#include "MXFAnalyzer.h"
