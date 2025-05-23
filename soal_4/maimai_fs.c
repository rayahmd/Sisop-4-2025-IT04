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
#include <libgen.h> 
#include <openssl/evp.h>
#include <openssl/aes.h>
#include <openssl/rand.h>
#include <zlib.h>

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
#define PATH_MAX_LEN    4096 
#define AES_BLOCK_SIZE 16

static int ensure_directory_exists(const char *path) {
    char *path_copy_for_dirname = strdup(path);
    if (path_copy_for_dirname == NULL) {
        return -ENOMEM;
    }

    struct stat st;
    if (stat(path, &st) == 0) { 
        if (S_ISDIR(st.st_mode)) {
            free(path_copy_for_dirname);
            return 0; 
        } else {
            free(path_copy_for_dirname);
            return -ENOTDIR; 
        }
    }

    if (errno != ENOENT) {
        int saved_errno = errno;
        free(path_copy_for_dirname);
        return -saved_errno;
    }

    
    char *parent_dir = dirname(path_copy_for_dirname); 

    if (strcmp(parent_dir, path) != 0 && strcmp(parent_dir, ".") != 0 && strcmp(parent_dir, "/") != 0) {
        int res = ensure_directory_exists(parent_dir);
        if (res != 0) {
            free(path_copy_for_dirname); 
            return res;
        }
    }
  
    if (mkdir(path, 0755) == 0) {
        free(path_copy_for_dirname);
        return 0; 
    } else {
        if (errno == EEXIST) {
            if (stat(path, &st) == 0 && S_ISDIR(st.st_mode)) {
                free(path_copy_for_dirname);
                return 0; 
            } else {
                int saved_errno = (errno == EEXIST && !S_ISDIR(st.st_mode)) ? ENOTDIR : errno;
                free(path_copy_for_dirname);
                return -saved_errno;
            }
        }
        int saved_errno = errno; 
        free(path_copy_for_dirname);
        return -saved_errno;
    }
}

static void get_starter(const char *fuse_sub_path, char *real_path) {
    snprintf(real_path, PATH_MAX_LEN, "%s/%s%s", STARTER_DIR, fuse_sub_path, STARTER_EXT);
}

static void get_metro(const char *fuse_sub_path, char *real_path) {
    snprintf(real_path, PATH_MAX_LEN, "%s/%s%s", METRO_DIR, fuse_sub_path, METRO_EXT);
}

static void get_dragon(const char *fuse_sub_path, char *real_path) {
    snprintf(real_path, PATH_MAX_LEN, "%s/%s%s", DRAGON_DIR, fuse_sub_path, DRAGON_EXT);
}

static void get_blackrose(const char *fuse_sub_path, char *real_path) {
    snprintf(real_path, PATH_MAX_LEN, "%s/%s%s", BLACKROSE_DIR, fuse_sub_path, BLACKROSE_EXT);
}

static void get_heaven(const char *fuse_sub_path, char *real_path) {
    snprintf(real_path, PATH_MAX_LEN, "%s/%s%s", HEAVEN_DIR, fuse_sub_path, HEAVEN_EXT);
}

static void get_youth(const char *fuse_sub_path, char *real_path) {
    snprintf(real_path, PATH_MAX_LEN, "%s/%s%s", YOUTH_DIR, fuse_sub_path, YOUTH_EXT);
}

