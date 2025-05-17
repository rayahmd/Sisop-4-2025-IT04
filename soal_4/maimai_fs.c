#define FUSE_USE_VERSION 31
#define _FILE_OFFSET_BITS 64
#include <fuse.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <openssl/aes.h>
#include <openssl/rand.h>
#include <zlib.h>

// Configuration defines
#define STARTER_DIR "chiho/starter"
#define STARTER_EXT ".mai"
#define METRO_DIR   "chiho/metro"
#define METRO_EXT   ".ccc"
#define DRAGON_DIR  "chiho/dragon"
#define DRAGON_EXT  ".rot"
#define BLACKROSE_DIR "chiho/blackrose"
#define BLACKROSE_EXT ".bin"
#define HEAVEN_DIR  "chiho/heaven"
#define HEAVEN_EXT  ".enc"
#define HEAVEN_KEY  "0123456789abcdef0123456789abcdef"
#define YOUTH_DIR   "chiho/skystreet"
#define YOUTH_EXT   ".gz"
#define PATH_MAX    4096

// Helper function to get real path for starter area
static void get_starter(const char *fuse_path, char *real_path) {
    snprintf(real_path, PATH_MAX, "%s/%s%s", STARTER_DIR, fuse_path, STARTER_EXT);
}

// Helper function for metro area shift encryption
static void metro_encrypt(const char *input, char *output, size_t len) {
    for (size_t i = 0; i < len; i++) {
        output[i] = (input[i] + (i % 256)) % 256;
    }
}

static void metro_decrypt(const char *input, char *output, size_t len) {
    for (size_t i = 0; i < len; i++) {
        output[i] = (input[i] - (i % 256)) % 256;
    }
}

// Helper function to get real path for metro area
static void get_metro(const char *fuse_path, char *real_path) {
    snprintf(real_path, PATH_MAX, "%s/%s%s", METRO_DIR, fuse_path, METRO_EXT);
}

// ROT13 implementation for dragon area
static void rot13(char *str, size_t len) {
    for (size_t i = 0; i < len; i++) {
        if (str[i] >= 'a' && str[i] <= 'z') {
            str[i] = 'a' + ((str[i] - 'a' + 13) % 26);
        } else if (str[i] >= 'A' && str[i] <= 'Z') {
            str[i] = 'A' + ((str[i] - 'A' + 13) % 26);
        }
    }
}

// Helper function to get real path for dragon area
static void get_dragon(const char *fuse_path, char *real_path) {
    snprintf(real_path, PATH_MAX, "%s/%s%s", DRAGON_DIR, fuse_path, DRAGON_EXT);
}

// Helper function to get real path for blackrose area
static void get_blackrose(const char *fuse_path, char *real_path) {
    snprintf(real_path, PATH_MAX, "%s/%s%s", BLACKROSE_DIR, fuse_path, BLACKROSE_EXT);
}

// AES encryption for heaven area
static void aes_encrypt(const char *input, size_t len, char *output, unsigned char *iv) {
    AES_KEY aes_key;
    AES_set_encrypt_key((const unsigned char *)HEAVEN_KEY, 256, &aes_key);
    AES_cbc_encrypt((const unsigned char *)input, (unsigned char *)output, len, &aes_key, iv, AES_ENCRYPT);
}

static void aes_decrypt(const char *input, size_t len, char *output, unsigned char *iv) {
    AES_KEY aes_key;
    AES_set_decrypt_key((const unsigned char *)HEAVEN_KEY, 256, &aes_key);
    AES_cbc_encrypt((const unsigned char *)input, (unsigned char *)output, len, &aes_key, iv, AES_DECRYPT);
}

// Helper function to get real path for heaven area
static void get_heaven(const char *fuse_path, char *real_path) {
    snprintf(real_path, PATH_MAX, "%s/%s%s", HEAVEN_DIR, fuse_path, HEAVEN_EXT);
}

