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
    cout << "===== IMPORT A FILE/FOLDER =====";
    cout << "\n\n";

    cout << "  Program: * Input a path of a folder or a file that you want to import to this volume"
         << "\n";
    cout << "           * Do not input anything then press Enter to";
    cout << " EXIT";

    cout << "\n\n"
         << "  User: ";

    string str;
    FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
    getline(cin, str);

    if (str == "")
    {
        return;
    }

    if (this->import(str, parent))
    {
        cout << "\n\n"
             << "  Program: Import successfully."
             << "\n\n";
        cout << "  ";
        system("pause");
    }
    else
    {
        cout << "\n\n"
             << "  Program: Can not import this path into the volume."
             << "\n\n";
        cout << "           Maybe this path does not exist OR"
             << "\n\n";
        cout << "           The size of this file is equal or larger than 4 GB (4,294,967,296 bytes) OR"
             << "\n\n";
        cout << "           This file or folder has the same name with the one in this volume."
             << "\n\n";
        cout << "           Please check again!"
             << "\n\n";
        cout << "  ";
        system("pause");
    }
}

bool Volume::import(string const &new_file_path, Entry *parent)
{
    fstream volumeStream(this->path, ios_base::in | ios_base::out | ios_base::binary);
    if (!volumeStream.is_open())
    {
        throw "Volume Path Error";
    }
    // Step 1: We'll get and store the info of the new file(s)
    // into our program.

    // If the file we inputted is just a file (not a folder),
    // we'll get only the info of that file. But if the file
    // we inputted in is in fact a folder, then:

    // We'll use "level order tree traversal" to travel to each
    // and every file or folder in the "directory tree"
    // rooted at the file we inputted in.
    // Each time we arrive at a file or folder in the tree,
    // we'll get the info of that file or folder.

    vector<Entry> file_entry_vector;

    Entry file_entry;
    _WIN32_FIND_DATAA ffd;

    // We'll use a variable to keep track of the position
    // Where data of a new file is going to be inserted.
    // At first its value will be the original offset
    // of the Entry Table (which is also the end position
    // of the Data Area).
    // Every time a file is found and an entry is created,
    // the value of this variable will be used to fill in
    // the "file offset" field of the entry. After that,
    // the value of this variable will be updated.

    uint64_t insert_pos;
    if (this->isEmpty())
    {
        insert_pos = 0;
    }
    else
    {
        insert_pos = this->VolumeInfo.getEntryTableOffset();
    }

    // Try finding the importing file and getting its info.
    HANDLE hFile = FindFirstFileA(new_file_path.c_str(), &ffd);

    // If the file can't be found
    if (hFile = INVALID_HANDLE_VALUE)
    {
        volumeStream.close();
        return false;
    }

    // If the size of the file is equal or larger than 4GB
    // (= 2^32 = 4,294,967,296 bytes)
    if (ffd.nFileSizeHigh != 0)
    {
        volumeStream.close();
        return false;
    }

    // If the importing file is alreay a folder
    if (ffd.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY)
    {
        queue<string> folder_path_queue;

        const string root_folder_path = new_file_path;
        const string root_folder_name = ffd.cFileName;
        string parent_folder_path;
        string file_name_in_volume;

        file_name_in_volume = root_folder_name;
        file_name_in_volume += "\\";

        file_entry.getFileInfoAndConvertToEntry(ffd, root_folder_path, file_name_in_volume, insert_pos);
        file_entry_vector.push_back(file_entry);

        folder_path_queue.push(root_folder_path);

        Entry root = file_entry_vector[0];
        root.standardizeAfterImport(parent);

        // Check if Entry already existed
        if (parent->hasChildWithTheSameName(root))
        {
            volumeStream.close();
            return false;
        }

        while (!folder_path_queue.empty())
        {
            parent_folder_path = folder_path_queue.front() + "\\*";
            folder_path_queue.pop();

            // Every folder always contains two special subdirectories:
            // "." (which refers to the current directory)
            // and ".." (which refers to the parent directory).
            // For some reasons, the file finding function in WinAPI
            // when searching for files in a folder
            // will always find these two special directories first.
            // So, when we've found these two directories, we have to
            // ignore them.

            // Find the "." directory, and then ignore it.
            hFile = FindFirstFileA(parent_folder_path.c_str(), &ffd);
            if (hFile == INVALID_HANDLE_VALUE)
                continue;

            // Find the ".." directory, and the ignore it
            if (FindNextFileA(hFile, &ffd) == 0)
                continue;

            // Now we can find the first REAL file in the current folder

            // If the finding function can't find any file
            // (in other words, this folder is empty),
            // we can continue with the next folder in the queue
            if (FindNextFileA(hFile, &ffd) == 0)
                continue;

            do
            {
                Entry temp_entry;
                string file_path = parent_folder_path.substr(0, parent_folder_path.length() - 1) + ffd.cFileName;
                file_name_in_volume = parent_folder_path.substr(
                                          root_folder_path.length() - root_folder_name.length(),
                                          parent_folder_path.length() - root_folder_path.length() + root_folder_name.length() - 1) +
                                      ffd.cFileName;

                if (ffd.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY)
                {
                    file_name_in_volume += "\\";
                }

                temp_entry.getFileInfoAndConvertToEntry(ffd, file_path, file_name_in_volume, insert_pos);
                file_entry_vector.push_back(temp_entry);

                // If the current file is a folder
                if (ffd.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY)
                {
                    // Get the parent folder's path, remove the asterisk
                    // at the tail, append the current folder's name,
                    // and push the whole path into the queue
                    folder_path_queue.push(parent_folder_path.substr(0, parent_folder_path.length() - 1) + ffd.cFileName);
                }
            } while (FindNextFileA(hFile, &ffd) != 0);
        }
    }

    // Else, which means the importing file is just a normal file
    // (not a folder).
    else
    {
        file_entry.getFileInfoAndConvertToEntry(ffd, new_file_path, ffd.cFileName, insert_pos);
        file_entry_vector.push_back(file_entry);

        Entry root = file_entry_vector[0];
        root.standardizeAfterImport(parent);
        if (parent->hasChildWithTheSameName(root))
        {
            volumeStream.close();
            return false;
        }
    }

    FindClose(hFile);

    // Convert to standardized entry
    for (size_t i = 0; i < file_entry_vector.size(); i++)
    {
        file_entry_vector[i].standardizeAfterImport(parent);
    }

    // Step 2: Get the data of new file(s),
    // and write both the info and data of the new file(s) into the volume.

    if (!this->isEmpty())
    {
        // Set volume stream's pointer
        // to the beginning of Entry Table
        this->seekToHeadOfEntryTable_p(volumeStream);
    }
    else
    {
        // If the volume is empty
        // Set the volume stream's pointer
        // to the beginning of the volume
        volumeStream.seekp(0, ios::beg);
    }

    // Write a bundle of
    // [Data of the new file(s)]
    // - [Entries of old files] (which is the content of the Entry Table
    // stored earlier)
    // - [Entry (or Entries) of the new file(s)] (consisting of
    // the new file's (or new files') info)
    // - [Volume Info Area]
    // and update the volume's info in the Volume Info Area.

    // Get data of the new file(s), and write to volume
    for (size_t i = 0; i < file_entry_vector.size(); i++)
    {

        // We only get the data of the files which are not folders
        if (file_entry_vector[i].getIsFolder() == false)
        {
            ifstream import_file_stream;
            import_file_stream.open(file_entry_vector[i].getFullPathOutside(), ios::binary);

            if (import_file_stream.is_open() == true)
            {
                const int BUFFER_SIZE = 4096;
                char buffer[BUFFER_SIZE];

                for (uint64_t j = 0; j < file_entry_vector[i].getSizeData() / BUFFER_SIZE; j++)
                {
                    import_file_stream.read(buffer, BUFFER_SIZE);
                    volumeStream.write(buffer, BUFFER_SIZE);
                }
                import_file_stream.read(buffer, file_entry_vector[i].getSizeData() % BUFFER_SIZE);
                volumeStream.write(buffer, file_entry_vector[i].getSizeData() % BUFFER_SIZE);
                import_file_stream.close();
            }
        }
    }

    // Update the offset of Entry Table
    this->VolumeInfo.updateOffsetEntryTable((uint64_t)volumeStream.tellp());

    // Add new entries to Entry Table (RAM)
    for (size_t i = 0; i < file_entry_vector.size(); i++)
    {
        this->EntryTable.add(file_entry_vector[i]);
    }

    // Write updated Entry Table
    this->EntryTable.write(volumeStream);

    // Update the size of Entry Table
    this->VolumeInfo.updateSizeEntryTable((unit64_t)volumeStream.tellp());

    // Write the Volume Info Area
    this->VolumeInfo.write(volumeStream);

    volumeStream.close();

    return true;
}

