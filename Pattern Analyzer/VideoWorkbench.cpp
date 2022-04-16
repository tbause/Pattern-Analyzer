// Pattern Analyzer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

using namespace cv;
using namespace std;

// --------------------------------------------------------------------------------------------------------------------------------
// GLOBAL VARIABLES
// --------------------------------------------------------------------------------------------------------------------------------

string g_AppDirectory;
string g_AppLogFilePath;

// --------------------------------------------------------------------------------------------------------------------------------
// Analyze Media File Function
// --------------------------------------------------------------------------------------------------------------------------------
void AnalyzeMediaFile(const char *pfilename)
{
	// ---------------------------------------------------------------
	// Local Variables (Start)
	// ---------------------------------------------------------------
	VideoAnalyzer VAnalyzer;

	char c = 0;
	char OverlayText[80];
    CvFont myTextFont;

	// Log file handling
	FILE 			*LogFileHandle;

	// Time measurement
	clock_t			StartTime;
	clock_t			EndTime;
	double			TimeDif;

	// Video stream info
	unsigned int	FrameCounter			= 0;

	// Pattern information
    CvPoint			LetterboxValues			= cvPoint(0, 0);
	bool			FirstFrame				= true;
    CvScalar		DifferenceAvgResults	= cvScalar(0,0,0,0);
    CvScalar		EnergyResults			= cvScalar(0,0,0,0);
	CvScalar		StructDiffAvgResults	= cvScalar(0,0,0,0);
	double			HistDiffBuffer[5]		= {0, 0, 0, 0, 0};
	int				EditListSignal			= 0;
	unsigned int	EditPosition			= 0;

	// Histogram variables
	int				h_bins					= 60;
	int				s_bins					= 64;
	int				scale					= 5;
	double			HistogramCompareResult	= 0;

	// Histogram created from active video frame
	CvHistogram* ActiveFrameHistogram;
	{
		int hist_size[] = { h_bins, s_bins };
		float h_ranges[] = { 0, 180 }; // hue is [0,180]
		float s_ranges[] = { 0, 255 };
		float* ranges[] = { h_ranges, s_ranges };
		ActiveFrameHistogram = cvCreateHist(2, hist_size, CV_HIST_ARRAY, ranges, 1);
	}

	// Histogram for buffered video frame
	CvHistogram* BufferedFrameHistogram;
	{
		int hist_size[] = { h_bins, s_bins };
		float h_ranges[] = { 0, 180 }; // hue is [0,180]
		float s_ranges[] = { 0, 255 };
		float* ranges[] = { h_ranges, s_ranges };
		BufferedFrameHistogram = cvCreateHist(2, hist_size, CV_HIST_ARRAY, ranges, 1);
	}

	// Variables to hold video frame data
	IplImage*			ActiveFrame;
    IplImage*			BufferedFrame;
	IplImage*			LBXDisplayFrame;
	IplImage*			HistogramActiveFrame;
	IplImage*			HistogramBufferedFrame;
    IplImage*			DifferenceDisplayFrame;
    IplImage*			CannyDisplayFrame;

	// NEW WAY OF DEFINING FRAME
	cv::Mat frame; // current video frame

	// ---------------------------------------------------------------
	// Local Variables (End)
	// ---------------------------------------------------------------

	// Define font for image overlays
	cvInitFont(&myTextFont, CV_FONT_HERSHEY_SIMPLEX, 1.0, 1.0, 0.0, 1, 8);

	// Create output windows
	cvNamedWindow( "Active Frame", CV_WINDOW_AUTOSIZE );
	// cvNamedWindow( "Letterbox", CV_WINDOW_AUTOSIZE );
	// cvNamedWindow( "Active Frame Histogram", CV_WINDOW_AUTOSIZE );
	// cvNamedWindow( "Frame Difference", CV_WINDOW_AUTOSIZE );
	cvNamedWindow( "Canny Filter", CV_WINDOW_AUTOSIZE );

	// OLD WAY TO OPEN VIDEO FILE
	// Open the source file
	// CvCapture* capture = cvCreateFileCapture( pfilename );

	// NEW WAY TO OPEN VIDEO FILE  pfilename
	VideoCapture capture(pfilename);


	// Open the log file to store pattern values
	LogFileHandle = fopen("E:\\CV\\ContentAnalyzerOutput.log", "w"); /* open for writing */

	// Print head line for log
	fprintf(LogFileHandle,"Frame# / Struct. Diff Ch.1 / Struct. Diff Ch.2 / Struct. Diff Ch.3 / Struct. Diff Ch.4 / Diff Ch.1 / Diff Ch.2 / Diff Ch.3 / Diff Ch.4 / Eng. Ch.1  / Eng. Ch.2 / Eng. Ch.3 / Eng. Ch.4 / LBX X / LBX Y / Time\n");

	// Initial histogram variables
	HistogramActiveFrame   = cvCreateImage(cvSize( h_bins * scale, s_bins * scale ), IPL_DEPTH_8U, 3);
	HistogramBufferedFrame = cvCreateImage(cvSize( h_bins * scale, s_bins * scale ), IPL_DEPTH_8U, 3);

	// Initialize frames
	LBXDisplayFrame			= cvCreateImage(cvSize(720, 480), IPL_DEPTH_8U, 3);
	BufferedFrame			= cvCreateImage(cvSize(720, 480), IPL_DEPTH_8U, 3);
	DifferenceDisplayFrame	= cvCreateImage(cvSize(720, 480), IPL_DEPTH_8U, 3);
	CannyDisplayFrame		= cvCreateImage(cvSize(720, 480), IPL_DEPTH_8U, 1);

	// Process frame by frame till ESC key was pressed
	while(c != 27)
	{
		// OLD WAY TO ACCESS VIDEO FRAME
		// Get one frame
		// IplImage* ActiveFrame = cvQueryFrame( capture );
		// if( !ActiveFrame ) break;

		// read next frame if any
		if (!capture.read(frame))
			break;

		// Copy picture data from new MAT structure to IplImage structure
		ActiveFrame = cvCloneImage(&(IplImage)frame);

		// Initial process variables using the first frame
		if(FirstFrame)
		{
			LBXDisplayFrame			= cvCreateImage(cvSize(ActiveFrame->width, ActiveFrame->height), IPL_DEPTH_8U, 3);
			BufferedFrame			= cvCreateImage(cvSize(ActiveFrame->width, ActiveFrame->height), IPL_DEPTH_8U, 3);
			DifferenceDisplayFrame	= cvCreateImage(cvSize(ActiveFrame->width, ActiveFrame->height), IPL_DEPTH_8U, 3);
			CannyDisplayFrame		= cvCreateImage(cvSize(ActiveFrame->width, ActiveFrame->height), IPL_DEPTH_8U, 1);

			FirstFrame = false;
		}

		// remember process start time
		StartTime = clock();

		// Calculate histogram 
		// ActiveFrameHistogram = VAnalyzer.CalculateHistogram(ActiveFrame, HistogramActiveFrame, h_bins, s_bins, scale);

		// Buffered operations. Exclude first frame.
		if(!FirstFrame)
		{
			// Calculate histogram difference (buffered vs. active histogram). 
			HistogramCompareResult		= cvCompareHist(ActiveFrameHistogram, BufferedFrameHistogram, CV_COMP_CHISQR);

			EditPosition = VAnalyzer.SceneChangeDetection(HistDiffBuffer, HistogramCompareResult, FrameCounter);

			if(EditPosition > 0)
				EditListSignal = 1;
			else
				EditListSignal = 0;
			/*
			// Calculate average difference between frames
			DifferenceAvgResults    	= VAnalyzer.AvgDifference(ActiveFrame, BufferedFrame, DifferenceDisplayFrame);
			// Calculate average difference in structure between frames
			StructDiffAvgResults		= VAnalyzer.AvgStructureDiff(ActiveFrame, BufferedFrame, CannyDisplayFrame);
			*/
		}

		// Save histogram data for calculation with next frame
		BufferedFrameHistogram = ActiveFrameHistogram;

		// Calculate letterbox values
		LetterboxValues = VAnalyzer.LetterboxDetection(ActiveFrame, LBXDisplayFrame, 0);
		// Calculate average energy contents of frame
		EnergyResults	= cvAvg(ActiveFrame, NULL);

		// remember process end time
		EndTime = clock();
		FrameCounter++;

		cvCopy(ActiveFrame, BufferedFrame, NULL);

	    sprintf(OverlayText, "Frame: %d", FrameCounter);
		cvPutText(ActiveFrame, OverlayText, cvPoint(10, 460), &myTextFont, cvScalar(255, 255, 255, 0));

		// Display source frame
		cvShowImage( "Active Frame", ActiveFrame );
		// Display Letterbox
		// cvShowImage( "Letterbox", LBXDisplayFrame );
		// Display Letterbox
		// cvShowImage( "Active Frame Histogram", HistogramActiveFrame );
		// Display Frame Difference
		// cvShowImage( "Frame Difference", DifferenceDisplayFrame );
		// Display Canny filter output
		cvShowImage( "Canny Filter", CannyDisplayFrame );

		// Caclulate processing time for one frame
		TimeDif = (double) VAnalyzer.diffclock(EndTime, StartTime);

		// Output line of values (one frame) to log file
		fprintf(LogFileHandle,"%d, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f, %d, %d, %d, %.6f\n",
				FrameCounter,
				StructDiffAvgResults.val[0],
				StructDiffAvgResults.val[1],
				StructDiffAvgResults.val[2],
				StructDiffAvgResults.val[3],
				DifferenceAvgResults.val[0],
				DifferenceAvgResults.val[1],
				DifferenceAvgResults.val[2],
				DifferenceAvgResults.val[3],
				EnergyResults.val[0],
				EnergyResults.val[1],
				EnergyResults.val[2],
				EnergyResults.val[3],
	    		HistogramCompareResult,
				LetterboxValues.x,
				LetterboxValues.y,
				EditListSignal,
				TimeDif);

		// Wait 42ms (frame rate = 24 frames a sec.)
		c = cvWaitKey(1);
	}

	// Close log files
	fclose(LogFileHandle);

	// Release source
	capture.release();

	// Destroy all test windows
	cvDestroyWindow( "Active Frame" );
	// cvDestroyWindow( "Letterbox" );
	// cvDestroyWindow( "Active Frame Histogram" );
	cvDestroyWindow( "Frame Difference" );
	cvDestroyWindow( "Canny Filter" );

	// Release memory for images
	// cvReleaseImage(&ActiveFrame);
	cvReleaseImage(&LBXDisplayFrame);
	cvReleaseImage(&HistogramActiveFrame);
	cvReleaseImage(&DifferenceDisplayFrame);
	cvReleaseImage(&CannyDisplayFrame);
}


