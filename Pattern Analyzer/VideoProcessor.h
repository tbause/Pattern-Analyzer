using namespace cv;

class VideoProcessor
{

	public:
		void salt(Mat &image, int n);
		void SplitIntoFields(Mat &SourceImage, Mat &EvenLineImage, Mat &OddLineImage);
		void CombineIntoFrame(Mat &EvenLineImage, Mat &OddLineImage, Mat &TargetFrame);
		bool DeInterlaceFile(string InFile, string OutFile, int StartFrame, int Duration);
		bool InterlaceFile(string InFile, string OutFile, int StartFrame, int Duration, int ImageWidth, int IntHeight);
		void HandOverTest(Mat &TestImage1, Mat &TestImage2);
};