static int mai_create(const char *path, mode_t mode, struct fuse_file_info *fi) {
    char real_path[PATH_MAX_LEN];
    const char *sub_path = NULL;
 
    if (strncmp(path, "/starter/", 9) == 0) {
        sub_path = path + 9;
        get_starter(sub_path, real_path);

    } else if (strncmp(path, "/metro/", 7) == 0) {
        sub_path = path + 7;
        get_metro(sub_path, real_path);
  
    } else if (strncmp(path, "/dragon/", 8) == 0) {
        sub_path = path + 8;
        get_dragon(sub_path, real_path);

    } else if (strncmp(path, "/blackrose/", 11) == 0) {
        sub_path = path + 11;
        get_blackrose(sub_path, real_path);

    } else if (strncmp(path, "/heaven/", 8) == 0) {
        sub_path = path + 8;
        get_heaven(sub_path, real_path);
       
    } else if (strncmp(path, "/skystreet/", 11) == 0) {
        sub_path = path + 11;
        get_youth(sub_path, real_path);
      
    }
    else {
        return -ENOENT;
    }
    
    char *real_path_copy_for_dirname = strdup(real_path);
    if (!real_path_copy_for_dirname) return -ENOMEM;

    char *parent_dir_of_real_file = dirname(real_path_copy_for_dirname);
    
    int res = ensure_directory_exists(parent_dir_of_real_file);
    
    free(real_path_copy_for_dirname); 

    if (res != 0) {
        return res; 
    }

    int fd = open(real_path, fi->flags, mode);
    if (fd == -1) return -errno;

    fi->fh = fd;
    return 0;
}


static void metro_encrypt(const char *input, char *output, size_t len) {
    for (size_t i = 0; i < len; i++) {
        output[i] = (input[i] + (char)(i % 256)) % 256;
    }
}

static void metro_decrypt(const char *input, char *output, size_t len) {
    for (size_t i = 0; i < len; i++) {
        output[i] = (input[i] - (char)(i % 256) + 256) % 256; // ensure positive before modulo
    }
}

static void rot13(char *str, size_t len) {
    for (size_t i = 0; i < len; i++) {
        if (str[i] >= 'a' && str[i] <= 'z') {
            str[i] = 'a' + ((str[i] - 'a' + 13) % 26);
        } else if (str[i] >= 'A' && str[i] <= 'Z') {
            str[i] = 'A' + ((str[i] - 'A' + 13) % 26);
        }
    }
}

static int aes_operation(const char *input, size_t in_len, char *output, size_t *out_len, unsigned char *iv, int encrypt) {
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) return -1;

    int len1 = 0, len2 = 0;
    const EVP_CIPHER *cipher = EVP_aes_256_cbc();

    if (encrypt) {
        if (1 != EVP_EncryptInit_ex(ctx, cipher, NULL, (const unsigned char *)HEAVEN_KEY, iv)) goto err;
        if (1 != EVP_EncryptUpdate(ctx, (unsigned char *)output, &len1, (const unsigned char *)input, in_len)) goto err;
        if (1 != EVP_EncryptFinal_ex(ctx, (unsigned char *)output + len1, &len2)) goto err;
    } else {
        if (1 != EVP_DecryptInit_ex(ctx, cipher, NULL, (const unsigned char *)HEAVEN_KEY, iv)) goto err;
        if (1 != EVP_DecryptUpdate(ctx, (unsigned char *)output, &len1, (const unsigned char *)input, in_len)) goto err;
        if (1 != EVP_DecryptFinal_ex(ctx, (unsigned char *)output + len1, &len2)) goto err;
    }
    
    *out_len = len1 + len2;
    EVP_CIPHER_CTX_free(ctx);
    return 0;
err:
    EVP_CIPHER_CTX_free(ctx);
    return -1; 
}


static int gzip_compress_op(const char *input, size_t len, char *output, size_t *outlen_ptr) {
    z_stream zs;
    memset(&zs, 0, sizeof(zs));
    zs.zalloc = Z_NULL;
    zs.zfree = Z_NULL;
    zs.opaque = Z_NULL;
    if (deflateInit2(&zs, Z_DEFAULT_COMPRESSION, Z_DEFLATED, 15 + 16, 8, Z_DEFAULT_STRATEGY) != Z_OK) {
        return -1;
    }
    zs.next_in = (Bytef *)input;
    zs.avail_in = len;
    zs.next_out = (Bytef *)output;
    zs.avail_out = *outlen_ptr;
    
    int ret = deflate(&zs, Z_FINISH);
    if (ret != Z_STREAM_END) { 
        deflateEnd(&zs);
        return (ret == Z_BUF_ERROR) ? -ENOBUFS : -1;
    }
    *outlen_ptr = zs.total_out;
    deflateEnd(&zs);
    return 0;
}