// Gzip compression for youth area
static int gzip_compress(const char *input, size_t len, char *output, size_t *outlen) {
    z_stream zs;
    zs.zalloc = Z_NULL;
    zs.zfree = Z_NULL;
    zs.opaque = Z_NULL;
    
    if (deflateInit2(&zs, Z_DEFAULT_COMPRESSION, Z_DEFLATED, 15 + 16, 8, Z_DEFAULT_STRATEGY) != Z_OK) {
        return -1;
    }
    
    zs.next_in = (Bytef *)input;
    zs.avail_in = len;
    zs.next_out = (Bytef *)output;
    zs.avail_out = *outlen;
    
    if (deflate(&zs, Z_FINISH) != Z_STREAM_END) {
        deflateEnd(&zs);
        return -1;
    }
    
    *outlen = zs.total_out;
    deflateEnd(&zs);
    return 0;
}

static int gzip_decompress(const char *input, size_t len, char *output, size_t *outlen) {
    z_stream zs;
    zs.zalloc = Z_NULL;
    zs.zfree = Z_NULL;
    zs.opaque = Z_NULL;
    
    if (inflateInit2(&zs, 15 + 16) != Z_OK) {
        return -1;
    }
    
    zs.next_in = (Bytef *)input;
    zs.avail_in = len;
    zs.next_out = (Bytef *)output;
    zs.avail_out = *outlen;
    
    if (inflate(&zs, Z_FINISH) != Z_STREAM_END) {
        inflateEnd(&zs);
        return -1;
    }
    
    *outlen = zs.total_out;
    inflateEnd(&zs);
    return 0;
}

// Helper function to get real path for youth area
static void get_youth(const char *fuse_path, char *real_path) {
    snprintf(real_path, PATH_MAX, "%s/%s%s", YOUTH_DIR, fuse_path, YOUTH_EXT);
}

// Helper function to parse 7sref paths
static int parse_7sref_path(const char *path, char *area, char *filename) {
    const char *underscore = strchr(path, '_');
    if (!underscore) return -1;
    
    size_t area_len = underscore - path;
    if (area_len >= PATH_MAX) return -1;
    
    strncpy(area, path, area_len);
    area[area_len] = '\0';
    strcpy(filename, underscore + 1);
    return 0;
}

// Get attributes for a file
static int mai_getattr(const char *path, struct stat *stbuf) {
    int res = 0;
    memset(stbuf, 0, sizeof(struct stat));
    
    if (strcmp(path, "/") == 0) {
        stbuf->st_mode = S_IFDIR | 0755;
        stbuf->st_nlink = 2;
        return 0;
    }
    
    // Check if this is a 7sref path
    char area[PATH_MAX], filename[PATH_MAX];
    if (strncmp(path, "/7sref/", 7) == 0 && parse_7sref_path(path + 7, area, filename) == 0) {
        char new_path[PATH_MAX];
        snprintf(new_path, PATH_MAX, "/%s/%s", area, filename);
        return mai_getattr(new_path, stbuf);
    }
    
    // Determine which area we're in
    char real_path[PATH_MAX];
    if (strncmp(path, "/starter/", 9) == 0) {
        get_starter(path + 9, real_path);
    } else if (strncmp(path, "/metro/", 7) == 0) {
        get_metro(path + 7, real_path);
    } else if (strncmp(path, "/dragon/", 8) == 0) {
        get_dragon(path + 8, real_path);
    } else if (strncmp(path, "/blackrose/", 11) == 0) {
        get_blackrose(path + 11, real_path);
    } else if (strncmp(path, "/heaven/", 8) == 0) {
        get_heaven(path + 8, real_path);
    } else if (strncmp(path, "/skystreet/", 11) == 0) {
        get_youth(path + 11, real_path);
    } else {
        return -ENOENT;
    }
    
    res = lstat(real_path, stbuf);
    if (res == -1) return -errno;
    
    return 0;
}

