#define FUSE_USE_VERSION 35
#include <fuse3/fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
#include <linux/limits.h>
#define _GNU_SOURCE
#include <fcntl.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <zlib.h>

#define CHIHO_ROOT "chiho"
#define FUSE_ROOT "fuse_dir"

const char *chiho_names[] = {"starter", "metro", "dragon", "blackrose", "heaven", "youth", "7sref"};
#define NUM_CHIHO (sizeof(chiho_names) / sizeof(chiho_names[0]))

// Helper untuk path
static void fullpath(char fpath[PATH_MAX], const char *chiho, const char *path, int add_mai) {
    const char *p = path[0] == '/' ? path + 1 : path;
    if (strncmp(p, chiho, strlen(chiho)) == 0 && p[strlen(chiho)] == '/')
        p += strlen(chiho) + 1;
    if (add_mai) 
        snprintf(fpath, PATH_MAX, "%s/%s/%s.mai", CHIHO_ROOT, chiho, p);
    else
        snprintf(fpath, PATH_MAX, "%s/%s/%s", CHIHO_ROOT, chiho, p);
}

// Validasi path
static int is_chiho_path(const char *path, const char *chiho) {
    const char *p = path[0] == '/' ? path + 1 : path;
    return strncmp(p, chiho, strlen(chiho)) == 0 && (p[strlen(chiho)] == '/' || p[strlen(chiho)] == '\0');
}

// Transformasi untuk setiap chiho
static void metro_transform_write(char *buf, size_t size) {
    for (size_t i = 0; i < size; i++) {
        buf[i] = (buf[i] + (i % 256)) % 256;
    }
}
static void metro_transform_read(char *buf, size_t size) {
    for (size_t i = 0; i < size; i++) {
        buf[i] = (buf[i] - (i % 256)) % 256;
    }
}

static void dragon_transform_write(char *buf, size_t size) {
    for (size_t i = 0; i < size; i++) {
        if ((buf[i] >= 'A' && buf[i] <= 'Z') || (buf[i] >= 'a' && buf[i] <= 'z')) {
            char base = (buf[i] >= 'a') ? 'a' : 'A';
            buf[i] = base + ((buf[i] - base + 13) % 26);
        }
    }
}
static void dragon_transform_read(char *buf, size_t size) {
    dragon_transform_write(buf, size); // ROT13 simetris
}

static void blackrose_transform_write(char *buf, size_t size) {
    // Tidak ada transformasi
}
static void blackrose_transform_read(char *buf, size_t size) {
    // Tidak ada transformasi
}

// Heaven Chiho: AES-256-CBC
#define AES_KEY "0123456789abcdef0123456789abcdef" // 32 byte (256 bit)
#define IV_SIZE 16
static void heaven_transform_write(char *buf, size_t size, char *out, size_t *out_size) {
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    unsigned char iv[IV_SIZE];
    RAND_bytes(iv, IV_SIZE);

    EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, (unsigned char *)AES_KEY, iv);
    
    int len;
    memcpy(out, iv, IV_SIZE); // Simpan IV di awal
    EVP_EncryptUpdate(ctx, (unsigned char *)(out + IV_SIZE), &len, (unsigned char *)buf, size);
    *out_size = IV_SIZE + len;
    EVP_EncryptFinal_ex(ctx, (unsigned char *)(out + *out_size), &len);
    *out_size += len;
    
    EVP_CIPHER_CTX_free(ctx);
}
static void heaven_transform_read(char *buf, size_t size, char *out, size_t *out_size) {
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    unsigned char iv[IV_SIZE];
    memcpy(iv, buf, IV_SIZE); // Ambil IV dari awal

    EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, (unsigned char *)AES_KEY, iv);
    
    int len;
    EVP_DecryptUpdate(ctx, (unsigned char *)out, &len, (unsigned char *)(buf + IV_SIZE), size - IV_SIZE);
    *out_size = len;
    EVP_DecryptFinal_ex(ctx, (unsigned char *)(out + *out_size), &len);
    *out_size += len;
    
    EVP_CIPHER_CTX_free(ctx);
}

