#ifndef __command__
#define __command__

typedef enum COMMAND {
  UPLOAD,
  DOWNLOAD,
  GET_SYNC_DIR,
  END_GET_SYNC_DIR,
  DELETE,
  LIST_SERVER,
  EXIT
} COMMAND;

#endif
