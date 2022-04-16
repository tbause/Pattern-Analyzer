// VideoProcessor.cpp : Defines the processor class and its methods and properties
//

#include "stdafx.h"

using namespace std;
using namespace cv;

void VideoProcessor::salt(Mat &image, int n)
{
	for (int k=0; k<n; k++) 
	{
		// rand() is the MFC random number generator
		// try qrand() with Qt
		int i= rand()%image.cols;
		int j= rand()%image.rows;

		if (image.channels() == 1) 
		{ // gray-level image
			image.at<uchar>(j,i)= 255;
		} 
		else if (image.channels() == 3) 
		{ // color image
			image.at<cv::Vec3b>(j,i)[0]= 255;
			image.at<cv::Vec3b>(j,i)[1]= 255;
			image.at<cv::Vec3b>(j,i)[2]= 255;
		}
	}

}


void VideoProcessor::HandOverTest(Mat &TestImage1, Mat &TestImage2)
{
	namedWindow("Test Image 1", CV_WINDOW_AUTOSIZE);
	namedWindow("Test Image 2", CV_WINDOW_AUTOSIZE);

	TestImage1.copyTo(TestImage2);


	imshow( "Test Image 1", TestImage1 );
	imshow( "Test Image 2", TestImage2 );

	cv::waitKey(0);

	destroyWindow( "Test Image 1" );
	destroyWindow( "Test Image 2" );
}


bool VideoProcessor::InterlaceFile(string InFile, string OutFile, int StartFrame, int Duration, int ImageWidth, int ImageHeight)
{
	CommonMessageHandler				MessageHandler;
	CommonToolBox						ToolBox;
	YUVFileHandling						YUVFileAccess;
	
	string			ProcessMessage		= "Undefined";

	fstream			OutputFileHandler;
	fstream			InputFileHandler;
	
	// Usage parameter
	int				WaitTime			= 0;					// Wait time in milli seconds for cvWait function
	int				EndKey				= 27;					// Key value to end playback loop (27 = ESC key)

	// Process variables
	char			KeyStroke			= 0;					// store key stroke
	bool			ProcessError		= false;				// store process error accured 
	int				Counter				= 0;					// Generic counter for loops
	unsigned int	FrameCounter		= 0;					// Counter for how many frames are read from media file	

    CvSize ImageSize;

	ImageSize.width  = ImageWidth;
    ImageSize.height = ImageHeight * 2;

	// Image variables
	Mat				FrameReturn(ImageSize.height, ImageSize.width, CV_8SC3, Scalar(0,0,0));						// buffered video frames for processing
	Mat				Field1Buffer(ImageHeight, ImageWidth, CV_8UC3, Scalar(0,0,0));								// buffer of processed video frame (results)
	Mat				Field2Buffer(ImageHeight, ImageWidth, CV_8UC3, Scalar(0,0,0));								// buffer of processed video frame (results)
	Mat				OutputBuffer(ImageHeight, ImageWidth, CV_8UC3, Scalar(0,0,0));								// buffer of processed video frame (results)

	// VideoWriter VWriter;

	OutputFileHandler.open(OutFile, ios::out | ios::trunc | ios::binary);
	InputFileHandler.open(InFile, ios::in | ios::binary);

	// ToolBox.TestDisplayImage("Test Image", Field1Buffer);

	// Go thru file till ESC key was pressed or process error
	while((KeyStroke != EndKey) && (!ProcessError))
	{
		// read next frame if any are available
		if (!YUVFileAccess.ReadYUVImage_444Planar(InputFileHandler, Field1Buffer, ImageWidth, ImageHeight, CV_8UC2))
			ProcessError = true;

		// read another frame if any are available
		if (!YUVFileAccess.ReadYUVImage_444Planar(InputFileHandler, Field2Buffer, ImageWidth, ImageHeight, CV_8UC2))
			ProcessError = true;

		// Process frames
		if(!ProcessError)
		{
			FrameCounter++;

			// Analyze the frames
			CombineIntoFrame(Field1Buffer, Field2Buffer, FrameReturn);

			// --------------------------------- TEST OUTPUTS (START) ----------------------------------------------------------------
			//namedWindow("Even Lines", CV_WINDOW_AUTOSIZE);					// !!!!!!!!!!!!!!!! WINDOW FOR TEST DISPLAY 
			//namedWindow("Odd Lines", CV_WINDOW_AUTOSIZE);					// !!!!!!!!!!!!!!!! WINDOW FOR TEST DISPLAY

			//imshow( "Even Lines", Field1Return );
			//imshow( "Odd Lines", Field2Return );

			//KeyStroke = cv::waitKey(WaitTime);

			//destroyWindow( "Even Lines" );
			//destroyWindow( "Odd Lines" );
			// --------------------------------- TEST OUTPUTS (END) ----------------------------------------------------------------

			// VWriter.write(FrameReturn);      // add the frame to the file

			// TEST MESSAGE OUTPUT FOR LOG FILE !!!!!!!!!!!!!!!!!!!
			ProcessMessage.clear();
			ProcessMessage.append("Frame: ");
			ProcessMessage.append(ToolBox.ConvertIntToString(FrameCounter));
			MessageHandler.SaveLogEntry(VW_LOG_FILENAME, "MESSAGE: ", ProcessMessage);

			cvtColor(FrameReturn, OutputBuffer, CV_BGR2YUV);

			YUVFileAccess.WriteYUVImage_444Planar(OutputFileHandler, OutputBuffer);
		}
	}

	if(ProcessError)
		MessageHandler.SaveLogEntry(VW_LOG_FILENAME, "ERROR: ", "Process error in function \"interlace\"");

	OutputFileHandler.close();

	// VWriter.release();

	return true;
}

