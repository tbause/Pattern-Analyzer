// YUVFileHandling.cpp : Defines the YUV File Handling class and its methods and properties
//
// Soruce code taken from: http://tech.groups.yahoo.com/group/OpenCV/message/59027
//

#include "stdafx.h"

using namespace std;
using namespace cv;

// ----------------------------------------------------------------------------------------------------------------
// Save a MAT structure to a YUV file with 4:2:2 Color Subsampling
//
// ----------------------------------------------------------------------------------------------------------------
void YUVFileHandling::WriteYUVImage_422Planar(fstream &TestFileHandle, Mat &ImageBuffer)
{
	CommonMessageHandler	MessageHandler;
	CommonToolBox			ToolBox;

	string					ProcessMessage = "Undefined";

	int		ImageHeight		= ImageBuffer.rows;
	int		ImageWidth		= ImageBuffer.cols;
	int		ImageChannels	= ImageBuffer.channels();
	int		ByteCounter		= 0;
	int		ChannelCounter	= 0;
	int		PixelCounter	= 0;

	//Mat		MonoBuffer(ImageHeight, ImageWidth, CV_8UC1);

	CvSize	ImageSize;

	int		Pixel1 = 0;
	int		Pixel2 = 0;

	ImageSize.height		= ImageHeight;
	ImageSize.width			= ImageWidth;

	ProcessMessage.clear();
	ProcessMessage.append("Wrote YUV Image (4:2:2 Planar). Resolution: ");
	ProcessMessage.append(ToolBox.ConvertIntToString(ImageWidth));
	ProcessMessage.append("x");
	ProcessMessage.append(ToolBox.ConvertIntToString(ImageHeight));
	ProcessMessage.append(", Channels: ");
	ProcessMessage.append(ToolBox.ConvertIntToString(ImageChannels));

	for(ChannelCounter=0;ChannelCounter<ImageChannels;ChannelCounter++)
	{
		uchar* data= ImageBuffer.ptr<uchar>(0);
		//uchar* MonoData= MonoBuffer.ptr<uchar>(0);

		ByteCounter = 0;

		// Check if Y channel image
		if(ChannelCounter == 0)
		{
			// Go thru pixels of Y channel image
			for(PixelCounter=0;PixelCounter<ImageHeight*ImageWidth;PixelCounter++)
			{
				// Write pixel value to file
				TestFileHandle.put(data[(PixelCounter * ImageChannels) + ChannelCounter]);
				ByteCounter++;
			}
		}
		else
		{
			// Go thru pixel data of each color channel image
			for(PixelCounter=0;PixelCounter<ImageHeight*ImageWidth;PixelCounter=PixelCounter+2)
			{
				// Get two adjacent pixels
				Pixel1 = data[( PixelCounter      * ImageChannels) + ChannelCounter];
				Pixel2 = data[((PixelCounter + 1) * ImageChannels) + ChannelCounter];
				// Form average of pixel values and write to file
				TestFileHandle.put((uchar) ((Pixel1 + Pixel2)/2));
				/*
		ProcessMessage.append(", Bytes [Ch. ");
		ProcessMessage.append(ToolBox.ConvertIntToString(ChannelCounter + 1));
		ProcessMessage.append("]: ");
		ProcessMessage.append(ToolBox.ConvertIntToString(ByteCounter));
		*/
		//namedWindow("Source", CV_WINDOW_AUTOSIZE);					// !!!!!!!!!!!!!!!! WINDOW FOR TEST DISPLAY
		//namedWindow("Mono Image", CV_WINDOW_AUTOSIZE);					// !!!!!!!!!!!!!!!! WINDOW FOR TEST DISPLAY
		//imshow( "Source", ImageBuffer );
		//imshow( "Mono Image", MonoBuffer );
		//waitKey(0);
		//destroyWindow( "Source" );
				ByteCounter++;
			}
		}
		//destroyWindow( "Mono Image" );

	}

	MessageHandler.SaveLogEntry(VW_LOG_FILENAME, "MESSAGE: ", ProcessMessage);
}

