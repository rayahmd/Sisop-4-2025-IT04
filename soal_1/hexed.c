#define FUSE_USE_VERSION 31

#include <fuse.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <ctype.h>
#include <sys/stat.h>

#define BUFFER_SIZE 1024
#define DEFAULT_MOUNT_POINT "mnt"

static const char *fileId = "1hi_GDdP51Kn2JJMw02WmCOxuc3qrXzh5";
static const char *zipFileName = "anomali.zip";
static char *hexStr = NULL;
static char outputFileName[256];
static FILE *logFile = NULL;

// Function to download file from Google Drive
void downloadFileFromDrive(const char *fileId, const char *outputFile) {
    char command[512];
    snprintf(command, sizeof(command),
             "wget --no-check-certificate 'https://docs.google.com/uc?export=download&id=%s' -O %s",
             fileId, outputFile);
    if (system(command) != 0) {
        fprintf(stderr, "Failed to download file from Google Drive.\n");
        exit(EXIT_FAILURE);
    }
}

void cleanHex(char *hexStr) {
    int j = 0;
    for (int i = 0; hexStr[i] != '\0'; i++) {
        if (isxdigit(hexStr[i])) {
            hexStr[j++] = hexStr[i];
        }
    }
    hexStr[j] = '\0';
}

int isValidHex(const char *str) {
    for (int i = 0; str[i] != '\0'; i++) {
        if (!isxdigit(str[i])) return 0;
    }
    return 1;
}

void unzipAndDelete(const char *zipFile) {
    char command[512];
    snprintf(command, sizeof(command), "unzip -o %s -d temp", zipFile);
    if (system(command) != 0) {
        perror("Failed to unzip file");
        return;
    }
    snprintf(command, sizeof(command), "mv temp/anomali/* anomali/ 2>/dev/null || true");
    system(command);
    snprintf(command, sizeof(command), "rm -rf temp");
    system(command);
    if (unlink(zipFile) != 0) {
        perror("Failed to delete zip file");
    }
}

void hexToImage(const char *inputPath) {
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    printf("Converting file: %s at %04d-%02d-%02d %02d:%02d:%02d\n", inputPath,
           tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);

    FILE *inFile = fopen(inputPath, "r");
    if (!inFile) {
        fprintf(stderr, "Cannot open %s: %s\n", inputPath, strerror(errno));
        return;
    }

    fseek(inFile, 0, SEEK_END);
    long fileSize = ftell(inFile);
    rewind(inFile);

    hexStr = realloc(hexStr, fileSize + 1);
    if (!hexStr) {
        fclose(inFile);
        fprintf(stderr, "Failed to allocate memory for hexStr\n");
        return;
    }

    size_t bytesRead = fread(hexStr, 1, fileSize, inFile);
    hexStr[bytesRead] = '\0';
    fclose(inFile);

    size_t len = strlen(hexStr);
    if (len > 0 && hexStr[len - 1] == '\n') {
        hexStr[len - 1] = '\0';
        len--;
    }

    cleanHex(hexStr);
    len = strlen(hexStr);
    if (len % 2 != 0 || !isValidHex(hexStr)) {
        fprintf(stderr, "Invalid hex string in file %s\n", inputPath);
        return;
    }

    const char *baseName = strrchr(inputPath, '/');
    if (baseName == NULL) baseName = inputPath;
    else baseName++;
    snprintf(outputFileName, sizeof(outputFileName), "%s/image/%.*s_image_%04d-%02d-%02d_%02d:%02d:%02d.png",
             DEFAULT_MOUNT_POINT, (int)(strstr(baseName, ".txt") - baseName), baseName,
             tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
             tm->tm_hour, tm->tm_min, tm->tm_sec);

    char imageDir[256];
    snprintf(imageDir, sizeof(imageDir), "%s/image", DEFAULT_MOUNT_POINT);
    struct stat st;
    if (stat(imageDir, &st) == -1) {
        if (mkdir(imageDir, 0777) != 0) {
            fprintf(stderr, "Failed to create directory %s: %s\n", imageDir, strerror(errno));
        }
    }

    FILE *outFile = fopen(outputFileName, "wb");
    if (outFile) {
        for (int i = 0; hexStr[i] && hexStr[i + 1]; i += 2) {
            char byteStr[3] = {hexStr[i], hexStr[i + 1], '\0'};
            unsigned char byte = (unsigned char)strtol(byteStr, NULL, 16);
            fwrite(&byte, 1, 1, outFile);
        }
        fclose(outFile);
        printf("Created image: %s\n", outputFileName);
    } else {
        fprintf(stderr, "Failed to create image file %s: %s\n", outputFileName, strerror(errno));
    }

    char logPath[256];
    snprintf(logPath, sizeof(logPath), "%s/conversion.log", DEFAULT_MOUNT_POINT);
    if (!logFile) {
        logFile = fopen(logPath, "a");
        if (!logFile) {
            fprintf(stderr, "Failed to open log file %s: %s\n", logPath, strerror(errno));
        }
    }
    if (logFile) {
        fprintf(logFile, "[%04d-%02d-%02d][%02d:%02d:%02d]: Successfully converted hexadecimal text %s to %s\n",
                tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
                tm->tm_hour, tm->tm_min, tm->tm_sec, baseName, outputFileName);
        fflush(logFile);
    }
}