void Volume::exportGUI(Entry *f)
{
    string pw, repw;
    if (f->isLocked())
    {
        system("cls");
        cout << "===== LOGIN =====" << endl
             << endl;
        while (true)
        {
            cout << "  Password:  ";
            cout << endl;
            getline(cin, pw);
            setColor(COLOR::LIGHT_CYAN, COLOR::BLACK);
            cout << "  Re-enter:  ";
            getline(cin, repw);
            cout << endl;

            if (pw.compare(repw) == 0 && pw.length() >= 8)
            {
                break;
            }

            if (!f->checkPassword(pw))
            {
                cout << "  Error: Invalid password. Access folder denied. ";
                system("pause");
                return;
            }
            cout << "  Error: Password must be the same and have at least 8 characters. " << endl;
        }
    }

    system("cls");
    cout << "===== EXPORT A FILE/FOLDER =====";
    cout << "\n\n";

    cout << "  File/folder to be exported: ";
    cout << f->getPath() << endl;
    cout << "\n";

    cout << "  Program: * Input a path of a folder that you want to export this file/folder to"
         << "\n";
    cout << "           * Do not input anything then press Enter to";
    cout << " EXIT";

    cout << "\n\n"
         << "  User: ";

    string str;
    FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
    getline(cin, str);

    if (str == "")
    {
        return;
    }

    // ExportState to get the export status
    ExportState state = this->exportFile(f, str);
    if (state == ExportState::SUCCESS)
    {
        cout << "\n\n"
             << "  Program: Export successfully."
             << "\n\n";
    }
    else if (state == ExportState::NOT_TOTALLY)
    {
        cout << "\n\n"
             << "  Program: Can not export this file/folder tottaly because there are still some files/folders are secured with password.";
        cout << "\n\n"
             << "           To export the locked files/folders, please unlock them first."
             << "\n\n";
    }
    else if (state == ExportState::SAME_NAME)
    {
        cout << "\n\n"
             << "  Program: This file or folder has the same name with the one at the specified path.";
        cout << "\n\n"
             << "           Please check again!"
             << "\n\n";
    }
    else if (state == ExportState::BAD_PATH)
    {
        cout << "\n\n"
             << "  Program: The specified path does not exist";
        cout << "\n\n"
             << "           Please check again!"
             << "\n\n";
    }
    else
    {
        throw "Enum Class Error";
    }

    cout << "  ";
}