// --------------------------------------------------------------------------------------------------------------------------------
// Main Function
// --------------------------------------------------------------------------------------------------------------------------------

int main(int argc, char* argv[] )
{
	VideoProcessor			VProcessor;
	VideoAnalyzer			VAnalyzer;
	CommonMessageHandler	MessageHandler;
	CommonToolBox			ToolBox;
	ParameterHandling		ParameterSet;

	ParameterSet.InitializeParameterSet(argc, argv);

	MessageHandler.PrintApplicationHeader();

	// Check if Help argument was not used
	if(!ParameterSet.GetHasShowHelp())
	{
		// Check that the mandatory arguments were provided
		if(ParameterSet.GetHasInputFilePath() && ParameterSet.GetHasOutputFilePath())
		{
			// Check if only processor function was used
			if (ParameterSet.GetHasStartFrame() && ParameterSet.GetHasDurationFrames() && ParameterSet.GetHasProcessorFunction() && !ParameterSet.GetHasAnalyzerFunction())
			{
				// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
				// Implement YUV file input for processing
				// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
				// De-Interlacer function selected
				if(ParameterSet.GetisDeInterlaceProcess())
				{
					MessageHandler.PrintCommandLineMessage("Starting de-interlacing process", true, true);
					// Call de-interlacer function of Video Processor
					VProcessor.DeInterlaceFile(	ParameterSet.GetInputFilePath(),
												ParameterSet.GetOutputFilePath(),
												ParameterSet.GetStartFrame(),
												ParameterSet.GetDurationFrames());
					MessageHandler.PrintCommandLineMessage("Finished de-interlacing process", false, true);
				}
				// Interlacer function selected
				else if (ParameterSet.GetisInterlaceProcess() && ParameterSet.GetHasInputImageWidth() && ParameterSet.GetHasInputImageHeight() && ParameterSet.GetHasInputImageType())
				{
					// Check if YUV file extension was used (both cases)
					if(ToolBox.ReturnFileExtension(ParameterSet.GetOutputFilePath()) == "yuv" || ToolBox.ReturnFileExtension(ParameterSet.GetOutputFilePath()) == "YUV")
					{
						MessageHandler.PrintCommandLineMessage("Starting interlacing process", true, true);
						// Call interlacer function of Video Processor
						VProcessor.InterlaceFile( ParameterSet.GetInputFilePath(),
												  ParameterSet.GetOutputFilePath(),
												  ParameterSet.GetStartFrame(),
												  ParameterSet.GetDurationFrames(),
												  ParameterSet.GetInputImageWidth(),
												  ParameterSet.GetInputImageHeight());
						MessageHandler.PrintCommandLineMessage("Finished interlacing process", false, true);
					}
					else
					{
						// Error message file extension mismatch
						MessageHandler.PrintCommandLineMessage("Input file is not a YUV file", true, true);
					}
				}
			}
			// Check if only analyzer function was used
			else if(!ParameterSet.GetHasProcessorFunction() && ParameterSet.GetHasAnalyzerFunction())
			{
				// ANALYZER FUNCTION: CONTENT ANALYZER
				if (ParameterSet.GetisContentAnalyzer())
				{
					MessageHandler.PrintCommandLineMessage("Starting Content Analyzer", true, true);

					VAnalyzer.ContentAnalyzer(ParameterSet.GetInputFilePath(),
											  ParameterSet.GetOutputFilePath(),
											  ParameterSet.GetStartFrame(),
											  ParameterSet.GetDurationFrames(),
											  ParameterSet.GetHasVerbose(),
											  ParameterSet.GetHasDisplay());

					MessageHandler.PrintCommandLineMessage("Finished content analyzing", false, true);
				}
				// ANALYZER FUNCTION: CADENCE ANALYZER
				if (ParameterSet.GetisCadenceAnalyzer())
				{
					MessageHandler.PrintCommandLineMessage("Starting Cadence Analyzer", true, true);
					
					VAnalyzer.CadenceDetector(ParameterSet.GetInputFilePath(),
											  ParameterSet.GetOutputFilePath(),
											  ParameterSet.GetStartFrame(),
											  ParameterSet.GetDurationFrames(),
											  ParameterSet.GetHasVerbose(),
											  ParameterSet.GetHasDisplay());

					MessageHandler.PrintCommandLineMessage("Finished Cadence analyzing", false, true);
				}
				// ANALYZER FUNCTION: DPX PARSER
				if (ParameterSet.GetisDPXAnalyzer())
				{
					MessageHandler.PrintCommandLineMessage("Starting DPX Analyzer", true, true);
					// Call DPX parser
					DPXAnalyzer DPXParser;

					DPXParser.ExtractDPXMetadata(ParameterSet.GetInputFilePath(), ParameterSet.GetOutputFilePath());

					MessageHandler.PrintCommandLineMessage("Finished DPX analyzing", false, true);
				}
				// ANALYZER FUNCTION: MXF PARSER
				if (ParameterSet.GetisMXFAnalyzer())
				{
					MessageHandler.PrintCommandLineMessage("Starting MXF Analyzer", true, true);
					// Call MXF parser
					MXFAnalyzer::MXFParser MXFAnalyzer;

					MXFAnalyzer.ExtractMXFMetadata(ParameterSet.GetInputFilePath(), ParameterSet.GetOutputFilePath());

					MessageHandler.PrintCommandLineMessage("Finished MXF analyzing", false, true);
				}

			}
			// Error message due to both processor and analyzer function selected
			else
			{
				MessageHandler.PrintCommandLineMessage("Error in argument list. Can only do processor or analyzer function, not both", true, true);
				MessageHandler.PrintApplicationLegend();
			}
		}
		// Error message due to missing mandatory argument
		else
		{
			MessageHandler.PrintCommandLineMessage("Error in argument list. Missing Start, Duration, Input or Output file argument.", true, true);
			MessageHandler.PrintApplicationLegend();
		}

	}
	else
	{
		MessageHandler.PrintApplicationLegend();
	}
	
	return 0;
}