static int hexed_getattr(const char *path, struct stat *stbuf) {
    memset(stbuf, 0, sizeof(struct stat));
    if (strcmp(path, "/") == 0) {
        stbuf->st_mode = S_IFDIR | 0755;
        stbuf->st_nlink = 2;
        return 0;
    }
    if (strcmp(path, "/image") == 0) {
        stbuf->st_mode = S_IFDIR | 0755;
        stbuf->st_nlink = 2;
        return 0;
    }
    if (strstr(path, "/") && strstr(path, ".txt")) {
        stbuf->st_mode = S_IFREG | 0644;
        stbuf->st_nlink = 1;
        stbuf->st_size = 1024;
        return 0;
    }
    if (strcmp(path, "/conversion.log") == 0) {
        stbuf->st_mode = S_IFREG | 0644;
        stbuf->st_nlink = 1;
        stbuf->st_size = 1024;
        return 0;
    }
    if (strstr(path, "/image/") && strstr(path, ".png")) {
        stbuf->st_mode = S_IFREG | 0644;
        stbuf->st_nlink = 1;
        stbuf->st_size = 1024;
        return 0;
    }
    return -ENOENT;
}

static int hexed_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                        off_t offset, struct fuse_file_info *fi) {
    filler(buf, ".", NULL, 0);
    filler(buf, "..", NULL, 0);
    if (strcmp(path, "/") == 0) {
        DIR *dp = opendir("anomali");
        if (dp) {
            struct dirent *de;
            while ((de = readdir(dp)) != NULL) {
                if (strstr(de->d_name, ".txt")) {
                    filler(buf, de->d_name, NULL, 0);
                }
            }
            closedir(dp);
        } else {
            fprintf(stderr, "Failed to open directory anomali: %s\n", strerror(errno));
        }
        filler(buf, "image", NULL, 0);
        filler(buf, "conversion.log", NULL, 0);
        return 0;
    }
    if (strcmp(path, "/image") == 0) {
        DIR *dp = opendir("mnt/image");
        if (dp) {
            struct dirent *de;
            while ((de = readdir(dp)) != NULL) {
                if (strstr(de->d_name, ".png")) {
                    filler(buf, de->d_name, NULL, 0);
                }
            }
            closedir(dp);
        } else {
            fprintf(stderr, "Failed to open directory mnt/image: %s\n", strerror(errno));
        }
        return 0;
    }
    return -ENOENT;
}

static int hexed_open(const char *path, struct fuse_file_info *fi) {
    if (strstr(path, "/") && strstr(path, ".txt")) {
        return 0;
    }
    if (strcmp(path, "/conversion.log") == 0) {
        return 0;
    }
    if (strstr(path, "/image/") && strstr(path, ".png")) {
        return 0;
    }
    return -ENOENT;
}

static int hexed_read(const char *path, char *buf, size_t size, off_t offset,
                     struct fuse_file_info *fi) {
    if (strstr(path, "/") && strstr(path, ".txt")) {
        char realPath[256];
        snprintf(realPath, sizeof(realPath), "anomali/%s", path + 1);
        printf("Reading file: %s -> %s\n", path, realPath);
        FILE *file = fopen(realPath, "r");
        if (file) {
            fseek(file, offset, SEEK_SET);
            size_t bytesRead = fread(buf, 1, size, file);
            fclose(file);
            hexToImage(realPath);
            return bytesRead;
        } else {
            fprintf(stderr, "Failed to open file %s: %s\n", realPath, strerror(errno));
        }
    }
    if (strcmp(path, "/conversion.log") == 0) {
        char logPath[256];
        snprintf(logPath, sizeof(logPath), "%s/conversion.log", DEFAULT_MOUNT_POINT);
        FILE *file = fopen(logPath, "r");
        if (file) {
            fseek(file, offset, SEEK_SET);
            size_t bytesRead = fread(buf, 1, size, file);
            fclose(file);
            return bytesRead;
        } else {
            fprintf(stderr, "Failed to open log file %s: %s\n", logPath, strerror(errno));
        }
    }
    if (strstr(path, "/image/") && strstr(path, ".png")) {
        char imagePath[256];
        snprintf(imagePath, sizeof(imagePath), "%s%s", DEFAULT_MOUNT_POINT, path);
        FILE *file = fopen(imagePath, "r");
        if (file) {
            fseek(file, offset, SEEK_SET);
            size_t bytesRead = fread(buf, 1, size, file);
            fclose(file);
            return bytesRead;
        } else {
            fprintf(stderr, "Failed to open image file %s: %s\n", imagePath, strerror(errno));
        }
    }
    return -ENOENT;
}

static struct fuse_operations hexed_oper = {
    .getattr = hexed_getattr,
    .readdir = hexed_readdir,
    .open = hexed_open,
    .read = hexed_read,
};

int main(int argc, char *argv[]) {
    struct stat st;
    if (stat("anomali", &st) == -1) {
        mkdir("anomali", 0755);
    }

    downloadFileFromDrive(fileId, zipFileName);
    unzipAndDelete(zipFileName);

    if (stat(DEFAULT_MOUNT_POINT, &st) == -1) {
        if (mkdir(DEFAULT_MOUNT_POINT, 0755) != 0) {
            perror("Failed to create default mount point");
            return 1;
        }
    }

    char *new_argv[3];
    new_argv[0] = argv[0];
    new_argv[1] = DEFAULT_MOUNT_POINT;
    new_argv[2] = NULL;

    printf("Starting FUSE with mount point: %s\n", DEFAULT_MOUNT_POINT);
    return fuse_main(2, new_argv, &hexed_oper, NULL);
}