ExportState Volume::exportFile(Entry *export_file_entry, string const &destination_path)
{
    fstream volume_stream(this->Path, ios::in | ios_base::binary);
    if (volume_stream.is_open() == false)
    {
        throw "Volume Path Error";
    }

    // Check if the destination path exists
    _WIN32_FIND_DATAA ffd;
    HADNLE hFile = FindFirstFileA(destianation_path.c_str(), &ffd);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        volume_stream.close();
        return ExportState::BAD_PATH;
    }

    // Check if there's already a file/folder with same name
    // at the destination path
    string file_path = destination_path + "\\" + export_file_entry->getName();
    hFile = FindFirstFileA(file_path.c_str(), &ffd);
    if (hFile != INVALID_HANDLE_VALUE)
    {
        volume_stream.close();
        return ExportState::SAME_NAME;
    }

    // We use "level order tree traversal" algorithm to travel to
    // each and every subfiles and subfolders of the inputted file
    // (if the inputted file is actually a folder).
    ExportState state = ExportState::SUCCESS;

    queue<Entry *> export_file_entry_queue;
    export_file_entry_queue.push(export_file_entry);

    bool firstItem = true;
    while (export_file_entry_queue.empty() == false)
    {
        Entry *file_entry = export_file_entry_queue.front();
        export_file_entry_queue.pop();

        if (file_entry->isLocked())
        {
            if (firstItem != true)
            {
                state = ExportState::NOT_TOTALLY;
                continue;
            }
        }
        firstItem = false;

        // If the file in question is a normal file (not a folder).
        if (file_entry->isFolder() == false)
        {
            string file_creation_path;
            file_creation_path = destination_path + "\\";

            if (file_entry->hasParent(this->EntryTable.Root))
            {
                file_creation_path += file_entry->getName();
            }
            else
            {
                file_creation_path += file_entry->getPath().substr(
                    export_file_entry->getPath().length() - (export_file_entry->getName().length() + 1),
                    file_entry->getPath().length() - (export_file_entry->getPath().length() - (export_file_entry->getName().length() + 1)));
            }
            ofstream export_file_stream;
            export_file_stream.open(file_creation_path, ios::binary);

            if (export_file_stream.is_open() == true)
            {
                const int BUFFER_SIZE = 4096;
                char buffer[BUFFER_SIZE];

                file_entry->seekToHeadOfData_g(volume_stream);
                for (uint64_t i = 0; i < file_entry->getSizeData() / BUFFER_SIZE;
                     i++)
                {
                    volume_stream.read(buffer, BUFFER_SIZE);
                    export_file_stream.write(buffer, BUFFER_SIZE);
                }
                volume_stream.read(buffer,
                                   file_entry->getSizeData() % BUFFER_SIZE);
                export_file_stream.write(buffer,
                                         file_entry->getSizeData() % BUFFER_SIZE);

                export_file_stream.close();
            }
        }

        // If the file in question is actually a folder.
        else
        {
            string folder_creation_path;
            folder_creation_path = destination_path + "\\" + file_entry->getPath().substr(export_file_entry->getPath().length() - (export_file_entry->getName().length() + 1), file_entry->getPath().length() - (export_file_entry->getPath().length() - (export_file_entry->getName().length() + 1)) - 1);
            if (CreateDirectoryA(folder_creation_path.c_str(), NULL) != 0)
            {
                // Push the entries of subfiles into the queue
                vector<Entry *> sub_entry_list = file_entry->getSubEntryList();
                for (size_t i = 0; i < sub_entry_list.size(); i++)
                {
                    export_file_entry_queue.push(sub_entry_list[i]);
                }
            }
        }
    }

    volume_stream.close();
    return state;
}
