using namespace std;

namespace MediaWorker
{
	// ---------------------------------------------------------------------------------------------------------
	// Watch Folder Class
	// Created: August 2014
	// Author: Thomas Bause Mason
	//
	// Class for handling watch folders
	//
	// ---------------------------------------------------------------------------------------------------------

	class FolderControl
	{
	private:
		string	FolderListFileName;


		bool	CreateFolderListFile(string ListFileName);

	public:
				FolderControl();
		bool	Add();
		bool	Delete();

	};
}