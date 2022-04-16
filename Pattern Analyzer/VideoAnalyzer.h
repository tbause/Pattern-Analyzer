class VideoAnalyzer
{
	public:
		// Supporting Methods
		unsigned int	SceneChangeDetection(double* HistDiffValues, double NewHistDiffValue, unsigned int FrameCount);
		void			LaplaceFilter(IplImage* ProcessImage, IplImage* BufferedImage, IplImage* OutputImage);
		CvPoint			LetterboxDetection(IplImage *SourceImage, IplImage *OutputImage, int Threshold);
		double			diffclock(clock_t clock1,clock_t clock2);
		void			SplitIntoFields(Mat &SourceImage, Mat &EvenLineImage, Mat &OddLineImage);
		// Declaration New Methods
		CvScalar		AvgDifference(Mat &ProcessImage, Mat &BufferedImage, Mat &OutputImage);
		CvScalar		AvgStructureDiff(Mat &ProcessImage, Mat &BufferedImage, Mat &OutputImageIplImage);
		MatND			CalculateHSVHistogram(Mat &SourceImage, int H_Bins, int S_Bins, int HUEMaxValue, int SatMaxValue);
		void			CannyFilter(Mat &ProcessImage, Mat &OutputImage);

		// Main Methods
		bool			ContentAnalyzer(string InFile, string OutFile, int StartFrame, int Duration, bool Verbose, bool Display);
		bool			CadenceDetector(string InFile, string OutFile, int StartFrame, int Duration, bool Verbose, bool Display);
};