// Read directory contents
static int mai_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                      off_t offset, struct fuse_file_info *fi) {
    (void) offset;
    (void) fi;
    
    if (strcmp(path, "/") == 0) {
        filler(buf, "starter", NULL, 0);
        filler(buf, "metro", NULL, 0);
        filler(buf, "dragon", NULL, 0);
        filler(buf, "blackrose", NULL, 0);
        filler(buf, "heaven", NULL, 0);
        filler(buf, "skystreet", NULL, 0);
        filler(buf, "7sref", NULL, 0);
        return 0;
    }
    
    // Handle 7sref directory
    if (strcmp(path, "/7sref") == 0) {
        // For simplicity, we don't list contents of 7sref
        return 0;
    }
    
    // Handle other directories
    char real_path[PATH_MAX];
    if (strcmp(path, "/starter") == 0) {
        snprintf(real_path, PATH_MAX, "%s", STARTER_DIR);
    } else if (strcmp(path, "/metro") == 0) {
        snprintf(real_path, PATH_MAX, "%s", METRO_DIR);
    } else if (strcmp(path, "/dragon") == 0) {
        snprintf(real_path, PATH_MAX, "%s", DRAGON_DIR);
    } else if (strcmp(path, "/blackrose") == 0) {
        snprintf(real_path, PATH_MAX, "%s", BLACKROSE_DIR);
    } else if (strcmp(path, "/heaven") == 0) {
        snprintf(real_path, PATH_MAX, "%s", HEAVEN_DIR);
    } else if (strcmp(path, "/skystreet") == 0) {
        snprintf(real_path, PATH_MAX, "%s", YOUTH_DIR);
    } else {
        return -ENOENT;
    }
    
    DIR *dp = opendir(real_path);
    if (dp == NULL) return -errno;
    
    struct dirent *de;
    while ((de = readdir(dp)) != NULL) {
        struct stat st;
        memset(&st, 0, sizeof(st));
        st.st_ino = de->d_ino;
        st.st_mode = de->d_type << 12;
        
        // Remove extensions for FUSE view
        char display_name[PATH_MAX];
        strncpy(display_name, de->d_name, PATH_MAX);
        
        if (strcmp(path, "/starter") == 0) {
            char *ext = strrchr(display_name, '.');
            if (ext && strcmp(ext, STARTER_EXT) == 0) {
                *ext = '\0';
            }
        } else if (strcmp(path, "/metro") == 0) {
            char *ext = strrchr(display_name, '.');
            if (ext && strcmp(ext, METRO_EXT) == 0) {
                *ext = '\0';
            }
        } else if (strcmp(path, "/dragon") == 0) {
            char *ext = strrchr(display_name, '.');
            if (ext && strcmp(ext, DRAGON_EXT) == 0) {
                *ext = '\0';
            }
        } else if (strcmp(path, "/blackrose") == 0) {
            char *ext = strrchr(display_name, '.');
            if (ext && strcmp(ext, BLACKROSE_EXT) == 0) {
                *ext = '\0';
            }
        } else if (strcmp(path, "/heaven") == 0) {
            char *ext = strrchr(display_name, '.');
            if (ext && strcmp(ext, HEAVEN_EXT) == 0) {
                *ext = '\0';
            }
        } else if (strcmp(path, "/skystreet") == 0) {
            char *ext = strrchr(display_name, '.');
            if (ext && strcmp(ext, YOUTH_EXT) == 0) {
                *ext = '\0';
            }
        }
        
        if (filler(buf, display_name, &st, 0)) break;
    }
    
    closedir(dp);
    return 0;
}

// Open a file
static int mai_open(const char *path, struct fuse_file_info *fi) {
    char real_path[PATH_MAX];
    
    // Handle 7sref paths
    char area[PATH_MAX], filename[PATH_MAX];
    if (strncmp(path, "/7sref/", 7) == 0 && parse_7sref_path(path + 7, area, filename) == 0) {
        char new_path[PATH_MAX];
        snprintf(new_path, PATH_MAX, "/%s/%s", area, filename);
        return mai_open(new_path, fi);
    }
    
    // Determine which area we're in
    if (strncmp(path, "/starter/", 9) == 0) {
        get_starter(path + 9, real_path);
    } else if (strncmp(path, "/metro/", 7) == 0) {
        get_metro(path + 7, real_path);
    } else if (strncmp(path, "/dragon/", 8) == 0) {
        get_dragon(path + 8, real_path);
    } else if (strncmp(path, "/blackrose/", 11) == 0) {
        get_blackrose(path + 11, real_path);
    } else if (strncmp(path, "/heaven/", 8) == 0) {
        get_heaven(path + 8, real_path);
    } else if (strncmp(path, "/skystreet/", 11) == 0) {
        get_youth(path + 11, real_path);
    } else {
        return -ENOENT;
    }
    
    int res = open(real_path, fi->flags);
    if (res == -1) return -errno;
    
    fi->fh = res;
    return 0;
}