static int gzip_decompress_op(const char *input, size_t len, char *output, size_t *outlen_ptr) {
    z_stream zs;
    memset(&zs, 0, sizeof(zs));
    zs.zalloc = Z_NULL;
    zs.zfree = Z_NULL;
    zs.opaque = Z_NULL;

    if (inflateInit2(&zs, 15 + 16) != Z_OK) { 
        return -1;
    }
    zs.next_in = (Bytef *)input;
    zs.avail_in = len;
    zs.next_out = (Bytef *)output;
    zs.avail_out = *outlen_ptr; 
    
    int ret = inflate(&zs, Z_FINISH); 
    if (ret != Z_STREAM_END) { 
        inflateEnd(&zs);
        return (ret == Z_BUF_ERROR) ? -ENOBUFS : (ret == Z_DATA_ERROR ? -EBADMSG : -1);
    }
    *outlen_ptr = zs.total_out;
    inflateEnd(&zs);
    return 0;
}


static int parse_7sref_path(const char *path_in_7sref, char *area, char *filename) {
    const char *underscore = strchr(path_in_7sref, '_');
    if (!underscore) return -1;
    size_t area_len = underscore - path_in_7sref;
    if (area_len == 0 || area_len >= PATH_MAX_LEN) return -1; 
    strncpy(area, path_in_7sref, area_len);
    area[area_len] = '\0';
    
    const char *filename_start = underscore + 1;
    size_t filename_len = strlen(filename_start);
    if (filename_len == 0 || filename_len >= PATH_MAX_LEN) return -1;
    strcpy(filename, filename_start); 
    return 0;
}

static int mai_getattr(const char *path, struct stat *stbuf) {
    memset(stbuf, 0, sizeof(struct stat));

    if (strcmp(path, "/") == 0) {
        stbuf->st_mode = S_IFDIR | 0755;
        stbuf->st_nlink = 9; 
        return 0;
    }

    if (strcmp(path, "/starter") == 0 || strcmp(path, "/metro") == 0 ||
        strcmp(path, "/dragon") == 0 || strcmp(path, "/blackrose") == 0 ||
        strcmp(path, "/heaven") == 0 || strcmp(path, "/skystreet") == 0 ||
        strcmp(path, "/7sref") == 0) {
        stbuf->st_mode = S_IFDIR | 0755;
        stbuf->st_nlink = 2; 
        return 0;
    }

    char real_path[PATH_MAX_LEN];
    const char *sub_path = NULL; 

    if (strncmp(path, "/7sref/", 7) == 0) {
        char area[PATH_MAX_LEN], filename[PATH_MAX_LEN];
        if (parse_7sref_path(path + 7, area, filename) == 0) {
            char new_path[PATH_MAX_LEN];
            int len_written = snprintf(new_path, PATH_MAX_LEN, "/%s/%s", area, filename);
            if (len_written < 0) return -EIO; // snprintf error
            if ((size_t)len_written >= PATH_MAX_LEN) return -ENAMETOOLONG; // Truncation
            return mai_getattr(new_path, stbuf); 
        } else {
            return -ENOENT; 
        }
    }

    if (strncmp(path, "/starter/", 9) == 0) { sub_path = path + 9; get_starter(sub_path, real_path); }
    else if (strncmp(path, "/metro/", 7) == 0) { sub_path = path + 7; get_metro(sub_path, real_path); }
    else if (strncmp(path, "/dragon/", 8) == 0) { sub_path = path + 8; get_dragon(sub_path, real_path); }
    else if (strncmp(path, "/blackrose/", 11) == 0) { sub_path = path + 11; get_blackrose(sub_path, real_path); }
    else if (strncmp(path, "/heaven/", 8) == 0) { sub_path = path + 8; get_heaven(sub_path, real_path); }
    else if (strncmp(path, "/skystreet/", 11) == 0) { sub_path = path + 11; get_youth(sub_path, real_path); }
    else { return -ENOENT; }

    if (lstat(real_path, stbuf) == -1) return -errno;

    if (S_ISREG(stbuf->st_mode)) {
        if (strncmp(path, "/heaven/", 8) == 0) {
            if (stbuf->st_size >= AES_BLOCK_SIZE) {
                stbuf->st_size -= AES_BLOCK_SIZE; 
            } else {
                stbuf->st_size = 0; 
            }
        } else if (strncmp(path, "/skystreet/", 11) == 0) {
            int fd_tmp = open(real_path, O_RDONLY);
            if (fd_tmp != -1) {
                struct stat st_gz;
                if (fstat(fd_tmp, &st_gz) == 0 && st_gz.st_size > 0) {
                    char *compressed_data = malloc(st_gz.st_size);
                    if (compressed_data) {
                        if (read(fd_tmp, compressed_data, st_gz.st_size) == st_gz.st_size) {
                            size_t decomp_capacity = st_gz.st_size * 10 + 1024; 
                            char *decomp_data = malloc(decomp_capacity);
                            if (decomp_data) {
                                size_t actual_decomp_size = decomp_capacity;
                                if (gzip_decompress_op(compressed_data, st_gz.st_size, decomp_data, &actual_decomp_size) == 0) {
                                    stbuf->st_size = actual_decomp_size;
                                } 
                                free(decomp_data);
                            }
                        }
                        free(compressed_data);
                    }
                }
                close(fd_tmp);
            }
        }
    }
    return 0;
}

