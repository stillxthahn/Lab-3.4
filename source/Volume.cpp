#include "Volume.h"

string const Volume::Extension = ".qtv";

Volume::Volume(string const &volumeFilePath) : VolumeInfo(), EntryTable()
{
    this->initialize(volumeFilePath);
}

Volume::~Volume() {}

bool Volume::create()
{
    // Check if this file exist, if yes, we cannot create a volume with this name
    fstream tempFile(this->path, ios_base::in | ios_base::binray);
    if (tempFile.is_open())
    {
        tempFile.close();
        return false;
    }

    // Create a volume file
    fstream file(this->path, ios_base::out | ios_base::binary);
    if (file.is_open())
    {
        file.clear();
        this->VolumeInfo.write(file);
        file.close();
        return true;
    }
    return false;
}

void Volume::open()
{
    // Open a volume file, then read info of VolumeInfo and EntryTable
    fstream file(this->path, ios_base::in | ios_base::binary);
    if (file.is_open())
    {
        this->seekToHeadOfVolumeInfo_g(file);
        this->VolumeInfo.read(file);

        this->seekToHeadOfEntryTable_g(file);
        this->EntryTable.read(file, this->VolumeInfo);
        file.close();
    }

    // Show list of files/folders in this volume
    // Perform all functions like: import, export, delete, set/reset password for a file/folder
    this->performFunctions();
}

bool Volume::isEmpty() const
{
    return this->VolumeInfo.isEmptyVolume();
}

void Volume::importGUI(Entry *parent)
{
    system("cls");
}