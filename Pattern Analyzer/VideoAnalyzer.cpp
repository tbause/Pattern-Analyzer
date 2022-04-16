// VideoAnalyzer.cpp : Defines the analyzer class and its methods and properties
//

#include "stdafx.h"

using namespace cv;
using namespace std;

// namespace VideoAnalyzer
// {

	// --------------------------------------------------------------------------------------------------------------------------------
	// diffclock
	// July 2014, Thomas Bause Mason
	//
	// Returns the difference between two time values
	// 
	// Parameter:
	// clock1				= 1st time value
	// clock2				= 2nd time value
	// double				= Return value for the difference between two times
	// --------------------------------------------------------------------------------------------------------------------------------
	double VideoAnalyzer::diffclock(clock_t clock1, clock_t clock2)
	{
		double diffticks = clock1 - clock2;
		double diffms = ((diffticks) / CLOCKS_PER_SEC) * 1000;
		return diffms;
	}

	// --------------------------------------------------------------------------------------------------------------------------------
	// CalculateHSVHistogram
	// July 2014, Thomas Bause Mason
	//
	// Calculate a histogram based on the HSV version of an image
	// 
	// Parameter:
	// SourceImage			= Source image
	// OutputHistogram		= Image of the resulting histogram (not implemented)
	// H_Bins				= Number of hue bins
	// S_Bins				= Number of saturation bins
	// MatND				= Return value is histogram
	// --------------------------------------------------------------------------------------------------------------------------------
	MatND VideoAnalyzer::CalculateHSVHistogram(Mat &SourceImage, int H_Bins, int S_Bins, int HUEMaxValue, int SatMaxValue)
	{
		bool			uniform			= true; 
		bool			accumulate		= false;

		Mat				HSVImage;												// HSV version of source image
		MatND			Histogram;												// Histogram

		int				HistogramSize[] = { H_Bins, S_Bins };					// Using bins for hue and for saturation to calculate histogram size
		float			H_Ranges[]		= { 0, HUEMaxValue };					// Hue varies from 0 to 179
		float			S_Ranges[]		= { 0, SatMaxValue };					// Saturation from 0 to 255
		const float*	Ranges[]		= { H_Ranges, S_Ranges };				// Histogram Ranges
		int				channels[]		= { 0, 1 };								// Use the o-th and 1-st channels

		Histogram.create(H_Bins, S_Bins, SourceImage.type());
		HSVImage.create(SourceImage.rows, SourceImage.cols, SourceImage.type());

		// Convert source image to HSV image
		cvtColor(SourceImage, HSVImage, COLOR_BGR2HSV);

		/// Compute the histograms
		calcHist(&HSVImage, 1, channels, Mat(), Histogram, 2, HistogramSize, Ranges, uniform, accumulate);
		normalize(Histogram, Histogram, 0, 1, NORM_MINMAX, -1, Mat());

		return Histogram;
	}

	// --------------------------------------------------------------------------------------------------------------------------------
	// Letterbox Detection
	// --------------------------------------------------------------------------------------------------------------------------------

	CvPoint VideoAnalyzer::LetterboxDetection(IplImage *SourceImage, IplImage *OutputImage, int Threshold)
	{
		int					RowCount = SourceImage->height;
		int					RowIndex = 0;
		int					ColIndex = 0;
		int					x1 = 0;
		int					x2 = SourceImage->width - 1;
		int					Average = 0;
		int					AverageBuffer = 0;
		int					TopLine = 0;
		int					BottomLine = 0;
		int					AssumedBottom = 0;
		bool				FoundTop = false;
		bool				FoundBottom = false;
		CvPoint				ValueBuffer = cvPoint(0, 0);

		int 				MaxBuffer;
		CvLineIterator		iterator;
		long				LineSum[3];

		cvCopy(SourceImage, OutputImage, NULL);

		// Check all lines
		for (RowIndex = 0; RowIndex < RowCount; RowIndex++)
		{
			// Initialize sum buffer for each channel (RGB)
			LineSum[0] = 0;
			LineSum[1] = 0;
			LineSum[2] = 0;

			// Calculate how many pixels in line
			MaxBuffer = cvInitLineIterator(SourceImage, cvPoint(x1, RowIndex), cvPoint(x2, RowIndex), &iterator, 8, 0);

			// Walk thru the line pixel by pixel
			for (ColIndex = 0; ColIndex < MaxBuffer; ColIndex++)
			{
				// Sum color values for each channel (RGB)
				LineSum[0] = LineSum[0] + iterator.ptr[0];
				LineSum[1] = LineSum[1] + iterator.ptr[1];
				LineSum[2] = LineSum[2] + iterator.ptr[2];

				CV_NEXT_LINE_POINT(iterator); //Step to the next pixel
			}

			// Calculate average over all channels
			Average = (LineSum[0] / MaxBuffer + LineSum[1] / MaxBuffer + LineSum[2] / MaxBuffer) / 3;

			// No top letterbox found yet
			if (!FoundTop)
			{
				// Upper letterbox criteria
				if ((AverageBuffer <= Threshold) && (Average > Threshold))
				{
					// Remember last line which was under threshold
					TopLine = RowIndex - 1;
					FoundTop = true;
					// Calculate where the bottom letterbox should be if symmetric letterbox
					AssumedBottom = SourceImage->height - TopLine;
					// Store return value
				}
			}
			// Top letterbox found. Go and look for lower letterbox
			else
			{
				// Lower letterbox criteria
				if ((AverageBuffer > Threshold) && (Average <= Threshold))
				{
					// Remember line which was under threshold
					BottomLine = RowIndex;
					FoundBottom = true;
				}
			}
			AverageBuffer = Average;
		}

		if (FoundTop)
		{
			// Calculate how many pixels in line
			MaxBuffer = cvInitLineIterator(OutputImage, cvPoint(x1, TopLine), cvPoint(x2, TopLine), &iterator, 8, 0);

			// Walk thru the line pixel by pixel
			for (ColIndex = 0; ColIndex < MaxBuffer; ColIndex++)
			{
				// Mark line in red
				iterator.ptr[0] = 0;
				iterator.ptr[1] = 0;
				iterator.ptr[2] = 255;

				CV_NEXT_LINE_POINT(iterator); //Step to the next pixel
			}
		}

		if (FoundBottom)
		{
			// Calculate how many pixels in line
			MaxBuffer = cvInitLineIterator(OutputImage, cvPoint(x1, BottomLine), cvPoint(x2, BottomLine), &iterator, 8, 0);

			// Walk thru the line pixel by pixel
			for (ColIndex = 0; ColIndex < MaxBuffer; ColIndex++)
			{
				// Mark line in red
				iterator.ptr[0] = 0;
				iterator.ptr[1] = 0;
				iterator.ptr[2] = 255;

				CV_NEXT_LINE_POINT(iterator); //Step to the next pixel
			}
		}

		ValueBuffer = cvPoint(TopLine, BottomLine);

		return ValueBuffer;
	}

	// --------------------------------------------------------------------------------------------------------------------------------
	// Compute Average Difference between 2 frames
	// --------------------------------------------------------------------------------------------------------------------------------
	CvScalar VideoAnalyzer::AvgDifference(Mat &ProcessImage, Mat &BufferedImage, Mat &OutputImage)
	{
		CvScalar Average = cvScalar(0, 0, 0, 0);

		OutputImage = ProcessImage - BufferedImage;

		Average = mean(OutputImage);

		return Average;
	}

	// --------------------------------------------------------------------------------------------------------------------------------
	// Filter image using Laplace filter
	// --------------------------------------------------------------------------------------------------------------------------------
	void VideoAnalyzer::LaplaceFilter(IplImage* ProcessImage, IplImage* BufferedImage, IplImage* OutputImage)
	{

	}

	// --------------------------------------------------------------------------------------------------------------------------------
	// Filter image using Canny filter
	// --------------------------------------------------------------------------------------------------------------------------------
	void VideoAnalyzer::CannyFilter(Mat &ProcessImage, Mat &OutputImage)
	{
		cvtColor(ProcessImage, OutputImage, CV_BGR2GRAY, 0);

		cvSmooth(&OutputImage.operator IplImage(), &OutputImage.operator IplImage(), CV_GAUSSIAN, 5, 5);

		Canny(OutputImage, OutputImage, 50, 150, 3, true);
	}


	// --------------------------------------------------------------------------------------------------------------------------------
	// Scene Change Detection
	// --------------------------------------------------------------------------------------------------------------------------------
	unsigned int VideoAnalyzer::SceneChangeDetection(double* HistDiffValues, double NewHistDiffValue, unsigned int FrameCount)
	{
		bool			PreCondition = false;
		bool			PostCondition = false;

		unsigned int	EditFramePosition = 0;

		// Thresholds for edit detection
		double			EditThresholdHigh = 0.4;
		double			PreThresholdHigh = 0.1;
		double			PostThresholdHigh = 0.1;

		double			EditThresholdMid = 0.15;
		double			PreThresholdMid = 0.015;
		double			PostThresholdMid = 0.015;

		double			EditThresholdLow = 0.05;
		double			PreThresholdLow = 0.006;
		double			PostThresholdLow = 0.006;

		HistDiffValues[0] = HistDiffValues[1];
		HistDiffValues[1] = HistDiffValues[2];
		HistDiffValues[2] = HistDiffValues[3];
		HistDiffValues[3] = HistDiffValues[4];
		HistDiffValues[4] = NewHistDiffValue;

		// High histogram difference values
		if (HistDiffValues[2] > EditThresholdHigh)
		{
			if ((HistDiffValues[0] < PreThresholdHigh) && (HistDiffValues[1] < PreThresholdHigh))
				PreCondition = true;

			if ((HistDiffValues[3] < PostThresholdHigh) && (HistDiffValues[4] < PostThresholdHigh))
				PostCondition = true;
		}
		// Mid histogram difference values
		else if (HistDiffValues[2] > EditThresholdMid)
		{
			if ((HistDiffValues[0] < PreThresholdMid) && (HistDiffValues[1] < PreThresholdMid))
				PreCondition = true;

			if ((HistDiffValues[3] < PostThresholdMid) && (HistDiffValues[4] < PostThresholdMid))
				PostCondition = true;
		}
		// Low histogram difference values
		else if (HistDiffValues[2] > EditThresholdLow)
		{
			if ((HistDiffValues[0] < PreThresholdLow) && (HistDiffValues[1] < PreThresholdLow))
				PreCondition = true;

			if ((HistDiffValues[3] < PostThresholdLow) && (HistDiffValues[4] < PostThresholdLow))
				PostCondition = true;
		}

		if (PreCondition && PostCondition)
		{
			if (FrameCount >= 2)
				return FrameCount - 2;
			else
				return FrameCount;
		}
		else
			return 0;
	}

	// --------------------------------------------------------------------------------------------------------------------------------
	// Compute Average structural difference between 2 frames
	// --------------------------------------------------------------------------------------------------------------------------------
	CvScalar VideoAnalyzer::AvgStructureDiff(Mat &ProcessImage, Mat &BufferedImage, Mat &OutputImage)
	{
		Mat			FrameA;
		Mat			FrameB;

		CvScalar Average = cvScalar(0, 0, 0, 0);

		FrameA.create(ProcessImage.cols, ProcessImage.rows, CV_8UC1);
		FrameB.create(BufferedImage.cols, BufferedImage.rows, CV_8UC1);

		CannyFilter(ProcessImage, FrameA);
		CannyFilter(BufferedImage, FrameB);

		OutputImage = FrameA - FrameB;

		Average = mean(OutputImage);

		FrameA.release();
		FrameB.release();

		return Average;
	}

	// --------------------------------------------------------------------------------------------------------------------------------
	// SplitIntoFields
	// June 2014, Thomas Bause Mason
	//
	// Detect the cadence of a video clip by comparing two fields of a frame
	// 
	// Parameter:
	// SourceImage		= Input Source Image
	// EvenLineImage	= Output image with even lines
	// OddLineImage		= Output image with odd lines
	// --------------------------------------------------------------------------------------------------------------------------------
	void VideoAnalyzer::SplitIntoFields(Mat &SourceImage, Mat &EvenLineImage, Mat &OddLineImage)
	{
		int	LineCounter				= 0;											// Counts the lines while going thru the image
		int ColCounter				= 0;											// Counts the columns while going thru the image
		int RowCount				= SourceImage.rows;								// number of lines
		int ColCount				= SourceImage.cols;								// number of columns
		int ColByteCount			= SourceImage.cols * SourceImage.channels();	// number of bytes per line

		int EvenFieldLineCounter	= 0;											// Counts the lines for the image with the even lines
		int OddFieldLineCounter		= 0;											// Counts the lines for the image with the odd lines
		int KeyStroke				= 0;											// Value of the key pressed to continue 
		int WaitTime				= 0;											// Value in milliseconds for Wait function (0 = wait for key stroke)

		// --------------------------------- TEST OUTPUTS (START) ----------------------------------------------------------------
		//uchar	value = 0;													// !!!!!!!!!!!!!!!! TEST VALUE !!!!!!!!!!!!!!!!!!!
		//namedWindow("Even Lines", CV_WINDOW_AUTOSIZE);					// !!!!!!!!!!!!!!!! WINDOW FOR TEST DISPLAY 
		//namedWindow("Odd Lines", CV_WINDOW_AUTOSIZE);						// !!!!!!!!!!!!!!!! WINDOW FOR TEST DISPLAY
		//namedWindow("Source", CV_WINDOW_AUTOSIZE);						// !!!!!!!!!!!!!!!! WINDOW FOR TEST DISPLAY

		// TEST !!! DISPLAY SOURCE IMAGE
		//imshow( "Source", SourceImage );
		// TEST !!! WAIT FOR KEY STROKE
		//KeyStroke = cv::waitKey(WaitTime);
		// TEST !!! DESTROY TEST WINDOW
		//destroyWindow( "Source" );
		// --------------------------------- TEST OUTPUTS (END) ----------------------------------------------------------------


		// Go thru all lines
		for (LineCounter = 0; LineCounter < RowCount; LineCounter++)
		{
			// Get line address
			uchar* data = SourceImage.ptr<uchar>(LineCounter);

			// Go thru all columns
			// capture even lines
			if (LineCounter % 2 == 0)
			{
				uchar* EvenData = EvenLineImage.ptr<uchar>(EvenFieldLineCounter);

				for (ColCounter = 0; ColCounter < ColByteCount; ColCounter++)
				{
					EvenData[ColCounter] = data[ColCounter];
					// value = data[ColCounter];
				}

				EvenFieldLineCounter++;
			}
			// capture odd lines
			else
			{
				uchar* OddData = OddLineImage.ptr<uchar>(OddFieldLineCounter);

				for (ColCounter = 0; ColCounter < ColByteCount; ColCounter++)
					OddData[ColCounter] = data[ColCounter];

				OddFieldLineCounter++;
			}
		}

		// --------------------------------- TEST OUTPUTS (START) ----------------------------------------------------------------
		//if(!EvenLineImage.data)
		//	value = 0;
		//else
		//	imshow( "Even Lines", EvenLineImage );

		//if(!OddLineImage.data)
		//	value = 0;
		//else
		//	imshow( "Odd Lines", OddLineImage );

		//KeyStroke = cv::waitKey(WaitTime);

		//destroyWindow( "Even Lines" );
		//destroyWindow( "Odd Lines" );
		// --------------------------------- TEST OUTPUTS (END) ----------------------------------------------------------------

	}

	// --------------------------------------------------------------------------------------------------------------------------------
	// ContentAnalyzer
	// June 2014, Thomas Bause Mason
	//
	// Analyze video content based on various metrics
	// 
	// Parameter:
	// InFile		= Source file
	// OutFile		= Log file output
	// StartFrame	= Frame where cadence detector will start
	// Duration		= Duration in frames to analyze
	// Verbose		= Enable error/message logging and display
	// Display		= Display process images
	// --------------------------------------------------------------------------------------------------------------------------------
	bool VideoAnalyzer::ContentAnalyzer(string InFile, string OutFile, int StartFrame, int Duration, bool Verbose, bool Display)
	{
		// ---------------------------------------------------------------
		// NEW VARIBALES DECLARATION
		// ---------------------------------------------------------------
		// ---------------------------------------------------------------
		// Local Variables (Start)
		// ---------------------------------------------------------------

		// Dense arrays to hold video frame data
		Mat						ActiveFrame;									// The active (to process) frame from the video sequence
		Mat						BufferedFrame;									// The last frame processed before active frame

		MatND					HistogramActiveFrame;							// Histogram for active frame
		MatND					HistogramBufferedFrame;							// Histogram for buffered frame

		Mat						LBXDisplayFrame;								// An image showing the letterbox margin detected
		Mat						DifferenceDisplayFrame;							// An image showing the difference between active and buffered frame
		Mat						CannyDisplayFrame;								// An image showing the results after Canny filter 

		// Video sequence info
		VideoCapture			VideoSeqHandle;									// File handle for video sequence
		int						FrameWidth		= 0;							// Frame width of input image
		int						FrameHeight		= 0;							// Frame height of input image
		int						FrameCount		= 0;							// Frame count for video sequence
		double					FrameRate		= 0.0;							// Frame rate of video sequence
		Size					ImageSize;										// Image size as a scalar

		// Define all parameters for image overlay text
		ostringstream			OverlayText;									// String buffer for overlay text in image display
		int						TextFont		= FONT_HERSHEY_PLAIN;			// Define font for overlay text
		Scalar					FontColor		= cvScalar(255, 255, 255, 0);	// Define the text color
		double					FontScale		= 1.0;							// Define font scale
		int						FontThickness	= 1;							// Define font thickness
		int						FontLine		= 8;							// Define the line styl

		// Control parameters values for user input 
		int						WaitTime		= 0;							// Wait time in milli seconds for cvWait function
		int						EndKey			= 27;							// Key value to end playback loop (27 = ESC key)
		char					KeyStroke		= 0;							// store key stroke

		// Variables for time measurement
		clock_t					StartTime;										// Holds the start time of a process
		clock_t					EndTime;										// Holds the end time of a process
		double					TimeDif;										// Holds the duration of a process

		// Log file handling
		fstream					OutputFileHandler;								// File handler for output to log file

		// Variables for message logging
		CommonMessageHandler	CMHandler;										// Handles all messages to the application log file
		ostringstream			MessageBuffer;									// Buffer for a log file message

		// ---------------------------------------------------------------
		// OLD VARIBALES DECLARATION
		// ---------------------------------------------------------------
		// ---------------------------------------------------------------
		// Local Variables (Start)
		// ---------------------------------------------------------------


		// Process variables
		int				ProcessError = -1;				// stores the value of a process error 
		int				Counter = 0;					// Generic counter for loops
		int				FrameCounter = 0;				// Counter for how many frames are read from media file	

		// Pattern information
		Point			LetterboxValues = cvPoint(0, 0);
		bool			FirstFrame = true;
		Scalar			DifferenceAvgResults = cvScalar(0, 0, 0, 0);
		Scalar			EnergyResults = cvScalar(0, 0, 0, 0);
		Scalar			StructDiffAvgResults = cvScalar(0, 0, 0, 0);
		double			HistDiffBuffer[5] = { 0, 0, 0, 0, 0 };
		int				EditListSignal = 0;
		unsigned int	EditPosition = 0;

		// Histogram variables
		int				h_bins = 60;
		int				s_bins = 64;
		double			HistogramCompareResult = 0;

		// ---------------------------------------------------------------
		// Local Variables (End)
		// ---------------------------------------------------------------


		// Clear buffer
		MessageBuffer.str("");
		MessageBuffer << "Content Analyzer Method = Started analyzing!" << endl;
		CMHandler.SaveLogEntry(VW_LOG_FILENAME, "MESSAGE", MessageBuffer.str());

		// Open the log file to store pattern values and output headers
		OutputFileHandler.open(OutFile, ios::out | ios::trunc | ios::binary);
		OutputFileHandler << "Frame#;Struct. Diff Ch.1;Struct. Diff Ch.2;Struct. Diff Ch.3;Struct. Diff Ch.4;Diff Ch.1;Diff Ch.2;Diff Ch.3;Diff Ch.4;Eng. Ch.1;Eng. Ch.2;Eng. Ch.3;Eng. Ch.4;Hist Comp;LBX X;LBX Y;EDL;Time" << endl;

		// Open stored video sequence and report error
		if(!VideoSeqHandle.open(InFile))
			// ERROR: Can't open video file
			ProcessError = 1;
		else
		{
			// Get full frame width and height as well as frame count and frame rate
			FrameWidth	= (int)VideoSeqHandle.get(CV_CAP_PROP_FRAME_WIDTH);
			FrameHeight = (int)VideoSeqHandle.get(CV_CAP_PROP_FRAME_HEIGHT);
			FrameCount	= (int)VideoSeqHandle.get(CV_CAP_PROP_FRAME_COUNT);
			FrameRate	= VideoSeqHandle.get(CV_CAP_PROP_FPS);
			// Position on start frame of video sequence
			if ((StartFrame < FrameCount) && ((StartFrame + Duration) <= FrameCount))
				VideoSeqHandle.set(CV_CAP_PROP_POS_FRAMES, (double)StartFrame);
			else
				// ERROR: Start frame or duration illegal
				ProcessError = 2;
		}

		// Initialize process variables if there is no error
		if (ProcessError < 0)
		{
			// Get field width and height
			ImageSize.width = FrameWidth;
			ImageSize.height = FrameHeight / 2;

			// Initial histogram variables
			// HistogramActiveFrame.create(h_bins * scale, s_bins * scale, ActiveFrame.type());
			// HistogramBufferedFrame.create(h_bins * scale, s_bins * scale, ActiveFrame.type());
			HistogramActiveFrame.create(h_bins, s_bins, ActiveFrame.type());
			HistogramBufferedFrame.create(h_bins, s_bins, ActiveFrame.type());
		}

		// Process frame by frame till ESC key was pressed
		while ((KeyStroke != EndKey) && (ProcessError < 0) && (FrameCounter < Duration))
		{
			// read next frame if any are available
			VideoSeqHandle >> ActiveFrame;
			if (!ActiveFrame.data)
			{
				// Handle read error
				if (FrameCounter == Duration)
					// End of file reached
					ProcessError = 0;
				else
					// Error reading file
					ProcessError = 3;
			}
			else
				// next frame
				FrameCounter++;

			if (Verbose)
			{
				// output to log file
				MessageBuffer.str("");
				MessageBuffer << "Frame: " << FrameCounter << ", Process Error: " << ProcessError << endl;
				CMHandler.SaveLogEntry(VW_LOG_FILENAME, "MESSAGE", MessageBuffer.str());

				// output to console
				cout << "Frame: " << FrameCounter << ", Process Error: " << ProcessError << endl;
			}

			// Process frames starting with Startframe till duration is met
			if (ProcessError < 0)
			{
				// Initial process variables using the first frame
				if (FirstFrame)
				{
					LBXDisplayFrame.create(FrameHeight, FrameWidth, ActiveFrame.type());
					BufferedFrame.create(FrameHeight, FrameWidth, ActiveFrame.type());
					DifferenceDisplayFrame.create(FrameHeight, FrameWidth, ActiveFrame.type());
					CannyDisplayFrame.create(FrameHeight, FrameWidth, CV_8UC1);

				}

				// remember process start time
				StartTime = clock();

				// Calculate histogram 
				HistogramActiveFrame = CalculateHSVHistogram(ActiveFrame, h_bins, s_bins, 180, 256);

				// Buffered operations. Exclude first frame.
				if (!FirstFrame)
				{
					// Calculate histogram difference (buffered vs. active histogram). 
					HistogramCompareResult = compareHist(HistogramActiveFrame, HistogramBufferedFrame, CV_COMP_CHISQR);

					EditPosition = SceneChangeDetection(HistDiffBuffer, HistogramCompareResult, FrameCounter);

					if (EditPosition > 0)
						EditListSignal = 1;
					else
						EditListSignal = 0;

					// Calculate average difference between frames
					DifferenceAvgResults = AvgDifference(ActiveFrame, BufferedFrame, DifferenceDisplayFrame);
					// Calculate average difference in structure between frames
					StructDiffAvgResults = AvgStructureDiff(ActiveFrame, BufferedFrame, CannyDisplayFrame);
				}
				else
					FirstFrame = false;

				// Save histogram data for calculation with next frame
				HistogramActiveFrame.copyTo(HistogramBufferedFrame);

				// Calculate letterbox values
				LetterboxValues = LetterboxDetection(&ActiveFrame.operator IplImage(), &LBXDisplayFrame.operator IplImage(), 0);
				// Calculate average energy contents of frame
				EnergyResults = cvAvg(&ActiveFrame.operator IplImage(), NULL);

				// remember process end time
				EndTime = clock();

				ActiveFrame.copyTo(BufferedFrame);

				// --------------------------------- TEST OUTPUTS (START) ----------------------------------------------------------------
				if (Display)
				{
					OverlayText.str("");
					OverlayText << "Frame: " << FrameCounter << endl;
					putText(ActiveFrame, OverlayText.str(), cvPoint(10, 460), TextFont, FontScale, FontColor, FontThickness, FontLine, false);

					// Create output windows
					namedWindow("Active Frame", CV_WINDOW_AUTOSIZE);
					// cvNamedWindow( "Letterbox", CV_WINDOW_AUTOSIZE );
					// cvNamedWindow( "Active Frame Histogram", CV_WINDOW_AUTOSIZE );
					// cvNamedWindow( "Frame Difference", CV_WINDOW_AUTOSIZE );
					namedWindow("Canny Filter", CV_WINDOW_AUTOSIZE);

					// Display source frame
					imshow("Active Frame", ActiveFrame);
					// Display Letterbox
					// cvShowImage( "Letterbox", LBXDisplayFrame );
					// Display Letterbox
					// cvShowImage( "Active Frame Histogram", HistogramActiveFrame );
					// Display Frame Difference
					// cvShowImage( "Frame Difference", DifferenceDisplayFrame );
					// Display Canny filter output
					imshow("Canny Filter", CannyDisplayFrame);
					// Wait 42ms (frame rate = 24 frames a sec.)
					KeyStroke = waitKey(WaitTime);

					// Destroy all test windows
					destroyWindow("Active Frame");
					// cvDestroyWindow( "Letterbox" );
					// cvDestroyWindow( "Active Frame Histogram" );
					// cvDestroyWindow("Frame Difference");
					destroyWindow("Canny Filter");
				}
				// --------------------------------- TEST OUTPUTS (END) ----------------------------------------------------------------

				// Caclulate processing time for one frame
				TimeDif = (double) diffclock(EndTime, StartTime);

				// Output line of values (one frame) to log file
				OutputFileHandler << FrameCounter << ';'
					<< setprecision(3) << StructDiffAvgResults.val[0] << ';'
					<< setprecision(3) << StructDiffAvgResults.val[1] << ';'
					<< setprecision(3) << StructDiffAvgResults.val[2] << ';'
					<< setprecision(3) << StructDiffAvgResults.val[3] << ';'
					<< setprecision(3) << DifferenceAvgResults.val[0] << ';'
					<< setprecision(3) << DifferenceAvgResults.val[1] << ';'
					<< setprecision(3) << DifferenceAvgResults.val[2] << ';'
					<< setprecision(3) << DifferenceAvgResults.val[3] << ';'
					<< setprecision(3) << EnergyResults.val[0] << ';'
					<< setprecision(3) << EnergyResults.val[1] << ';'
					<< setprecision(3) << EnergyResults.val[2] << ';'
					<< setprecision(3) << EnergyResults.val[3] << ';'
					<< HistogramCompareResult << ';'
					<< LetterboxValues.x << ';'
					<< LetterboxValues.y << ';'
					<< EditListSignal << ';'
					<< setprecision(6) << TimeDif << endl;
			}
		}
		// CLEAN UP ---------------------------------------------------------------------------------------------
		// Close log files
		OutputFileHandler.close();

		// Release source
		VideoSeqHandle.release();

		// Release memory for images
		// cvReleaseImage(&ActiveFrame);
		LBXDisplayFrame.release();
		HistogramActiveFrame.release();
		DifferenceDisplayFrame.release();
		CannyDisplayFrame.release();
		// CLEAN UP ---------------------------------------------------------------------------------------------

		// Deal with Errors -------------------------------------------------------------------------------------
		if (ProcessError > 0)
		{
			// Clear buffer
			MessageBuffer.clear();

			switch (ProcessError)
			{
			// ERROR: Can't open video file
			case 1: MessageBuffer << "Content Analyzer Method = Can't open video sequence!" << endl;
				break;
			// ERROR: Start frame or duration illegal
			case 2: MessageBuffer << "Content Analyzer Method = Start frame or duration illegal!" << endl;
				break;
			// ERROR: Can't read file
			case 3: MessageBuffer << "Content Analyzer Method = Can't read frame from video sequence!" << endl;
				break;
			default: MessageBuffer << "Content Analyzer Method = Unknown error!" << endl;

			}
			CMHandler.SaveLogEntry(VW_LOG_FILENAME, "ERROR", MessageBuffer.str());

			if (Verbose)
				cout << "ERROR: " << MessageBuffer.str() << endl;

			return false;
		}
		else
		{
			// Clear buffer
			MessageBuffer.str("");
			MessageBuffer << "Content Analyzer Method = Finished analyzing (status = normal)!" << endl;
			CMHandler.SaveLogEntry(VW_LOG_FILENAME, "MESSAGE", MessageBuffer.str());

			if (Verbose)
			{
				// Clear buffer
				MessageBuffer.str("");
				MessageBuffer << "Content Analyzer Method = Finished analyzing (status = normal)!" << endl;
				cout << "ERROR: " << MessageBuffer.str() << endl;
			}

			return true;
		}
	}

	// --------------------------------------------------------------------------------------------------------------------------------
	// CadenceDetector
	// June 2014, Thomas Bause Mason
	//
	// Detect the cadence of a video clip by comparing two fields of a frame
	// 
	// Parameter:
	// InFile		= Source file
	// OutFile		= Log file output
	// StartFrame	= Frame where cadence detector will start
	// Duration		= Duration in frames to analyze
	// Verbose		= Enable error/message logging and display
	// Display		= Display process images
	// --------------------------------------------------------------------------------------------------------------------------------
	bool VideoAnalyzer::CadenceDetector(string InFile, string OutFile, int StartFrame, int Duration, bool Verbose, bool Display)
	{
		CommonMessageHandler	MessageHandler;
		CommonToolBox			ToolBox;
		//ColorToolBox			ColorTools;

		// Usage parameter
		int				WaitTime = 0;					// Wait time in milli seconds for cvWait function
		int				EndKey = 27;					// Key value to end playback loop (27 = ESC key)
		char			KeyStroke = 0;					// store key stroke

		// Process variables
		bool			ProcessError = false;			// store process error accured 
		int				Counter = 0;					// Generic counter for loops
		int				FrameCounter = 0;				// Counter for how many frames are read from media file	

		// Image variables
		Mat				FrameBuffer;					// buffered video frames for processing
		Mat				Field1Return;					// buffer of processed video frame (Field 1)
		Mat				Field2Return;					// buffer of processed video frame (Field 2)
		Mat				OutputImage;					// buffer of video frame differences (Difference: Field 1 - Field 2)

		fstream			OutputFileHandler;				// File handler for output to log file

		int				FrameWidth = 0;					// Frame width of input image
		int				FrameHeight = 0;				// Frame height of input image
		CvSize			ImageSize;						// Image size as a scalar

		double			AverageTotal = 0;				// The total average intensite in frame with difference 

		queue<double>	ValueBuffer;					// Buffer for a sequence of difference values

		string			ProcessMessage = "Undefined";	// String buffer for process message

		// Open media file
		VideoCapture capture(InFile);

		// Open log file
		OutputFileHandler.open(OutFile, ios::out | ios::trunc | ios::binary);

		// Get full frame width and height
		FrameWidth = (int)capture.get(CV_CAP_PROP_FRAME_WIDTH);
		FrameHeight = (int)capture.get(CV_CAP_PROP_FRAME_HEIGHT);

		// Get field width and height
		ImageSize.width = FrameWidth;
		ImageSize.height = FrameHeight / 2;

		// Go thru file till ESC key was pressed or process error
		while ((KeyStroke != EndKey) && (!ProcessError))
		{
			// read next frame if any are available
			if (!capture.read(FrameBuffer))
				// end of file or read error
				ProcessError = true;
			else
			{
				// next frame
				FrameCounter++;
			}
			// Process frames starting with Startframe till duration is met
			if (!ProcessError && ((FrameCounter >= StartFrame) && (FrameCounter < (Duration + StartFrame))))
			{
				// Create field and output image buffers
				Field1Return.create((FrameHeight / 2), FrameWidth, FrameBuffer.type());
				Field2Return.create((FrameHeight / 2), FrameWidth, FrameBuffer.type());
				OutputImage.create((FrameHeight / 2), FrameWidth, FrameBuffer.type());

				// Split input frame into fields
				SplitIntoFields(FrameBuffer, Field1Return, Field2Return);

				// Initialize 
				CvScalar AverageSet = cvScalar(0, 0, 0, 0);

				// Subtract field 2 from field 1 and save resulting image
				OutputImage = Field1Return - Field2Return;

				// --------------------------------- TEST OUTPUTS (START) ----------------------------------------------------------------
				if (Display)
				{
					namedWindow("FrameBuffer", CV_WINDOW_AUTOSIZE);					// !!!!!!!!!!!!!!!! WINDOW FOR TEST DISPLAY 
					namedWindow("Field1Return", CV_WINDOW_AUTOSIZE);					// !!!!!!!!!!!!!!!! WINDOW FOR TEST DISPLAY 
					namedWindow("Field2Return", CV_WINDOW_AUTOSIZE);					// !!!!!!!!!!!!!!!! WINDOW FOR TEST DISPLAY 
					namedWindow("OutputImage", CV_WINDOW_AUTOSIZE);						// !!!!!!!!!!!!!!!! WINDOW FOR TEST DISPLAY 

					imshow("FrameBuffer", FrameBuffer);
					imshow("Field1Return", Field1Return);
					imshow("Field2Return", Field2Return);
					imshow("OutputImage", OutputImage);
					KeyStroke = cv::waitKey(WaitTime);

					destroyWindow("FrameBuffer");
					destroyWindow("Field1Return");
					destroyWindow("Field2Return");
					destroyWindow("OutputImage");
				}
				// --------------------------------- TEST OUTPUTS (END) ----------------------------------------------------------------

				// Caclulate average intensity across the resulting image for each channel
				AverageSet = cvAvg(&OutputImage.operator IplImage(), NULL);

				// Calculate average from average of each image channel and round
				AverageTotal = round((AverageSet.val[0] + AverageSet.val[1] + AverageSet.val[2]) / 3);

				// OutputFileHandler << "Frame: " << FrameCounter << " - " << AverageTotal << endl;
				OutputFileHandler << FrameCounter << ';' << AverageTotal << endl;

				// ValueBuffer.push(AverageTotal);

				// Output frame count to console and log values to log file
				if (Verbose)
				{
					cout << "Frame: " << FrameCounter << endl;

					// TEST MESSAGE OUTPUT FOR LOG FILE !!!!!!!!!!!!!!!!!!!
					ProcessMessage.clear();
					ProcessMessage.append("Frame: ");
					ProcessMessage.append(ToolBox.ConvertIntToString(FrameCounter));
					MessageHandler.SaveLogEntry(VW_LOG_FILENAME, "MESSAGE: ", ProcessMessage);
				}
			}
		}
		// Close handle to log file
		OutputFileHandler.close();

		return true;
	}



// } // Namespace end