// Read from a file
static int mai_read(const char *path, char *buf, size_t size, off_t offset,
                   struct fuse_file_info *fi) {
    int fd;
    if (fi) {
        fd = fi->fh;
    } else {
        // Handle 7sref paths
        char area[PATH_MAX], filename[PATH_MAX];
        if (strncmp(path, "/7sref/", 7) == 0 && parse_7sref_path(path + 7, area, filename) == 0) {
            char new_path[PATH_MAX];
            snprintf(new_path, PATH_MAX, "/%s/%s", area, filename);
            return mai_read(new_path, buf, size, offset, fi);
        }
        
        char real_path[PATH_MAX];
        if (strncmp(path, "/starter/", 9) == 0) {
            get_starter(path + 9, real_path);
        } else if (strncmp(path, "/metro/", 7) == 0) {
            get_metro(path + 7, real_path);
        } else if (strncmp(path, "/dragon/", 8) == 0) {
            get_dragon(path + 8, real_path);
        } else if (strncmp(path, "/blackrose/", 11) == 0) {
            get_blackrose(path + 11, real_path);
        } else if (strncmp(path, "/heaven/", 8) == 0) {
            get_heaven(path + 8, real_path);
        } else if (strncmp(path, "/skystreet/", 11) == 0) {
            get_youth(path + 11, real_path);
        } else {
            return -ENOENT;
        }
        
        fd = open(real_path, O_RDONLY);
        if (fd == -1) return -errno;
    }
    
    int res;
    if (strncmp(path, "/starter/", 9) == 0) {
        res = pread(fd, buf, size, offset);
        if (res == -1) res = -errno;
    } else if (strncmp(path, "/metro/", 7) == 0) {
        char *encrypted = malloc(size);
        res = pread(fd, encrypted, size, offset);
        if (res == -1) {
            res = -errno;
        } else {
            metro_decrypt(encrypted, buf, res);
        }
        free(encrypted);
    } else if (strncmp(path, "/dragon/", 8) == 0) {
        char *rotated = malloc(size);
        res = pread(fd, rotated, size, offset);
        if (res == -1) {
            res = -errno;
        } else {
            memcpy(buf, rotated, res);
            rot13(buf, res);
        }
        free(rotated);
    } else if (strncmp(path, "/blackrose/", 11) == 0) {
        res = pread(fd, buf, size, offset);
        if (res == -1) res = -errno;
    } else if (strncmp(path, "/heaven/", 8) == 0) {
        // Read IV from first 16 bytes of file
        unsigned char iv[AES_BLOCK_SIZE];
        if (offset == 0) {
            if (pread(fd, iv, AES_BLOCK_SIZE, 0) != AES_BLOCK_SIZE) {
                if (!fi) close(fd);
                return -EIO;
            }
        } else {
            // For simplicity, we'll assume the IV is stored in the first 16 bytes
            // In a real implementation, you might want to store it differently
            if (!fi) close(fd);
            return -EIO;
        }
        
        // Read encrypted data
        char *encrypted = malloc(size);
        res = pread(fd, encrypted, size, offset + AES_BLOCK_SIZE);
        if (res == -1) {
            res = -errno;
        } else {
            aes_decrypt(encrypted, res, buf, iv);
        }
        free(encrypted);
    } else if (strncmp(path, "/skystreet/", 11) == 0) {
        // Read compressed data
        char *compressed = malloc(size);
        res = pread(fd, compressed, size, offset);
        if (res == -1) {
            res = -errno;
        } else {
            size_t outsize = size * 4; // Guess at decompressed size
            char *decompressed = malloc(outsize);
            if (gzip_decompress(compressed, res, decompressed, &outsize) == 0) {
                memcpy(buf, decompressed, outsize > size ? size : outsize);
                res = outsize;
            } else {
                res = -EIO;
            }
            free(decompressed);
        }
        free(compressed);
    } else {
        res = -ENOENT;
    }
    
    if (!fi) close(fd);
    return res;
}

