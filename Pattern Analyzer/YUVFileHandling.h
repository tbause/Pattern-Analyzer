class YUVFileHandling
{

	public:
		void ShowImageYUV(char * window, IplImage * img, int waitTime);
		void SaveImageYUV(char * name_file, IplImage * img);
		IplImage * cvLoadImageYUV(char * name_file, int w, int h);
		void check_error(int val);
		void WriteYUVImage_444Planar(std::fstream &TestFileHandle, Mat &ImageBuffer);
		bool ReadYUVImage_444Planar(std::fstream &TestFileHandle, Mat &ImageBuffer, int ImageWidth, int ImageHeight, int ImageType);
		void WriteYUVImage_422Planar(std::fstream &TestFileHandle, Mat &ImageBuffer);
		bool ReadYUVImage_422Planar(std::fstream &TestFileHandle, Mat &ImageBuffer, int ImageWidth, int ImageHeight, int ImageType);
		void WriteYUVImage_420Planar(std::fstream &TestFileHandle, Mat &ImageBuffer);
		bool ReadYUVImage_420Planar(std::fstream &TestFileHandle, Mat &ImageBuffer, int ImageWidth, int ImageHeight, int ImageType);
};