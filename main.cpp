//естественная многопутевая сбалансированная двухфазная неравномерная

#include <string>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <Windows.h>

using std::cout;
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

bool isEndOfTempFiles(FileData *files, const int &from, const int &to) {
    bool isEnd = true;

    for (int i = from; i < to && isEnd; ++i) {
        isEnd = files[i].eof;
    }

    return isEnd;
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

bool merge(FileData *files, const int &amountOfFiles, bool isSwap) {
    bool endOfSort = false;

    for (int i = 0; i < amountOfFiles; ++i) {
        if (isSwap) {
            openForWriting(files[i]);
        } else {
            openForReading(files[i]);
        }
    }

    for (int i = amountOfFiles; i < amountOfFiles * 2; ++i) {
        if (isSwap) {
            openForReading(files[i]);
        } else {
            openForWriting(files[i]);
        }
    }

    int copyToFileNum = isSwap ? 0 : amountOfFiles;
    int checkFrom = isSwap ? amountOfFiles : 0;
    int checkTo = isSwap ? amountOfFiles * 2 : amountOfFiles;

    while (!isEndOfTempFiles(files, checkFrom, checkTo)) {
        while (!isEndOfTempSequences(files, checkFrom, checkTo)) {
            copyElement(minInSequences(files, checkFrom, checkTo), files[copyToFileNum]);
        }

        if (isSwap) {
            for (int i = amountOfFiles; i < amountOfFiles * 2; ++i) {
                files[i].eos = files[i].eof;
            }

            if (copyToFileNum == amountOfFiles - 1) {
                copyToFileNum = 0;
            } else {
                copyToFileNum++;
            }
        } else {
            for (int i = 0; i < amountOfFiles; ++i) {
                files[i].eos = files[i].eof;
            }

            if (copyToFileNum == amountOfFiles * 2 - 1) {
                copyToFileNum = amountOfFiles;
            } else {
                copyToFileNum++;
            }
        }
    }

    closeAll(files, amountOfFiles * 2);

    FileData &secondFileInNextStep = isSwap ? files[1] : files[amountOfFiles + 1];

    secondFileInNextStep.file.open(secondFileInNextStep.filename);

    if (secondFileInNextStep.file.peek() == EOF) {
        endOfSort = true;
    }

    secondFileInNextStep.file.close();

    return endOfSort;
}

void sort(string source) {
    const int amountOfActiveFiles = 100;
    const int amountOfFiles = amountOfActiveFiles * 2;

    auto *files = new FileData[amountOfFiles];

    FileData mainFile;
    mainFile.filename = source;

    for (int i = 0; i < amountOfFiles; i++) {
        files[i].filename = "files/f" + to_string(i) + ".txt";
    }

    distribution(mainFile, files, amountOfActiveFiles);

    bool isSwap = false;
    bool end = false;
    int counter = 1;

    while (!end) {
        cout << counter++ << " merge\n";

        end = merge(files, amountOfActiveFiles, isSwap);
        isSwap = !isSwap;
    }

    CopyFile((isSwap ? files[amountOfActiveFiles] : files[0]).filename.c_str(), source.c_str(), 0);

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