// Write to a file
static int mai_write(const char *path, const char *buf, size_t size, off_t offset,
                    struct fuse_file_info *fi) {
    int fd;
    if (fi) {
        fd = fi->fh;
    } else {
        // Handle 7sref paths
        char area[PATH_MAX], filename[PATH_MAX];
        if (strncmp(path, "/7sref/", 7) == 0 && parse_7sref_path(path + 7, area, filename) == 0) {
            char new_path[PATH_MAX];
            snprintf(new_path, PATH_MAX, "/%s/%s", area, filename);
            return mai_write(new_path, buf, size, offset, fi);
        }
        
        char real_path[PATH_MAX];
        if (strncmp(path, "/starter/", 9) == 0) {
            get_starter(path + 9, real_path);
        } else if (strncmp(path, "/metro/", 7) == 0) {
            get_metro(path + 7, real_path);
        } else if (strncmp(path, "/dragon/", 8) == 0) {
            get_dragon(path + 8, real_path);
        } else if (strncmp(path, "/blackrose/", 11) == 0) {
            get_blackrose(path + 11, real_path);
        } else if (strncmp(path, "/heaven/", 8) == 0) {
            get_heaven(path + 8, real_path);
        } else if (strncmp(path, "/skystreet/", 11) == 0) {
            get_youth(path + 11, real_path);
        } else {
            return -ENOENT;
        }
        
        fd = open(real_path, O_WRONLY);
        if (fd == -1) return -errno;
    }
    
    int res;
    if (strncmp(path, "/starter/", 9) == 0) {
        res = pwrite(fd, buf, size, offset);
        if (res == -1) res = -errno;
    } else if (strncmp(path, "/metro/", 7) == 0) {
        char *encrypted = malloc(size);
        metro_encrypt(buf, encrypted, size);
        res = pwrite(fd, encrypted, size, offset);
        if (res == -1) res = -errno;
        free(encrypted);
    } else if (strncmp(path, "/dragon/", 8) == 0) {
        char *rotated = malloc(size);
        memcpy(rotated, buf, size);
        rot13(rotated, size);
        res = pwrite(fd, rotated, size, offset);
        if (res == -1) res = -errno;
        free(rotated);
    } else if (strncmp(path, "/blackrose/", 11) == 0) {
        res = pwrite(fd, buf, size, offset);
        if (res == -1) res = -errno;
    } else if (strncmp(path, "/heaven/", 8) == 0) {
        // Generate random IV
        unsigned char iv[AES_BLOCK_SIZE];
        RAND_bytes(iv, AES_BLOCK_SIZE);
        
        // Write IV first
        if (offset == 0) {
            if (pwrite(fd, iv, AES_BLOCK_SIZE, 0) != AES_BLOCK_SIZE) {
                if (!fi) close(fd);
                return -EIO;
            }
        }
        
        // Encrypt data
        char *encrypted = malloc(size + AES_BLOCK_SIZE);
        aes_encrypt(buf, size, encrypted, iv);
        res = pwrite(fd, encrypted, size, offset + AES_BLOCK_SIZE);
        if (res == -1) res = -errno;
        free(encrypted);
    } else if (strncmp(path, "/skystreet/", 11) == 0) {
        // Compress data
        size_t outsize = size + size / 100 + 12; // Rough estimate
        char *compressed = malloc(outsize);
        if (gzip_compress(buf, size, compressed, &outsize) == 0) {
            res = pwrite(fd, compressed, outsize, offset);
            if (res == -1) res = -errno;
        } else {
            res = -EIO;
        }
        free(compressed);
    } else {
        res = -ENOENT;
    }
    
    if (!fi) close(fd);
    return res;
}