static int mai_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                      off_t offset, struct fuse_file_info *fi) {
    (void) offset; 
    (void) fi;     

    filler(buf, ".", NULL, 0);  
    filler(buf, "..", NULL, 0); 

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

    if (strcmp(path, "/7sref") == 0) {
        return 0;
    }

    char underlying_dir_path[PATH_MAX_LEN];
    const char *target_ext = NULL;

    if (strcmp(path, "/starter") == 0) { snprintf(underlying_dir_path, PATH_MAX_LEN, "%s", STARTER_DIR); target_ext = STARTER_EXT; }
    else if (strcmp(path, "/metro") == 0) { snprintf(underlying_dir_path, PATH_MAX_LEN, "%s", METRO_DIR); target_ext = METRO_EXT; }
    else if (strcmp(path, "/dragon") == 0) { snprintf(underlying_dir_path, PATH_MAX_LEN, "%s", DRAGON_DIR); target_ext = DRAGON_EXT; }
    else if (strcmp(path, "/blackrose") == 0) { snprintf(underlying_dir_path, PATH_MAX_LEN, "%s", BLACKROSE_DIR); target_ext = BLACKROSE_EXT; }
    else if (strcmp(path, "/heaven") == 0) { snprintf(underlying_dir_path, PATH_MAX_LEN, "%s", HEAVEN_DIR); target_ext = HEAVEN_EXT; }
    else if (strcmp(path, "/skystreet") == 0) { snprintf(underlying_dir_path, PATH_MAX_LEN, "%s", YOUTH_DIR); target_ext = YOUTH_EXT; }
    else { return -ENOENT; }
    
    ensure_directory_exists(underlying_dir_path); 

    DIR *dp = opendir(underlying_dir_path);
    if (dp == NULL) return -errno;

    struct dirent *de;
    while ((de = readdir(dp)) != NULL) {
        if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0) continue;

        char display_name[FILENAME_MAX]; 
        strncpy(display_name, de->d_name, FILENAME_MAX -1);
        display_name[FILENAME_MAX-1] = '\0';

        if (target_ext) {
            char *ext_ptr = strrchr(display_name, '.');
            if (ext_ptr && strcmp(ext_ptr, target_ext) == 0) {
                *ext_ptr = '\0'; 
            } else {
                continue;
            }
        }
        
        if (strlen(display_name) == 0) continue;
        if (filler(buf, display_name, NULL, 0)) break;
    }
    closedir(dp);
    return 0;
}