// Skystreet Chiho: Gzip (zlib)
static void skystreet_transform_write(char *buf, size_t size, char *out, size_t *out_size) {
    z_stream strm;
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    deflateInit2(&strm, Z_DEFAULT_COMPRESSION, Z_DEFLATED, 15 | 16, 8, Z_DEFAULT_STRATEGY);
    
    strm.avail_in = size;
    strm.next_in = (Bytef *)buf;
    strm.avail_out = *out_size;
    strm.next_out = (Bytef *)out;
    
    deflate(&strm, Z_FINISH);
    *out_size = strm.total_out;
    
    deflateEnd(&strm);
}
static void skystreet_transform_read(char *buf, size_t size, char *out, size_t *out_size) {
    z_stream strm;
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    inflateInit2(&strm, 15 | 32);
    
    strm.avail_in = size;
    strm.next_in = (Bytef *)buf;
    strm.avail_out = *out_size;
    strm.next_out = (Bytef *)out;
    
    inflate(&strm, Z_FINISH);
    *out_size = strm.total_out;
    
    inflateEnd(&strm);
}

// 7sRef Chiho: Gateway
typedef struct {
    const char *chiho;
    const char *filename;
} Parsed7sRefPath;

static int parse_7sref_path(const char *path, Parsed7sRefPath *parsed) {
    const char *p = path[0] == '/' ? path + 1 : path;
    if (strncmp(p, "7sref/", 6) != 0) return 0;
    p += 6;
    
    const char *underscore = strchr(p, '_');
    if (!underscore) return 0;
    
    static char chiho[NAME_MAX];
    static char filename[NAME_MAX];
    strncpy(chiho, p, underscore - p);
    chiho[underscore - p] = '\0';
    strcpy(filename, underscore + 1);
    
    parsed->chiho = chiho;
    parsed->filename = filename;
    return 1;
}

// FUSE Operations
static int mai_getattr(const char *path, struct stat *stbuf, struct fuse_file_info *fi) {
    (void) fi;
    int res;

    if (strcmp(path, "/") == 0) {
        res = lstat(CHIHO_ROOT, stbuf);
        if (res == -1) return -errno;
        return 0;
    }

    // Cek setiap chiho
    for (size_t i = 0; i < NUM_CHIHO; i++) {
        if (is_chiho_path(path, chiho_names[i])) {
            char fpath[PATH_MAX];
            fullpath(fpath, chiho_names[i], path, strcmp(chiho_names[i], "starter") == 0);
            res = lstat(fpath, stbuf);
            if (res == -1) return -errno;
            return 0;
        }
    }

    // Cek 7sref
    Parsed7sRefPath parsed;
    if (parse_7sref_path(path, &parsed)) {
        char newpath[PATH_MAX];
        snprintf(newpath, PATH_MAX, "/%s/%s", parsed.chiho, parsed.filename);
        return mai_getattr(newpath, stbuf, fi);
    }

    return -ENOENT;
}

static int mai_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                       off_t offset, struct fuse_file_info *fi,
                       enum fuse_readdir_flags flags) {
    (void) offset;
    (void) fi;
    (void) flags;

    if (strcmp(path, "/") == 0) {
        filler(buf, ".", NULL, 0, 0);
        filler(buf, "..", NULL, 0, 0);
        for (size_t i = 0; i < NUM_CHIHO; i++) {
            filler(buf, chiho_names[i], NULL, 0, 0);
        }
        return 0;
    }

    // Cek setiap chiho
    for (size_t i = 0; i < NUM_CHIHO; i++) {
        if (is_chiho_path(path, chiho_names[i])) {
            filler(buf, ".", NULL, 0, 0);
            filler(buf, "..", NULL, 0, 0);

            char dirpath[PATH_MAX];
            snprintf(dirpath, PATH_MAX, "%s/%s", CHIHO_ROOT, chiho_names[i]);

            DIR *dp = opendir(dirpath);
            if (dp == NULL) return -errno;

            struct dirent *de;
            while ((de = readdir(dp)) != NULL) {
                if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0) continue;
                char name[NAME_MAX];
                strcpy(name, de->d_name);
                if (strcmp(chiho_names[i], "starter") == 0) {
                    size_t len = strlen(name);
                    if (len > 4 && strcmp(name + len - 4, ".mai") == 0) {
                        name[len - 4] = '\0';
                    }
                }
                filler(buf, name, NULL, 0, 0);
            }
            closedir(dp);
            return 0;
        }
    }

    // 7sRef: Tampilkan semua file dari semua chiho
    if (is_chiho_path(path, "7sref")) {
        filler(buf, ".", NULL, 0, 0);
        filler(buf, "..", NULL, 0, 0);

        for (size_t i = 0; i < NUM_CHIHO - 1; i++) { // Skip 7sref itself
            char dirpath[PATH_MAX];
            snprintf(dirpath, PATH_MAX, "%s/%s", CHIHO_ROOT, chiho_names[i]);

            DIR *dp = opendir(dirpath);
            if (dp == NULL) continue;

            struct dirent *de;
            while ((de = readdir(dp)) != NULL) {
                if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0) continue;
                char name[NAME_MAX];
                if (strcmp(chiho_names[i], "starter") == 0) {
                    size_t len = strlen(de->d_name);
                    if (len > 4 && strcmp(de->d_name + len - 4, ".mai") == 0) {
                        strncpy(name, de->d_name, len - 4);
                        name[len - 4] = '\0';
                    } else {
                        continue;
                    }
                } else {
                    strcpy(name, de->d_name);
                }
                char display_name[2 * NAME_MAX]; // Perbesar buffer untuk menghindari truncation
                snprintf(display_name, sizeof(display_name), "%s_%s", chiho_names[i], name);
                filler(buf, display_name, NULL, 0, 0);
            }
            closedir(dp);
        }
        return 0;
    }

    return -ENOENT;
}