// ----------------------------------------------------------------------------------------------------------------
// Save a MAT structure to a YUV file with 4:4:4 Color Subsampling
//
// ----------------------------------------------------------------------------------------------------------------
void YUVFileHandling::WriteYUVImage_444Planar(fstream &TestFileHandle, Mat &ImageBuffer)
{
	CommonMessageHandler	MessageHandler;
	CommonToolBox			ToolBox;

	string					ProcessMessage = "Undefined";

	int		ImageHeight		= ImageBuffer.rows;
	int		ImageWidth		= ImageBuffer.cols;
	int		ImageChannels	= ImageBuffer.channels();
	int		ByteCounter		= 0;
	int		ChannelCounter	= 0;
	int		PixelCounter	= 0;

	//Mat		MonoBuffer(ImageHeight, ImageWidth, CV_8UC1);

	CvSize	ImageSize;

	ImageSize.height		= ImageHeight;
	ImageSize.width			= ImageWidth;

	ProcessMessage.clear();
	ProcessMessage.append("Wrote YUV Image (4:4:4 Planar). Resolution: ");
	ProcessMessage.append(ToolBox.ConvertIntToString(ImageWidth));
	ProcessMessage.append("x");
	ProcessMessage.append(ToolBox.ConvertIntToString(ImageHeight));
	ProcessMessage.append(", Channels: ");
	ProcessMessage.append(ToolBox.ConvertIntToString(ImageChannels));

	for(ChannelCounter=0;ChannelCounter<ImageChannels;ChannelCounter++)
	{
		uchar* data= ImageBuffer.ptr<uchar>(0);
		//uchar* MonoData= MonoBuffer.ptr<uchar>(0);

		ByteCounter = 0;

		for(PixelCounter=0;PixelCounter<ImageHeight*ImageWidth;PixelCounter++)
		{
			TestFileHandle.put(data[(PixelCounter * ImageChannels) + ChannelCounter]);
			//MonoData[PixelCounter] = data[(PixelCounter * ImageChannels) + ChannelCounter];	
			ByteCounter++;
		}
		ProcessMessage.append(", Bytes [Ch. ");
		ProcessMessage.append(ToolBox.ConvertIntToString(ChannelCounter + 1));
		ProcessMessage.append("]: ");
		ProcessMessage.append(ToolBox.ConvertIntToString(ByteCounter));

		//namedWindow("Source", CV_WINDOW_AUTOSIZE);					// !!!!!!!!!!!!!!!! WINDOW FOR TEST DISPLAY
		//namedWindow("Mono Image", CV_WINDOW_AUTOSIZE);					// !!!!!!!!!!!!!!!! WINDOW FOR TEST DISPLAY
		//imshow( "Source", ImageBuffer );
		//imshow( "Mono Image", MonoBuffer );
		//waitKey(0);
		//destroyWindow( "Source" );
		//destroyWindow( "Mono Image" );

	}

	MessageHandler.SaveLogEntry(VW_LOG_FILENAME, "MESSAGE: ", ProcessMessage);
}

bool YUVFileHandling::ReadYUVImage_444Planar(std::fstream &TestFileHandle, Mat &ImageBuffer, int ImageWidth, int ImageHeight, int ImageType)
{
	CommonMessageHandler	MessageHandler;
	CommonToolBox			ToolBox;

	string	ProcessMessage = "Undefined";

	int		ChannelCount	= 3;
	int		ChannelCounter	= 0;
	int		PixelCounter	= 0;
	int		ByteCounter		= 0;

	char	ByteBuffer;

	bool	ErrorDetected	= false;

	ProcessMessage.clear();
	ProcessMessage.append("Read YUV Image. Resolution: ");
	ProcessMessage.append(ToolBox.ConvertIntToString(ImageWidth));
	ProcessMessage.append("x");
	ProcessMessage.append(ToolBox.ConvertIntToString(ImageHeight));
	ProcessMessage.append(", Channels: ");
	ProcessMessage.append(ToolBox.ConvertIntToString(ChannelCount));

	for(ChannelCounter=0;ChannelCounter<ChannelCount;ChannelCounter++)
	{
		uchar* data= ImageBuffer.ptr<uchar>(0);

		ByteCounter = 0;

		for(PixelCounter=0; PixelCounter<(ImageWidth * ImageHeight);PixelCounter++)
		{
			ByteBuffer = TestFileHandle.get();
			if(TestFileHandle.good())
			{
				int testv = (PixelCounter * ChannelCount) + ChannelCounter;
				data[(PixelCounter * ChannelCount) + ChannelCounter] = (unsigned int) ByteBuffer;
				ByteCounter++;
			}
			else
			{
				ErrorDetected = true;
				PixelCounter = ImageWidth * ImageHeight;
				ChannelCounter = ChannelCount;
			}
		}
		if(!ErrorDetected)
		{
			ProcessMessage.append(", Bytes [Ch. ");
			ProcessMessage.append(ToolBox.ConvertIntToString(ChannelCounter + 1));
			ProcessMessage.append("]: ");
			ProcessMessage.append(ToolBox.ConvertIntToString(ByteCounter));
		}
	}

	if(ErrorDetected)
	{
		MessageHandler.SaveLogEntry(VW_LOG_FILENAME, "ERROR: ", "Could not read YUV image");
		return false;
	}
	else
	{
		MessageHandler.SaveLogEntry(VW_LOG_FILENAME, "MESSAGE: ", ProcessMessage);
		return true;
	}
}

/* Function added to OpenCV to improve the capability to read and write YUV420
* image files and convert in YUV 444 format.
*
*
* Author: Alberto - bstlc@...
* Not very optimized, but these funcions works fine!
*
* *** functions ***
* Load image YUV 420
* Save Image YUV 420
* New: Visualize YUV 420 images
*
*/

void YUVFileHandling::check_error (int val) 
{
	if (val<0)
	{
		fprintf(stderr, "Error load YUV file!\nPress ENTER to exit\n");
		getchar();
		exit(-1);
	}
}

