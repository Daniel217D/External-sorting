//естественная многопутевая сбалансированная двухфазная неравномерная

#include <string>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <windows.h>

using std::cout;
using std::swap;
using std::string;
using std::fstream;
using std::ifstream;
using std::to_string;

struct FileData {
    string filename;
    fstream file;
    bool eof = false;
    bool eos = false;

    int elem = 0;
};

void readNext(FileData &fd) {
    int prev = fd.elem;
    if (fd.file >> fd.elem) {
        if (prev > fd.elem) {
            fd.eos = true;
        }
    } else {
        fd.eof = true;
        fd.eos = true;
    }
}

void openForReading(FileData &fd) {
    fd.file.open(fd.filename, fstream::in);

    if (fd.file.peek() == EOF) {
        fd.eof = fd.eos = true;
    } else {
        fd.file >> fd.elem;
    }
}

void openForWriting(FileData &fd) {
    fd.file.open(fd.filename, fstream::out);
}

void copyElement(FileData &fd1, FileData &fd2) {
    fd2.file << fd1.elem << ' ';
    readNext(fd1);
}

void copyRun(FileData &fd1, FileData &fd2) {
    while (!fd1.eos) {
        copyElement(fd1, fd2);
    }
}

void close(FileData &file) {
    file.eof = false;
    file.eos = false;
    file.file.close();
}

void closeAll(FileData *files, const int &amountOfFiles) {
    for (int i = 0; i < amountOfFiles; i++) {
        close(files[i]);
    }
}

int findEndOfTempFiles(FileData *files, const int &from, const int &to) {
    int firstEmpty = to;
    
    for (int i = to - 1; i >= from && firstEmpty == to; --i) {
        if(files[i].eof) {
            firstEmpty = i;
        }    
    }

    for (int i = from; i < firstEmpty; ++i) {
        if(files[i].eof) {
            swap(files[i], files[firstEmpty - 1]);
            firstEmpty--;
        }
    }
    
    return files[0].eof ? -1 : firstEmpty;
}

bool isEndOfTempSequences(FileData *files, const int &from, const int &to) {
    bool isEnd = true;

    for (int i = from; i < to && isEnd; ++i) {
        isEnd = files[i].eos;
    }

    return isEnd;
}

FileData &minInSequences(FileData *files, const int &from, const int &to) {
    FileData *fd = &files[from];
    int i = from + 1;

    while (fd->eos) {
        fd = &files[i++];
    }

    while (i < to) {
        if (!files[i].eos && files[i].elem < fd->elem) {
            fd = &files[i];
        }

        i++;
    }

    return *fd;
}

void distribution(FileData &mainFile, FileData *files, const int &amountOfFiles) {
    openForReading(mainFile);

    for (int i = 0; i < amountOfFiles; ++i) {
        openForWriting(files[i]);
    }

    while (!mainFile.eof) {
        for (int i = 0; i < amountOfFiles && !mainFile.eof; ++i) {
            copyRun(mainFile, files[i]);
            mainFile.eos = mainFile.eof;
        }
    }

    closeAll(files, amountOfFiles);
}

bool merge(FileData *files, const int &amountOfFiles) {
    bool endOfSort = false;

    for (int i = 0; i < amountOfFiles; ++i) {
        openForReading(files[i]);
    }

    for (int i = amountOfFiles; i < amountOfFiles * 2; ++i) {
        openForWriting(files[i]);
    }

    int copyToFileNum = amountOfFiles;
    int amountOfNotEmptyFiles = amountOfFiles;

    while (amountOfNotEmptyFiles != -1) {
        while (!isEndOfTempSequences(files, 0, amountOfNotEmptyFiles)) {
            copyElement(minInSequences(files, 0, amountOfNotEmptyFiles), files[copyToFileNum]);
        }

        for (int i = 0; i < amountOfNotEmptyFiles; ++i) {
            files[i].eos = files[i].eof;
        }

        if (copyToFileNum == amountOfFiles * 2 - 1) {
            copyToFileNum = amountOfFiles;
        } else {
            copyToFileNum++;
        }

        amountOfNotEmptyFiles = findEndOfTempFiles(files, 0, amountOfNotEmptyFiles);
    }

    closeAll(files, amountOfFiles * 2);

    FileData &secondFileInNextStep = files[amountOfFiles + 1];

    secondFileInNextStep.file.open(secondFileInNextStep.filename);

    if (secondFileInNextStep.file.peek() == EOF) {
        endOfSort = true;
    }

    secondFileInNextStep.file.close();

    return endOfSort;
}

void sort(string source) {
    const int amountOfActiveFiles = 3;
    const int amountOfFiles = amountOfActiveFiles * 2;

    auto *files = new FileData[amountOfFiles];

    FileData mainFile;
    mainFile.filename = source;

    for (int i = 0; i < amountOfFiles; i++) {
        files[i].filename = "files/f" + to_string(i) + ".txt";
    }

    distribution(mainFile, files, amountOfActiveFiles);

    bool end = false;
    int counter = 1;

    while (!end) {
        cout << counter++ << " merge\n";

        end = merge(files, amountOfActiveFiles);

        for (int i = 0; i < amountOfActiveFiles; ++i) {
            swap(files[i], files[amountOfActiveFiles + i]);
        }
    }

    CopyFile(files[0].filename.c_str(), source.c_str(), 0);

    for (int i = 0; i < amountOfFiles; i++) {
        remove(("files/f" + to_string(i) + ".txt").c_str());
    }

    delete[] files;
}

int main() {
    string backup = "files/in_backup_big.txt";
    string source = "files/in.txt";
    CopyFile(backup.c_str(), source.c_str(), 0);

    ifstream fileIn(source);

    if (!fileIn) {
        cout << "No file!\n";
    } else {
        cout << "Start sorting\n";

        sort(source);

        cout << "Ready\n";
    }

    return 0;
}