static int mai_open(const char *path, struct fuse_file_info *fi) {
    char real_path[PATH_MAX_LEN];
    const char *sub_path = NULL;

    if (strncmp(path, "/7sref/", 7) == 0) {
        char area[PATH_MAX_LEN], filename[PATH_MAX_LEN];
        if (parse_7sref_path(path + 7, area, filename) == 0) {
            char new_path[PATH_MAX_LEN];
            int len_written = snprintf(new_path, PATH_MAX_LEN, "/%s/%s", area, filename);
            if (len_written < 0) return -EIO;
            if ((size_t)len_written >= PATH_MAX_LEN) return -ENAMETOOLONG;
            return mai_open(new_path, fi);
        } else { return -ENOENT; }
    }

    if (strncmp(path, "/starter/", 9) == 0) { sub_path = path + 9; get_starter(sub_path, real_path); }
    else if (strncmp(path, "/metro/", 7) == 0) { sub_path = path + 7; get_metro(sub_path, real_path); }
    else if (strncmp(path, "/dragon/", 8) == 0) { sub_path = path + 8; get_dragon(sub_path, real_path); }
    else if (strncmp(path, "/blackrose/", 11) == 0) { sub_path = path + 11; get_blackrose(sub_path, real_path); }
    else if (strncmp(path, "/heaven/", 8) == 0) { sub_path = path + 8; get_heaven(sub_path, real_path); }
    else if (strncmp(path, "/skystreet/", 11) == 0) { sub_path = path + 11; get_youth(sub_path, real_path); }
    else { return -ENOENT; }

    int fd = open(real_path, fi->flags);
    if (fd == -1) return -errno;
    
    fi->fh = fd; 
    return 0;
}