// Create a file
static int mai_create(const char *path, mode_t mode, struct fuse_file_info *fi) {
    char real_path[PATH_MAX];
    
    // Handle 7sref paths
    char area[PATH_MAX], filename[PATH_MAX];
    if (strncmp(path, "/7sref/", 7) == 0 && parse_7sref_path(path + 7, area, filename) == 0) {
        char new_path[PATH_MAX];
        snprintf(new_path, PATH_MAX, "/%s/%s", area, filename);
        return mai_create(new_path, mode, fi);
    }
    
    // Determine which area we're in
    if (strncmp(path, "/starter/", 9) == 0) {
        get_starter(path + 9, real_path);
    } else if (strncmp(path, "/metro/", 7) == 0) {
        get_metro(path + 7, real_path);
    } else if (strncmp(path, "/dragon/", 8) == 0) {
        get_dragon(path + 8, real_path);
    } else if (strncmp(path, "/blackrose/", 11) == 0) {
        get_blackrose(path + 11, real_path);
    } else if (strncmp(path, "/heaven/", 8) == 0) {
        get_heaven(path + 8, real_path);
    } else if (strncmp(path, "/skystreet/", 11) == 0) {
        get_youth(path + 11, real_path);
    } else {
        return -ENOENT;
    }
    
    int fd = open(real_path, fi->flags, mode);
    if (fd == -1) return -errno;
    
    fi->fh = fd;
    return 0;
}

// Unlink (delete) a file
static int mai_unlink(const char *path) {
    char real_path[PATH_MAX];
    
    // Handle 7sref paths
    char area[PATH_MAX], filename[PATH_MAX];
    if (strncmp(path, "/7sref/", 7) == 0 && parse_7sref_path(path + 7, area, filename) == 0) {
        char new_path[PATH_MAX];
        snprintf(new_path, PATH_MAX, "/%s/%s", area, filename);
        return mai_unlink(new_path);
    }
    
    // Determine which area we're in
    if (strncmp(path, "/starter/", 9) == 0) {
        get_starter(path + 9, real_path);
    } else if (strncmp(path, "/metro/", 7) == 0) {
        get_metro(path + 7, real_path);
    } else if (strncmp(path, "/dragon/", 8) == 0) {
        get_dragon(path + 8, real_path);
    } else if (strncmp(path, "/blackrose/", 11) == 0) {
        get_blackrose(path + 11, real_path);
    } else if (strncmp(path, "/heaven/", 8) == 0) {
        get_heaven(path + 8, real_path);
    } else if (strncmp(path, "/skystreet/", 11) == 0) {
        get_youth(path + 11, real_path);
    } else {
        return -ENOENT;
    }
    
    int res = unlink(real_path);
    if (res == -1) return -errno;
    
    return 0;
}

// Truncate a file
static int mai_truncate(const char *path, off_t size) {
    char real_path[PATH_MAX];
    
    // Handle 7sref paths
    char area[PATH_MAX], filename[PATH_MAX];
    if (strncmp(path, "/7sref/", 7) == 0 && parse_7sref_path(path + 7, area, filename) == 0) {
        char new_path[PATH_MAX];
        snprintf(new_path, PATH_MAX, "/%s/%s", area, filename);
        return mai_truncate(new_path, size);
    }
    
    // Determine which area we're in
    if (strncmp(path, "/starter/", 9) == 0) {
        get_starter(path + 9, real_path);
    } else if (strncmp(path, "/metro/", 7) == 0) {
        get_metro(path + 7, real_path);
    } else if (strncmp(path, "/dragon/", 8) == 0) {
        get_dragon(path + 8, real_path);
    } else if (strncmp(path, "/blackrose/", 11) == 0) {
        get_blackrose(path + 11, real_path);
    } else if (strncmp(path, "/heaven/", 8) == 0) {
        get_heaven(path + 8, real_path);
    } else if (strncmp(path, "/skystreet/", 11) == 0) {
        get_youth(path + 11, real_path);
    } else {
        return -ENOENT;
    }
    
    int res = truncate(real_path, size);
    if (res == -1) return -errno;
    
    return 0;
}

static struct fuse_operations mai_oper = {
    .getattr    = mai_getattr,
    .readdir    = mai_readdir,
    .open       = mai_open,
    .read       = mai_read,
    .write      = mai_write,
    .create     = mai_create,
    .unlink     = mai_unlink,
    .truncate   = mai_truncate,
};

int main(int argc, char *argv[]) {
    return fuse_main(argc, argv, &mai_oper, NULL);
}