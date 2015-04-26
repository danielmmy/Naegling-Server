#ifndef __NAEGLING_MAIN_H_
#define __NAEGLING_MAIN_H_

#include <semaphore.h>
#include <sys/stat.h>
#include <pwd.h>
#include <pthread.h>
#include <openssl/md5.h>
#include "db-control-sqlite.h"
#include "vm-control.h"
#include "naegling-com.h"


#define MAX_IP_SIZE 16
#define NAEGLING_CLUSTER_PORT 9291
#define NAEGLING_SERVER_PORT 9292
#define UDP_MESSAGE_MAX_SIZE 1024
#define TCP_MESSAGE_MAX_SIZE 5242880 
#define BACKLOG 5
#define MAX_FILE_PATH_SIZE 512
#define MAX_UDP_MESSAGE_WAIT 5
#define MAX_DOMAIN_SIZE 256
#define MAX_XML_FILE_SIZE 1024
#define XML_BUF_SIZE 128
#define MAX_MD5_SIZE 33
#define MAX_QUERY_SIZE 128
#define MAX_LINE_SIZE 1024
#define MAX_PATH_SIZE 256


extern const char *NAEGLING_PATH;
extern const char *NAEGLING_TEMPLATE_PATH;
extern const char *NAEGLING_JOBS_PATH;
extern const char *DATABASE_PATH;
extern const char *NAEGLING_TEMPLATES_FILE_LIST;
extern const char *MESSAGE_DELIMITER;
extern const char *NAEGLING_TEMPLATES_DIRECTORY;
extern const char *VIR_NAEGLING_NETWORK;
extern int FILE_TRANSFER_AVAILABLE;
extern sem_t sem;

#endif