static int mai_open(const char *path, struct fuse_file_info *fi) {
    // Cek 7sref
    Parsed7sRefPath parsed;
    if (parse_7sref_path(path, &parsed)) {
        char newpath[PATH_MAX];
        snprintf(newpath, PATH_MAX, "/%s/%s", parsed.chiho, parsed.filename);
        return mai_open(newpath, fi);
    }

    for (size_t i = 0; i < NUM_CHIHO; i++) {
        if (is_chiho_path(path, chiho_names[i])) {
            char fpath[PATH_MAX];
            fullpath(fpath, chiho_names[i], path, strcmp(chiho_names[i], "starter") == 0);
            int fd = open(fpath, fi->flags);
            if (fd == -1) return -errno;
            fi->fh = fd;
            return 0;
        }
    }

    return -ENOENT;
}

static int mai_read(const char *path, char *buf, size_t size, off_t offset,
                    struct fuse_file_info *fi) {
    // Cek 7sref
    Parsed7sRefPath parsed;
    if (parse_7sref_path(path, &parsed)) {
        char newpath[PATH_MAX];
        snprintf(newpath, PATH_MAX, "/%s/%s", parsed.chiho, parsed.filename);
        return mai_read(newpath, buf, size, offset, fi);
    }

    int res = pread(fi->fh, buf, size, offset);
    if (res == -1) return -errno;

    for (size_t i = 0; i < NUM_CHIHO; i++) {
        if (is_chiho_path(path, chiho_names[i])) {
            if (strcmp(chiho_names[i], "metro") == 0) {
                metro_transform_read(buf, res);
            } else if (strcmp(chiho_names[i], "dragon") == 0) {
                dragon_transform_read(buf, res);
            } else if (strcmp(chiho_names[i], "blackrose") == 0) {
                blackrose_transform_read(buf, res);
            } else if (strcmp(chiho_names[i], "heaven") == 0) {
                char *temp = malloc(res + 1);
                memcpy(temp, buf, res);
                size_t out_size = res;
                heaven_transform_read(temp, res, buf, &out_size);
                free(temp);
                return out_size;
            } else if (strcmp(chiho_names[i], "youth") == 0) {
                char *temp = malloc(res + 1);
                memcpy(temp, buf, res);
                size_t out_size = size;
                skystreet_transform_read(temp, res, buf, &out_size);
                free(temp);
                return out_size;
            }
            break;
        }
    }

    return res;
}

