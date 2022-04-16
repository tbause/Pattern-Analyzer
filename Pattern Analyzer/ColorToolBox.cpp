#include "stdafx.h"

using namespace std;

// YUVType:
// 1 = Planar
// 2 = Packed

void ColorToolBox::ConvertRGBToYUV(Mat &RGBImage, Mat &YUVImage)
{
	int	FieldLineCounter	= 0;												// Counts the lines while going thru the image
	int ColCounter			= 0;												// Counts the columns while going thru the image
	int RowCount			= RGBImage.rows;									// number of lines
	int ColCount			= RGBImage.cols;									// number of columns
	int ColByteCount		= RGBImage.cols * RGBImage.channels();				// number of bytes per line

	int FrameLineCounter	= 0;												// Counts the lines for the image with the even lines
	int KeyStroke			= 0;												// Value of the key pressed to continue 
	int WaitTime			= 0;												// Value in milliseconds for Wait function (0 = wait for key stroke)

	double RValue			= 0.0;
	double GValue			= 0.0;
	double BValue			= 0.0;
	double YValue			= 0.0;
	double UValue			= 0.0;
	double VValue			= 0.0;

	// Rec 709 offset values
	double YOffset = 16;
	double UOffset = 128;
	double VOffset = 128;

	// Rec 709
	// source: http://en.wikipedia.org/wiki/YUV
	double YCoeff_R = 0.2126;
	double YCoeff_G = 0.7152;
	double YCoeff_B = 0.0722;

	double UCoeff_R = -0.09991;
	double UCoeff_G = -0.33609;
	double UCoeff_B = 0.436;

	double VCoeff_R = 0.615;
	double VCoeff_G = -0.55861;
	double VCoeff_B = -0.05639;

	// Rec 709
	// source: http://www.martinreddy.net/gfx/faqs/colorconv.faq
	//double YCoeff_R = 0.2215;
	//double YCoeff_G = 0.7154;
	//double YCoeff_B = 0.0721;

	//double UCoeff_R = -0.1145;
	//double UCoeff_G = -0.3855;
	//double UCoeff_B = 0.5000;

	//double VCoeff_R = 0.5016;
	//double VCoeff_G = -0.4556;
	//double VCoeff_B = -0.0459;

	// SMPTE 240M 
	// source: http://www.martinreddy.net/gfx/faqs/colorconv.faq
	//double YCoeff_R = 0.2122;
	//double YCoeff_G = 0.7013;
	//double YCoeff_B = 0.0865;

	//double UCoeff_R = -0.1162;
	//double UCoeff_G = -0.3838;
	//double UCoeff_B = 0.5000;

	//double VCoeff_R = 0.5000;
	//double VCoeff_G = -0.4451;
	//double VCoeff_B = -0.0549;

	// Go thru all lines of the field frames
	for (FieldLineCounter=0; FieldLineCounter<RowCount; FieldLineCounter++) 
	{

		// ----------------------------------------------------------------------------------
		// write even line to frame
		// ----------------------------------------------------------------------------------
		// Get pointer to line of target frame
		uchar* YUVData = YUVImage.ptr<uchar>(FieldLineCounter);

		// Get pointer to line of even field frame
		uchar* RGBData = RGBImage.ptr<uchar>(FieldLineCounter);

		// Copy pixel information
		for (ColCounter=0; ColCounter<ColByteCount; ColCounter=ColCounter+3)
		{
			BValue = RGBData[ColCounter];
			GValue = RGBData[ColCounter + 1];
			RValue = RGBData[ColCounter + 2];

			YValue = ((YCoeff_R * RValue) + (YCoeff_G * GValue) + (YCoeff_B * BValue)) + YOffset;
			UValue = ((UCoeff_R * RValue) + (UCoeff_G * GValue) + (UCoeff_B * BValue)) + UOffset;
			VValue = ((VCoeff_R * RValue) + (VCoeff_G * GValue) + (VCoeff_B * BValue)) + VOffset;

			// Clip upper values
			if(YValue > 240)
				YValue = 240;
			if(UValue > 240)
				UValue = 240;
			if(VValue > 240)
				VValue = 240;

			// Clip lower values
			if(YValue < 16)
				YValue = 16;
			if(UValue < 16)
				UValue = 16;
			if(VValue < 16)
				VValue = 16;

			YUVData[ColCounter]     = (uchar) YValue;
			YUVData[ColCounter + 1] = (uchar) UValue;
			YUVData[ColCounter + 2] = (uchar) VValue;
		}
	}
}