static int mai_read(const char *path, char *buf, size_t size, off_t offset,
                   struct fuse_file_info *fi) {
    int fd = -1;
    int res;
    char real_path_for_direct_open[PATH_MAX_LEN]; 
    int must_close_fd = 0;

    if (strncmp(path, "/7sref/", 7) == 0) {
        char area[PATH_MAX_LEN], filename[PATH_MAX_LEN];
        if (parse_7sref_path(path + 7, area, filename) == 0) {
            char new_path[PATH_MAX_LEN];
            int len_written = snprintf(new_path, PATH_MAX_LEN, "/%s/%s", area, filename);
            if (len_written < 0) return -EIO;
            if ((size_t)len_written >= PATH_MAX_LEN) return -ENAMETOOLONG;
            return mai_read(new_path, buf, size, offset, fi);
        } else { return -ENOENT; }
    }

    if (fi != NULL && fi->fh != 0 && fi->fh != (uint64_t)-1) { 
        fd = fi->fh;
    } else {
        const char *sub_path_rd = NULL; 
        if (strncmp(path, "/starter/", 9) == 0) { sub_path_rd = path + 9; get_starter(sub_path_rd, real_path_for_direct_open); }
        else if (strncmp(path, "/metro/", 7) == 0) { sub_path_rd = path + 7; get_metro(sub_path_rd, real_path_for_direct_open); }
        else if (strncmp(path, "/dragon/", 8) == 0) { sub_path_rd = path + 8; get_dragon(sub_path_rd, real_path_for_direct_open); }
        else if (strncmp(path, "/blackrose/", 11) == 0) { sub_path_rd = path + 11; get_blackrose(sub_path_rd, real_path_for_direct_open); }
        else if (strncmp(path, "/heaven/", 8) == 0) { sub_path_rd = path + 8; get_heaven(sub_path_rd, real_path_for_direct_open); }
        else if (strncmp(path, "/skystreet/", 11) == 0) { sub_path_rd = path + 11; get_youth(sub_path_rd, real_path_for_direct_open); }
        else return -ENOENT;
        
        fd = open(real_path_for_direct_open, O_RDONLY);
        if (fd == -1) return -errno;
        must_close_fd = 1;
    }

    if (strncmp(path, "/starter/", 9) == 0 || strncmp(path, "/blackrose/", 11) == 0) {
        res = pread(fd, buf, size, offset);
        if (res == -1) res = -errno;
    } else if (strncmp(path, "/metro/", 7) == 0) {
        char *temp_buf = malloc(size);
        if (!temp_buf) { res = -ENOMEM; goto end_read_label; }
        res = pread(fd, temp_buf, size, offset);
        if (res > 0) metro_decrypt(temp_buf, buf, res);
        else if (res == -1) res = -errno;
        free(temp_buf);
    } else if (strncmp(path, "/dragon/", 8) == 0) {
        res = pread(fd, buf, size, offset); 
        if (res > 0) rot13(buf, res);
        else if (res == -1) res = -errno;
    } else if (strncmp(path, "/heaven/", 8) == 0) {
        unsigned char iv[AES_BLOCK_SIZE];
        if (pread(fd, iv, AES_BLOCK_SIZE, 0) != AES_BLOCK_SIZE) { res = -EIO; goto end_read_label; }

        char *encrypted_data = malloc(size); 
        if (!encrypted_data) { res = -ENOMEM; goto end_read_label; }

        ssize_t encrypted_read_len = pread(fd, encrypted_data, size, offset + AES_BLOCK_SIZE);
        if (encrypted_read_len < 0) { res = -errno; free(encrypted_data); goto end_read_label; }
        
        if (encrypted_read_len == 0) { res = 0; free(encrypted_data); goto end_read_label; }

        size_t decrypted_len = size; 
        if (aes_operation(encrypted_data, encrypted_read_len, buf, &decrypted_len, iv, 0) != 0) {
             res = -EIO; 
        } else {
            res = decrypted_len;
        }
        free(encrypted_data);
    } else if (strncmp(path, "/skystreet/", 11) == 0) {
        struct stat st_gz_read;
        if (fstat(fd, &st_gz_read) == -1) { res = -errno; goto end_read_label; }
        if (st_gz_read.st_size == 0) { res = 0; goto end_read_label; } 

        char *compressed_data = malloc(st_gz_read.st_size);
        if (!compressed_data) { res = -ENOMEM; goto end_read_label; }
        if (pread(fd, compressed_data, st_gz_read.st_size, 0) != st_gz_read.st_size) {
            res = -EIO; free(compressed_data); goto end_read_label;
        }
        
        size_t decomp_capacity = st_gz_read.st_size * 10 + 1024; 
        char *decomp_total_data = malloc(decomp_capacity);
        if (!decomp_total_data) { res = -ENOMEM; free(compressed_data); goto end_read_label; }
        
        size_t actual_decomp_size = decomp_capacity;
        if (gzip_decompress_op(compressed_data, st_gz_read.st_size, decomp_total_data, &actual_decomp_size) != 0) {
            res = -EIO; 
        } else {
            if (offset >= (off_t)actual_decomp_size) {
                res = 0; 
            } else {
                size_t bytes_to_copy = actual_decomp_size - offset;
                if (bytes_to_copy > size) bytes_to_copy = size;
                memcpy(buf, decomp_total_data + offset, bytes_to_copy);
                res = bytes_to_copy;
            }
        }
        free(decomp_total_data);
        free(compressed_data);
    } else {
        res = -ENOENT;
    }

end_read_label:
    if (must_close_fd && fd != -1) close(fd);
    return res;
}