/*
* Given a yuv 420 file stream extract the luma and chroma components, the 2 chroma
* are then upsampled by a 2 factor, and return an image composed by 3 layer
* Y, U and V (format YUV 444, i.e. 3 byte for each pixel)
*
*/

IplImage * YUVFileHandling::cvLoadImageYUV (char * name_file, int w, int h)
{
	IplImage *py, *pu, *pv, *pu_big, *pv_big, *image;
	int i, temp;

	FILE * pf= fopen(name_file, "rb");
	if (pf == NULL)
	{
		fprintf(stderr, "Error open file %s\nPress ENTER to exit\n", name_file);
		getchar();
		exit(-1);
	}

	py = cvCreateImage(cvSize(w,h), IPL_DEPTH_8U, 1);
	pu = cvCreateImage(cvSize(w/2,h/2), IPL_DEPTH_8U, 1);
	pv = cvCreateImage(cvSize(w/2,h/2), IPL_DEPTH_8U, 1);

	pu_big = cvCreateImage(cvSize(w,h), IPL_DEPTH_8U, 1);
	pv_big = cvCreateImage(cvSize(w,h), IPL_DEPTH_8U, 1);

	image = cvCreateImage(cvSize(w,h), IPL_DEPTH_8U, 3);

	assert(py);
	assert(pu);
	assert(pv);
	assert(pu_big);
	assert(pv_big);
	assert(image);

	// Read Y
	for (i=0; i<w*h; i++)
	{
		temp = fgetc(pf);
		check_error(temp);

		py->imageData[i] = (unsigned char) temp;
	}


	// Read U
	for (i=0; i<w*h/4; i++)
	{
		temp = fgetc(pf);
		check_error(temp);

		pu->imageData[i] = (unsigned char) temp;
	}



	// Read V
	for (i=0; i<w*h/4; i++)
	{
		temp = fgetc(pf);
		check_error(temp);

		pv->imageData[i] = (unsigned char) temp;
	}

	fclose(pf);

	cvResize(pu, pu_big, CV_INTER_LINEAR);
	cvResize(pv, pv_big, CV_INTER_LINEAR);

	cvReleaseImage(&pu);
	cvReleaseImage(&pv);

	cvMerge(py, pu_big, pv_big, NULL, image);

	cvReleaseImage(&py);
	cvReleaseImage(&pu_big);
	cvReleaseImage(&pv_big);

	return image;
}

/*
* Save image YUV 444 in YUV 420 format onto a file
*
*/

void YUVFileHandling::SaveImageYUV (char * name_file, IplImage * img)
{
	IplImage * pu_small, *pv_small, *py, *pu, *pv;
	int w = img->width;
	int h = img->height;
	int i, temp;
	FILE * pf;

	py = cvCreateImage(cvSize(w,h), IPL_DEPTH_8U, 1);
	pu = cvCreateImage(cvSize(w,h), IPL_DEPTH_8U, 1);
	pv = cvCreateImage(cvSize(w,h), IPL_DEPTH_8U, 1);
	pu_small = cvCreateImage(cvSize(w/2,h/2), IPL_DEPTH_8U, 1);
	pv_small = cvCreateImage(cvSize(w/2,h/2), IPL_DEPTH_8U, 1);

	assert(py);
	assert(pu);
	assert(pv);
	assert(pu_small);
	assert(pv_small);

	cvSplit(img, py, pu, pv, NULL);

	cvResize(pu, pu_small, CV_INTER_LINEAR);
	cvResize(pv, pv_small, CV_INTER_LINEAR);

	cvReleaseImage(&pu);
	cvReleaseImage(&pv);

	pf = fopen(name_file, "wb");
	if (pf == NULL)
	{
		fprintf(stderr, "Error open file %s\nPress ENTER to exit\n", name_file);
		getchar();
		exit(-1);
	}

	for (i=0; i<w*h; i++)
	{
		temp = fputc(py->imageData[i], pf);
		check_error(temp);
	}

	for (i=0; i<w*h/4; i++)
	{
		temp = fputc(pu_small->imageData[i], pf);
		check_error(temp);
	}

	for (i=0; i<w*h/4; i++)
	{
		temp = fputc(pv_small->imageData[i], pf);
		check_error(temp);
	}

	fclose(pf);

	cvReleaseImage(&py);
	cvReleaseImage(&pu_small);
	cvReleaseImage(&pv_small);

	return;
}


/*
* Show YUV 444 images after conversion in RGB color
*
*/


void YUVFileHandling::ShowImageYUV(char * window, IplImage * img, int waitTime) 
{

	int w = img->width;
	int h = img->height;
	IplImage * dst = cvCreateImage(cvSize(w,h), IPL_DEPTH_8U, 3);

	cvCvtColor(img,dst,CV_YCrCb2RGB);
	cvShowImage(window, dst);
	cvWaitKey(waitTime);

	cvReleaseImage(&dst);
}