bool VideoProcessor::DeInterlaceFile(string InFile, string OutFile, int StartFrame, int Duration)
{
	CommonMessageHandler	MessageHandler;
	CommonToolBox			ToolBox;
	ColorToolBox			ColorTools;

	// Usage parameter
	int				WaitTime			= 0;					// Wait time in milli seconds for cvWait function
	int				EndKey				= 27;					// Key value to end playback loop (27 = ESC key)

	// Process variables
	char			KeyStroke			= 0;					// store key stroke
	bool			ProcessError		= false;				// store process error accured 
	int				Counter				= 0;					// Generic counter for loops
	int				FrameCounter		= 0;					// Counter for how many frames are read from media file	

	// Image variables
	Mat				FrameBuffer;								// buffered video frames for processing
	Mat				Field1Return;								// buffer of processed video frame (results)
	Mat				Field2Return;								// buffer of processed video frame (results)
	Mat				Field1ReturnYUV;							// buffer of processed video frame (results)
	Mat				Field2ReturnYUV;							// buffer of processed video frame (results)

	// VideoWriter		VWriter;
	YUVFileHandling YUVWriter;

	fstream			OutputFileHandler;

    int				FrameWidth			= 0;
    int				FrameHeight			= 0;
    int				TargetFrameRate		= 0;  
	int				SourceFrameRate		= 0;

    CvSize			ImageSize;

	// Open media file
	VideoCapture capture(InFile);

	OutputFileHandler.open(OutFile, ios::out | ios::trunc | ios::binary);

	string			ProcessMessage		= "Undefined";

	FrameWidth  = (int) capture.get(CV_CAP_PROP_FRAME_WIDTH);
	FrameHeight = (int) capture.get(CV_CAP_PROP_FRAME_HEIGHT);

	ImageSize.width  = FrameWidth;
    ImageSize.height = FrameHeight/2;

	// Get the frame rate
	SourceFrameRate = (int) capture.get(CV_CAP_PROP_FPS) * 100;
	TargetFrameRate = SourceFrameRate * 2;

	// Open file for writing
	// VWriter.open(OutFile, CV_FOURCC('M','J','P','G'), 59.94, ImageSize);
	// VWriter.open(OutFile, CV_FOURCC('U','Y','V','Y'), 59.94, ImageSize);

	// Go thru file till ESC key was pressed or process error
	while((KeyStroke != EndKey) && (!ProcessError))
	{
		// read next frame if any are available
		if (!capture.read(FrameBuffer))
			ProcessError = true;
		else
		{
			FrameCounter++;
		}
		// Process frames starting with Startframe till duration is met
		if(!ProcessError && ((FrameCounter >= StartFrame) && (FrameCounter < (Duration + StartFrame))))
		{
			Field1Return.create((FrameHeight/2), FrameWidth, FrameBuffer.type());
			Field2Return.create((FrameHeight/2), FrameWidth, FrameBuffer.type());
			Field1ReturnYUV.create((FrameHeight/2), FrameWidth, FrameBuffer.type());
			Field2ReturnYUV.create((FrameHeight/2), FrameWidth, FrameBuffer.type());

			// Analyze the frames
			SplitIntoFields(FrameBuffer, Field1Return, Field2Return);

			// --------------------------------- TEST OUTPUTS (START) ----------------------------------------------------------------
			//namedWindow("Even Lines", CV_WINDOW_AUTOSIZE);					// !!!!!!!!!!!!!!!! WINDOW FOR TEST DISPLAY 
			//namedWindow("Odd Lines", CV_WINDOW_AUTOSIZE);					// !!!!!!!!!!!!!!!! WINDOW FOR TEST DISPLAY

			//imshow( "Even Lines", Field1Return );
			//imshow( "Odd Lines", Field2Return );

			//KeyStroke = cv::waitKey(WaitTime);

			//destroyWindow( "Even Lines" );
			//destroyWindow( "Odd Lines" );
			// --------------------------------- TEST OUTPUTS (END) ----------------------------------------------------------------

			// VWriter.write(Field1Return);      // add the frame to the file
			// VWriter.write(Field2Return);      // add the frame to the file

			cout << "Frame: " << FrameCounter << endl;

			// TEST MESSAGE OUTPUT FOR LOG FILE !!!!!!!!!!!!!!!!!!!
			ProcessMessage.clear();
			ProcessMessage.append("Frame: ");
			ProcessMessage.append(ToolBox.ConvertIntToString(FrameCounter));
			MessageHandler.SaveLogEntry(VW_LOG_FILENAME, "MESSAGE: ", ProcessMessage);

			ColorTools.ConvertRGBToYUV(Field1Return, Field1ReturnYUV);
			ColorTools.ConvertRGBToYUV(Field2Return, Field2ReturnYUV);

			// --------------------------------- TEST OUTPUTS (START) ----------------------------------------------------------------
			//namedWindow("Even Lines YUV", CV_WINDOW_AUTOSIZE);					// !!!!!!!!!!!!!!!! WINDOW FOR TEST DISPLAY 
			//namedWindow("Odd Lines YUV", CV_WINDOW_AUTOSIZE);					// !!!!!!!!!!!!!!!! WINDOW FOR TEST DISPLAY

			//imshow( "Even Lines YUV", Field1ReturnYUV );
			//imshow( "Odd Lines YUV", Field2ReturnYUV );

			//KeyStroke = cv::waitKey(WaitTime);

			//destroyWindow( "Even Lines YUV" );
			//destroyWindow( "Odd Lines YUV" );
			// --------------------------------- TEST OUTPUTS (END) ----------------------------------------------------------------


			// Swapped Field2Return and Field1Return for Bottom Field First
			YUVWriter.WriteYUVImage_422Planar(OutputFileHandler, Field2ReturnYUV);
			YUVWriter.WriteYUVImage_422Planar(OutputFileHandler, Field1ReturnYUV);
			//YUVWriter.WriteYUVImage_444Planar(OutputFileHandler, Field2ReturnYUV);
			//YUVWriter.WriteYUVImage_444Planar(OutputFileHandler, Field1ReturnYUV);
			//YUVWriter.WriteYUVImage_BGR444P(OutputFileHandler, Field2Return);
			//YUVWriter.WriteYUVImage_BGR444P(OutputFileHandler, Field1Return);

		}
	}
	OutputFileHandler.close();

	// VWriter.release();

	return true;
}