static int mai_write(const char *path, const char *buf, size_t size, off_t offset,
                    struct fuse_file_info *fi) {
    int fd = -1;
    int res;
    char real_path_for_direct_open[PATH_MAX_LEN];
    int must_close_fd = 0;

    if (strncmp(path, "/7sref/", 7) == 0) {
        char area[PATH_MAX_LEN], filename[PATH_MAX_LEN];
        if (parse_7sref_path(path + 7, area, filename) == 0) {
            char new_path[PATH_MAX_LEN];
            int len_written = snprintf(new_path, PATH_MAX_LEN, "/%s/%s", area, filename);
            if (len_written < 0) return -EIO;
            if ((size_t)len_written >= PATH_MAX_LEN) return -ENAMETOOLONG;
            return mai_write(new_path, buf, size, offset, fi);
        } else { return -ENOENT; }
    }

    if (fi != NULL && fi->fh != 0 && fi->fh != (uint64_t)-1) {
        fd = fi->fh;
    } else {
        const char *sub_path_wr = NULL; 
        if (strncmp(path, "/starter/", 9) == 0) { sub_path_wr = path + 9; get_starter(sub_path_wr, real_path_for_direct_open); }
        else if (strncmp(path, "/metro/", 7) == 0) { sub_path_wr = path + 7; get_metro(sub_path_wr, real_path_for_direct_open); }
        else if (strncmp(path, "/dragon/", 8) == 0) { sub_path_wr = path + 8; get_dragon(sub_path_wr, real_path_for_direct_open); }
        else if (strncmp(path, "/blackrose/", 11) == 0) { sub_path_wr = path + 11; get_blackrose(sub_path_wr, real_path_for_direct_open); }
        else if (strncmp(path, "/heaven/", 8) == 0) { sub_path_wr = path + 8; get_heaven(sub_path_wr, real_path_for_direct_open); }
        else if (strncmp(path, "/skystreet/", 11) == 0) { sub_path_wr = path + 11; get_youth(sub_path_wr, real_path_for_direct_open); }
        else return -ENOENT;
        
        fd = open(real_path_for_direct_open, O_WRONLY | O_CREAT, 0644); 
        if (fd == -1) return -errno;
        must_close_fd = 1;
    }

    if (strncmp(path, "/starter/", 9) == 0 || strncmp(path, "/blackrose/", 11) == 0) {
        res = pwrite(fd, buf, size, offset);
        if (res == -1) res = -errno;
    } else if (strncmp(path, "/metro/", 7) == 0) {
        char *transformed_buf = malloc(size);
        if (!transformed_buf) { res = -ENOMEM; goto end_write_label; }
        metro_encrypt(buf, transformed_buf, size);
        res = pwrite(fd, transformed_buf, size, offset);
        if (res == -1) res = -errno;
        free(transformed_buf);
    } else if (strncmp(path, "/dragon/", 8) == 0) {
        char *transformed_buf = malloc(size);
        if (!transformed_buf) { res = -ENOMEM; goto end_write_label; }
        memcpy(transformed_buf, buf, size); 
        rot13(transformed_buf, size);
        res = pwrite(fd, transformed_buf, size, offset);
        if (res == -1) res = -errno;
        free(transformed_buf);
    } else if (strncmp(path, "/heaven/", 8) == 0) {
        unsigned char iv[AES_BLOCK_SIZE];
        size_t encrypted_data_capacity = size + AES_BLOCK_SIZE; 
        char *encrypted_data = malloc(encrypted_data_capacity);
        if (!encrypted_data) { res = -ENOMEM; goto end_write_label; }

        if (offset == 0) { 
            RAND_bytes(iv, AES_BLOCK_SIZE);
            if (pwrite(fd, iv, AES_BLOCK_SIZE, 0) != AES_BLOCK_SIZE) {
                res = -EIO; free(encrypted_data); goto end_write_label;
            }
        } else {
            if (pread(fd, iv, AES_BLOCK_SIZE, 0) != AES_BLOCK_SIZE) {
                res = -EIO; free(encrypted_data); goto end_write_label;
            }
        }
        
        size_t actual_encrypted_len = encrypted_data_capacity;
        if (aes_operation(buf, size, encrypted_data, &actual_encrypted_len, iv, 1) != 0) {
            res = -EIO; 
        } else {
            res = pwrite(fd, encrypted_data, actual_encrypted_len, offset + AES_BLOCK_SIZE);
            if (res == -1) res = -errno;
        }
        free(encrypted_data);
    } else if (strncmp(path, "/skystreet/", 11) == 0) {
        if (offset == 0) { 
            size_t compressed_out_capacity = size * 2 + 1024; 
            char *compressed_data = malloc(compressed_out_capacity);
            if (!compressed_data) { res = -ENOMEM; goto end_write_label; }

            size_t actual_compressed_len = compressed_out_capacity;
            if (gzip_compress_op(buf, size, compressed_data, &actual_compressed_len) != 0) {
                res = -EIO; 
            } else {
                if (ftruncate(fd, 0) == -1) { res = -errno; free(compressed_data); goto end_write_label; }
                res = pwrite(fd, compressed_data, actual_compressed_len, 0); 
                if (res == -1) res = -errno;
            }
            free(compressed_data);
        } else {
            res = -EINVAL; 
        }
    } else {
        res = -ENOENT;
    }

end_write_label:
    if (must_close_fd && fd != -1) close(fd);
    return res;
}

