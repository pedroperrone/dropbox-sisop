#include "../include/files_handler.h"

int countElementsOnDir(DIR* dirDescriptor) {
    int i = -2;
    struct dirent *myfile;
    rewinddir(dirDescriptor);
    while ((myfile = readdir(dirDescriptor)) != NULL) {
        i++;
    }
    return i;
}

LIST* getListOfFilesInfo(DIR* dirDescriptor, char username[]) {
    LIST* listOfFilesInfo = createList();
    FILE_INFO *fileInfo;
    struct dirent *myfile;
    struct stat mystat;
    char buf[512];

    if(listOfFilesInfo == NULL) return NULL;

    rewinddir(dirDescriptor);
    while ((myfile = readdir(dirDescriptor)) != NULL) {
        if ((strcmp((char *)&(myfile->d_name), ".") != 0) && (strcmp((char *)&(myfile->d_name), "..") != 0)) {
            fileInfo = (FILE_INFO*) malloc(sizeof(FILE_INFO));
            strncpy((char *)&(fileInfo->filename), myfile->d_name, FILENAME_LENGTH);
            sprintf(buf, "%s/%s", username, myfile->d_name);
            stat(buf, &mystat);
            memcpy(&(fileInfo->details), &mystat, sizeof(struct stat));
            add(fileInfo, listOfFilesInfo);
        }
    }
    return listOfFilesInfo;
}
