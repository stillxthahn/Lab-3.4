#pramge once
#include "VolumeInfor.h"
#include "Entry.h"
#include "Folder.h"

class EntryTable
{
    friend class Volume;

public:
    EntryTable();
    ~EntryTable();

    void read(fstream &file, VolumeInfor const &volumeInfo);
    void write(fstream &file) const;

    void add(Entry const &entry);

    void updateAfterDel(Entry const *entry);

private:
    Entry *Root;
    vector<Entry> EntryList;
}