static int mai_unlink(const char *path) {
    char real_path[PATH_MAX_LEN];
    const char *sub_path = NULL;

    if (strncmp(path, "/7sref/", 7) == 0) {
        char area[PATH_MAX_LEN], filename[PATH_MAX_LEN];
        if (parse_7sref_path(path + 7, area, filename) == 0) {
            char new_path[PATH_MAX_LEN];
            int len_written = snprintf(new_path, PATH_MAX_LEN, "/%s/%s", area, filename);
            if (len_written < 0) return -EIO;
            if ((size_t)len_written >= PATH_MAX_LEN) return -ENAMETOOLONG;
            return mai_unlink(new_path);
        } else { return -ENOENT; }
    }
    
    if (strncmp(path, "/starter/", 9) == 0) { sub_path = path + 9; get_starter(sub_path, real_path); }
    else if (strncmp(path, "/metro/", 7) == 0) { sub_path = path + 7; get_metro(sub_path, real_path); }
    else if (strncmp(path, "/dragon/", 8) == 0) { sub_path = path + 8; get_dragon(sub_path, real_path); }
    else if (strncmp(path, "/blackrose/", 11) == 0) { sub_path = path + 11; get_blackrose(sub_path, real_path); }
    else if (strncmp(path, "/heaven/", 8) == 0) { sub_path = path + 8; get_heaven(sub_path, real_path); }
    else if (strncmp(path, "/skystreet/", 11) == 0) { sub_path = path + 11; get_youth(sub_path, real_path); }
    else return -ENOENT;

    if (unlink(real_path) == -1) return -errno;
    return 0;
}

static int mai_truncate(const char *path, off_t size) {
    char real_path[PATH_MAX_LEN];
    const char *sub_path = NULL;
    int res_trunc;

    if (strncmp(path, "/7sref/", 7) == 0) {
        char area[PATH_MAX_LEN], filename[PATH_MAX_LEN];
        if (parse_7sref_path(path + 7, area, filename) == 0) {
            char new_path[PATH_MAX_LEN];
            int len_written = snprintf(new_path, PATH_MAX_LEN, "/%s/%s", area, filename);
            if (len_written < 0) return -EIO;
            if ((size_t)len_written >= PATH_MAX_LEN) return -ENAMETOOLONG;
            return mai_truncate(new_path, size);
        } else { return -ENOENT; }
    }
    
    if (strncmp(path, "/starter/", 9) == 0) { sub_path = path + 9; get_starter(sub_path, real_path); }
    else if (strncmp(path, "/metro/", 7) == 0) { sub_path = path + 7; get_metro(sub_path, real_path); }
    else if (strncmp(path, "/dragon/", 8) == 0) { sub_path = path + 8; get_dragon(sub_path, real_path); }
    else if (strncmp(path, "/blackrose/", 11) == 0) { sub_path = path + 11; get_blackrose(sub_path, real_path); }
    else if (strncmp(path, "/heaven/", 8) == 0) { sub_path = path + 8; get_heaven(sub_path, real_path); }
    else if (strncmp(path, "/skystreet/", 11) == 0) { sub_path = path + 11; get_youth(sub_path, real_path); }
    else return -ENOENT;

    if (strncmp(path, "/starter/", 9) == 0 || strncmp(path, "/blackrose/", 11) == 0) {
        res_trunc = truncate(real_path, size);
        if (res_trunc == -1) return -errno;
        return 0;
    }

    if (size == 0) {
        off_t underlying_truncate_size = 0;
        if (strncmp(path, "/heaven/", 8) == 0) {
            underlying_truncate_size = 0; 
        }
        res_trunc = truncate(real_path, underlying_truncate_size);
        if (res_trunc == -1 && errno != ENOENT) return -errno; 
        return 0;
    } else {
        return -EOPNOTSUPP; 
    }
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
    umask(0); 
    return fuse_main(argc, argv, &mai_oper, NULL);
}