void VideoProcessor::SplitIntoFields(Mat &SourceImage, Mat &EvenLineImage, Mat &OddLineImage)
{
	int	LineCounter		= 0;											// Counts the lines while going thru the image
	int ColCounter		= 0;											// Counts the columns while going thru the image
	int RowCount		= SourceImage.rows;								// number of lines
	int ColCount		= SourceImage.cols;								// number of columns
	int ColByteCount	= SourceImage.cols * SourceImage.channels();	// number of bytes per line

	int EvenFieldLineCounter = 0;										// Counts the lines for the image with the even lines
	int OddFieldLineCounter = 0;										// Counts the lines for the image with the odd lines
	int KeyStroke = 0;													// Value of the key pressed to continue 
	int WaitTime = 0;													// Value in milliseconds for Wait function (0 = wait for key stroke)

	// --------------------------------- TEST OUTPUTS (START) ----------------------------------------------------------------
	 //uchar	value = 0;												// !!!!!!!!!!!!!!!! TEST VALUE !!!!!!!!!!!!!!!!!!!
	 //namedWindow("Even Lines", CV_WINDOW_AUTOSIZE);					// !!!!!!!!!!!!!!!! WINDOW FOR TEST DISPLAY 
	 //namedWindow("Odd Lines", CV_WINDOW_AUTOSIZE);					// !!!!!!!!!!!!!!!! WINDOW FOR TEST DISPLAY
	 //namedWindow("Source", CV_WINDOW_AUTOSIZE);						// !!!!!!!!!!!!!!!! WINDOW FOR TEST DISPLAY

	// TEST !!! DISPLAY SOURCE IMAGE
     //imshow( "Source", SourceImage );
	// TEST !!! WAIT FOR KEY STROKE
	 //KeyStroke = cv::waitKey(WaitTime);
	// TEST !!! DESTROY TEST WINDOW
	 //destroyWindow( "Source" );
	// --------------------------------- TEST OUTPUTS (END) ----------------------------------------------------------------


	// Go thru all lines
	for (LineCounter=0; LineCounter<RowCount; LineCounter++) 
	{
		// Get line address
		uchar* data= SourceImage.ptr<uchar>(LineCounter);

		// Go thru all columns
		// capture even lines
		if(LineCounter % 2 == 0)
		{
			uchar* EvenData= EvenLineImage.ptr<uchar>(EvenFieldLineCounter);

			for (ColCounter=0; ColCounter<ColByteCount; ColCounter++)
			{
				EvenData[ColCounter] = data[ColCounter];
				// value = data[ColCounter];
			}

			EvenFieldLineCounter++;
		}
		// capture odd lines
		else
		{
			uchar* OddData= OddLineImage.ptr<uchar>(OddFieldLineCounter);

			for (ColCounter=0; ColCounter<ColByteCount; ColCounter++) 
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

void VideoProcessor::CombineIntoFrame(Mat &EvenLineImage, Mat &OddLineImage, Mat &TargetFrame)
{
	int	FieldLineCounter	= 0;												// Counts the lines while going thru the image
	int ColCounter			= 0;												// Counts the columns while going thru the image
	int RowCount			= EvenLineImage.rows;								// number of lines
	int ColCount			= EvenLineImage.cols;								// number of columns
	int ColByteCount		= EvenLineImage.cols * EvenLineImage.channels();	// number of bytes per line

	int FrameLineCounter	= 0;												// Counts the lines for the image with the even lines
	int KeyStroke			= 0;												// Value of the key pressed to continue 
	int WaitTime			= 0;												// Value in milliseconds for Wait function (0 = wait for key stroke)

	// Go thru all lines of the field frames
	for (FieldLineCounter=0; FieldLineCounter<RowCount; FieldLineCounter++) 
	{
		// ----------------------------------------------------------------------------------
		// write even line to frame
		// ----------------------------------------------------------------------------------
		// Get pointer to line of target frame
		uchar* TargetDataEven = TargetFrame.ptr<uchar>(FrameLineCounter);

		// Get pointer to line of even field frame
		uchar* EvenData = EvenLineImage.ptr<uchar>(FieldLineCounter);

		// Copy pixel information
		for (ColCounter=0; ColCounter<ColByteCount; ColCounter++) 
			TargetDataEven[ColCounter] = EvenData[ColCounter];

		// Go to next line in target frame
		FrameLineCounter++;

		// ----------------------------------------------------------------------------------
		// write odd line to frame
		// ----------------------------------------------------------------------------------
		// Get pointer to line of target frame
		uchar* TargetDataOdd = TargetFrame.ptr<uchar>(FrameLineCounter);

		// Get pointer to line of target frame
		uchar* OddData = OddLineImage.ptr<uchar>(FieldLineCounter);

		// Copy pixel information
		for (ColCounter=0; ColCounter<ColByteCount; ColCounter++) 
			TargetDataOdd[ColCounter] = OddData[ColCounter];

		// Go to next line in target frame
		FrameLineCounter++;
	}

	// --------------------------------- TEST OUTPUTS (START) ----------------------------------------------------------------
	//namedWindow("Source (Field 1)", CV_WINDOW_AUTOSIZE);				// !!!!!!!!!!!!!!!! WINDOW FOR TEST DISPLAY 
	//namedWindow("Source (Field 2)", CV_WINDOW_AUTOSIZE);				// !!!!!!!!!!!!!!!! WINDOW FOR TEST DISPLAY
	//namedWindow("Target Frame", CV_WINDOW_AUTOSIZE);					// !!!!!!!!!!!!!!!! WINDOW FOR TEST DISPLAY

	//imshow( "Source (Field 1)", EvenLineImage );
	//imshow( "Source (Field 2)", OddLineImage );
	//imshow( "Target Frame", TargetFrame );

	//KeyStroke = cv::waitKey(WaitTime);

	//destroyWindow( "Target Frame" );
	//destroyWindow( "Source (Field 1)" );
	//destroyWindow( "Source (Field 2)" );
	// --------------------------------- TEST OUTPUTS (END) ----------------------------------------------------------------

}