static int mai_write(const char *path, const char *buf, size_t size, off_t offset,
                     struct fuse_file_info *fi) {
    // Cek 7sref
    Parsed7sRefPath parsed;
    if (parse_7sref_path(path, &parsed)) {
        char newpath[PATH_MAX];
        snprintf(newpath, PATH_MAX, "/%s/%s", parsed.chiho, parsed.filename);
        return mai_write(newpath, buf, size, offset, fi);
    }

    char *temp = malloc(size + 1);
    memcpy(temp, buf, size);

    for (size_t i = 0; i < NUM_CHIHO; i++) {
        if (is_chiho_path(path, chiho_names[i])) {
            if (strcmp(chiho_names[i], "metro") == 0) {
                metro_transform_write(temp, size);
            } else if (strcmp(chiho_names[i], "dragon") == 0) {
                dragon_transform_write(temp, size);
            } else if (strcmp(chiho_names[i], "blackrose") == 0) {
                blackrose_transform_write(temp, size);
            } else if (strcmp(chiho_names[i], "heaven") == 0) {
                char *out = malloc(size + 256); // Tambahan ruang untuk IV dan padding
                size_t out_size = size + 256;
                heaven_transform_write(temp, size, out, &out_size);
                free(temp);
                temp = out;
                size = out_size;
            } else if (strcmp(chiho_names[i], "youth") == 0) {
                char *out = malloc(size + 256); // Tambahan ruang untuk kompresi
                size_t out_size = size + 256;
                skystreet_transform_write(temp, size, out, &out_size);
                free(temp);
                temp = out;
                size = out_size;
            }
            break;
        }
    }

    int res = pwrite(fi->fh, temp, size, offset);
    free(temp);
    if (res == -1) return -errno;
    return res;
}

static int mai_create(const char *path, mode_t mode, struct fuse_file_info *fi) {
    // Cek 7sref
    Parsed7sRefPath parsed;
    if (parse_7sref_path(path, &parsed)) {
        char newpath[PATH_MAX];
        snprintf(newpath, PATH_MAX, "/%s/%s", parsed.chiho, parsed.filename);
        return mai_create(newpath, mode, fi);
    }

    for (size_t i = 0; i < NUM_CHIHO; i++) {
        if (is_chiho_path(path, chiho_names[i])) {
            char dirpath[PATH_MAX];
            snprintf(dirpath, PATH_MAX, "%s/%s", CHIHO_ROOT, chiho_names[i]);
            struct stat st = {0};
            if (stat(dirpath, &st) == -1) {
                if (mkdir(dirpath, 0755) == -1 && errno != EEXIST) {
                    return -errno;
                }
            }

            char fpath[PATH_MAX];
            fullpath(fpath, chiho_names[i], path, strcmp(chiho_names[i], "starter") == 0);
            int fd = open(fpath, O_CREAT | O_RDWR, mode);
            if (fd == -1) return -errno;
            fi->fh = fd;
            return 0;
        }
    }

    return -ENOENT;
}

static int mai_unlink(const char *path) {
    // Cek 7sref
    Parsed7sRefPath parsed;
    if (parse_7sref_path(path, &parsed)) {
        char newpath[PATH_MAX];
        snprintf(newpath, PATH_MAX, "/%s/%s", parsed.chiho, parsed.filename);
        return mai_unlink(newpath);
    }

    for (size_t i = 0; i < NUM_CHIHO; i++) {
        if (is_chiho_path(path, chiho_names[i])) {
            char fpath[PATH_MAX];
            fullpath(fpath, chiho_names[i], path, strcmp(chiho_names[i], "starter") == 0);
            int res = unlink(fpath);
            if (res == -1) return -errno;
            return 0;
        }
    }

    return -ENOENT;
}

static const struct fuse_operations mai_oper = {
    .getattr = mai_getattr,
    .readdir = mai_readdir,
    .open    = mai_open,
    .read    = mai_read,
    .write   = mai_write,
    .create  = mai_create,
    .unlink  = mai_unlink,
};

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <mountpoint>\n", argv[0]);
        return 1;
    }

    const char *mountpoint = argv[1];
    struct stat st = {0};

    // Buat mount point
    if (stat(mountpoint, &st) == -1) {
        if (mkdir(mountpoint, 0755) == -1) {
            perror("mkdir mountpoint");
            return 1;
        }
    }

    // Buat direktori chiho
    if (stat(CHIHO_ROOT, &st) == -1) {
        if (mkdir(CHIHO_ROOT, 0755) == -1) {
            perror("mkdir chiho");
            return 1;
        }
    }

    for (size_t i = 0; i < NUM_CHIHO; i++) {
        char dirpath[PATH_MAX];
        snprintf(dirpath, PATH_MAX, "%s/%s", CHIHO_ROOT, chiho_names[i]);
        if (stat(dirpath, &st) == -1) {
            if (mkdir(dirpath, 0755) == -1) {
                perror("mkdir chiho subdir");
                return 1;
            }
        }
    }

    return fuse_main(argc, argv, &mai_oper, NULL);
}