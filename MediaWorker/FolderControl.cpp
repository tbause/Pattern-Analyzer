#include "stdafx.h"

using namespace xercesc;
using namespace MediaWorker;

FolderControl::FolderControl()
{

}

bool FolderControl::CreateFolderListFile(string ListFileName)
{

	XMLPlatformUtils::Initialize();



	XMLPlatformUtils::Terminate();

	return true;
}