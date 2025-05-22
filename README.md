# Sisop-4-2025-IT04

KELOMPOK IT04
| No | Nama                              | NRP         |
|----|-----------------------------------|-------------|
| 1  | Raya Ahmad Syarif                 | 5027241041  |
| 2  | Salsa Bil Ulla         | 5027241052 |
| 3  | Adinda Cahya Pramesti   | 5027241117  |

# Pengantar

Laporan resmi ini dibuat untuk praktikum modul 4. Praktikum Modul 4 ini terdiri dari empat soal dan dikerjakan oleh tiga orang anggota dengan pembagian tertentu.

Berikut ini pembagian pengerjaan dari kelompok IT04:

    Soal 1 dikerjakan oleh Salsa Bil Ulla
    Soal 2 dikerjakan oleh Adinda Cahya Pramesti
    Soal 3 dikerjakan oleh Salsa Bil Ulla
    Soal 4 dikerjakan oleh Raya Ahmad Syarif

Sehingga dengan demikian, Pembagian bobot pengerjaan soal menjadi (Raya 25%, Salsa 50%, Dinda 25%).

Kelompok IT04 juga telah menyelesaikan tugas praktikum modul 4 yang telah diberikan. Dari hasil praktikum yang telah dilakukan sebelumnya, maka diperoleh hasil sebagaimana yang dituliskan pada setiap bab di bawah ini.
# Ketentuan
Berikut ini struktur dari repositori praktikum modul 4:

```
Struktur repository seperti berikut:
			—soal_1:
				— hexed.c
                                  
			—soal_2:
— baymax.c
			
—soal_3:	
	— Dockerfile
	— docker-compose.yml
	— antink.c

			—soal_4:
—maimai_fs.c

			—assets
```
## SOAL NOMOR 2

Pada suatu hari, seorang ilmuwan muda menemukan sebuah drive tua yang tertanam di reruntuhan laboratorium robotik. Saat diperiksa, drive tersebut berisi pecahan data dari satu-satunya robot perawat legendaris yang dikenal dengan nama Baymax. Sayangnya, akibat kerusakan sistem selama bertahun-tahun, file utuh Baymax telah terfragmentasi menjadi 14 bagian kecil, masing-masing berukuran 1 kilobyte, dan tersimpan dalam direktori bernama relics. Pecahan tersebut diberi nama berurutan seperti Baymax.jpeg.000, Baymax.jpeg.001, hingga Baymax.jpeg.013. Ilmuwan tersebut kini ingin membangkitkan kembali Baymax ke dalam bentuk digital yang utuh, namun ia tidak ingin merusak file asli yang telah rapuh tersebut.

Sistem ini terdiri dari struktur:
- baymax.c

#### > A
Sebagai asisten teknis, tugasmu adalah membuat sebuah sistem file virtual menggunakan FUSE (Filesystem in Userspace) yang dapat membantu sang ilmuwan. Buatlah sebuah direktori mount bernama bebas (misalnya mount_dir) yang merepresentasikan tampilan Baymax dalam bentuk file utuh Baymax.jpeg. File sistem tersebut akan mengambil data dari folder relics sebagai sumber aslinya.
```
├── mount_dir
├── relics
│   ├── Baymax.jpeg.000
│   ├── Baymax.jpeg.001
│   ├── dst dst…
│   └── Baymax.jpeg.013
└── activity.log
```

#### > B
Ketika direktori FUSE diakses, pengguna hanya akan melihat Baymax.jpeg seolah-olah tidak pernah terpecah, meskipun aslinya terdiri dari potongan `.000` hingga `.013`. File `Baymax.jpeg` tersebut dapat dibaca, ditampilkan, dan disalin sebagaimana file gambar biasa, hasilnya merupakan gabungan sempurna dari keempat belas pecahan tersebut.

#### > C
Namun sistem ini harus lebih dari sekadar menyatukan. Jika pengguna membuat file baru di dalam direktori FUSE, maka sistem harus secara otomatis memecah file tersebut ke dalam potongan-potongan berukuran maksimal 1 KB, dan menyimpannya di direktori `relics` menggunakan format `[namafile].000`, `[namafile].001`, dan seterusnya. 

#### > D
Ketika file tersebut dihapus dari direktori `mount`, semua pecahannya di `relics` juga harus ikut dihapus.

#### > E
Untuk keperluan analisis ilmuwan, sistem juga harus mencatat seluruh aktivitas pengguna dalam sebuah file log bernama `activity.log` yang disimpan di direktori yang sama. Aktivitas yang dicatat antara lain:
Membaca file (misalnya membuka baymax.png)
Membuat file baru (termasuk nama file dan jumlah pecahan)
Menghapus file (termasuk semua pecahannya yang terhapus)
Menyalin file (misalnya cp baymax.png /tmp/)
Contoh Log :
``` 
[2025-05-11 10:24:01] READ: Baymax.jpeg
[2025-05-11 10:25:14] WRITE: hero.txt -> hero.txt.000, hero.txt.001
[2025-05-11 10:26:03] DELETE: Baymax.jpeg.000 - Baymax.jpeg.013
[2025-05-11 10:27:45] COPY: Baymax.jpeg -> /tmp/Baymax.jpeg
```
### JAWAB

- #### KODE baymax.c
```
   #define FUSE_USE_VERSION 26

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <time.h>
#include <stdarg.h>

#define RELICS_DIR "./relics"
#define LOG_FILE "./activity.log"
#define PIECE_SIZE 1024

static void write_log(const char *format, ...) {
    FILE *logf = fopen(LOG_FILE, "a");
    if (!logf) return;

    time_t now = time(NULL);
    struct tm *t = localtime(&now);

    char timestr[32];
    strftime(timestr, sizeof(timestr), "[%Y-%m-%d %H:%M:%S]", t);

    fprintf(logf, "%s ", timestr);

    va_list args;
    va_start(args, format);
    vfprintf(logf, format, args);
    va_end(args);

    fprintf(logf, "\n");
    fclose(logf);
}

static int baymax_getattr(const char *path, struct stat *stbuf) {
    memset(stbuf, 0, sizeof(struct stat));

    if (strcmp(path, "/") == 0) {
        stbuf->st_mode = S_IFDIR | 0755;
        stbuf->st_nlink = 2;
        stbuf->st_uid = getuid();
        stbuf->st_gid = getgid();
        return 0;
    }

    const char *filename = path + 1;
    char piece_path[PATH_MAX];
    int found = 0;

    // Check if any piece exists
    for (int i = 0; i < 1000; i++) {
        snprintf(piece_path, sizeof(piece_path), "%s/%s.%03d", RELICS_DIR, filename, i);
        if (access(piece_path, F_OK) == 0) {
            found = 1;
            break;
        }
    }

    if (!found) return -ENOENT;

    stbuf->st_mode = S_IFREG | 0644;
    stbuf->st_nlink = 1;
    stbuf->st_uid = getuid();
    stbuf->st_gid = getgid();
    
    // Calculate total size
    off_t total_size = 0;
    for (int i = 0; i < 1000; i++) {
        snprintf(piece_path, sizeof(piece_path), "%s/%s.%03d", RELICS_DIR, filename, i);
        struct stat st;
        if (stat(piece_path, &st) == 0) {
            total_size += st.st_size;
        } else {
            break;
        }
    }
    stbuf->st_size = total_size;

    return 0;
}

static int baymax_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                         off_t offset, struct fuse_file_info *fi) {
    (void) offset;
    (void) fi;

    if (strcmp(path, "/") != 0) return -ENOENT;

    filler(buf, ".", NULL, 0);
    filler(buf, "..", NULL, 0);

    DIR *dir = opendir(RELICS_DIR);
    if (!dir) return -EIO;

    struct dirent *entry;
    char *last_name = NULL;
    int count = 0;

    while ((entry = readdir(dir))) {
        if (entry->d_name[0] == '.') continue;

        char *dot = strrchr(entry->d_name, '.');
        if (!dot) continue;
        
        if (strlen(dot) == 4 && isdigit(dot[1]) && isdigit(dot[2]) && isdigit(dot[3])) {
            char base_name[256];
            strncpy(base_name, entry->d_name, dot - entry->d_name);
            base_name[dot - entry->d_name] = '\0';

            if (last_name && strcmp(last_name, base_name) == 0) continue;
            
            if (last_name) free(last_name);
            last_name = strdup(base_name);
            
            filler(buf, base_name, NULL, 0);
            count++;
        }
    }

    if (last_name) free(last_name);
    closedir(dir);
    return 0;
}

static int baymax_open(const char *path, struct fuse_file_info *fi) {
    char piece_path[512];
    snprintf(piece_path, sizeof(piece_path), "%s/%s.000", RELICS_DIR, path + 1);
    if (access(piece_path, F_OK) != 0) return -ENOENT;

    write_log("READ: %s", path + 1);
    return 0;
}

static int baymax_read(const char *path, char *buf, size_t size, off_t offset,
                       struct fuse_file_info *fi) {
    (void) fi;

    const char *filename = path + 1;
    size_t bytes_read = 0;
    char piece_path[512];

    for (int i = 0; size > 0; i++) {
        snprintf(piece_path, sizeof(piece_path), "%s/%s.%03d", RELICS_DIR, filename, i);
        FILE *f = fopen(piece_path, "rb");
        if (!f) break;

        fseek(f, 0, SEEK_END);
        size_t piece_size = ftell(f);
        fseek(f, 0, SEEK_SET);

        if (offset >= piece_size) {
            offset -= piece_size;
            fclose(f);
            continue;
        }

        size_t to_read = piece_size - offset;
        if (to_read > size) to_read = size;

        fseek(f, offset, SEEK_SET);
        fread(buf + bytes_read, 1, to_read, f);
        fclose(f);

        bytes_read += to_read;
        size -= to_read;
        offset = 0;
    }

    if (bytes_read > 0) {
        write_log("READ: %s (potential COPY)", filename);
    }

    return bytes_read;
}

static int baymax_unlink(const char *path) {
    const char *filename = path + 1;
    char piece_path[512];
    char log_msg[1024];
    snprintf(log_msg, sizeof(log_msg), "DELETE: ");
    int count = 0;

    for (int i = 0;; i++) {
        snprintf(piece_path, sizeof(piece_path), "%s/%s.%03d", RELICS_DIR, filename, i);
        if (access(piece_path, F_OK) != 0) break;
        if (unlink(piece_path) == 0) {
            char piece[32];
            snprintf(piece, sizeof(piece), "%s.%03d", filename, i);
            strncat(log_msg, piece, sizeof(log_msg) - strlen(log_msg) - 1);
            if (i < 1000) strncat(log_msg, ", ", sizeof(log_msg) - strlen(log_msg) - 1);
            count++;
        }
    }

    if (count == 0) return -ENOENT;
    write_log("%s", log_msg);
    return 0;
}

static int split_file_to_pieces(const char *filepath, const char *filename) {
    FILE *src = fopen(filepath, "rb");
    if (!src) return -ENOENT;

    char piece_path[512];
    char buffer[PIECE_SIZE];
    int index = 0;

    while (1) {
        size_t n = fread(buffer, 1, PIECE_SIZE, src);
        if (n == 0) break;

        snprintf(piece_path, sizeof(piece_path), "%s/%s.%03d", RELICS_DIR, filename, index);
        FILE *dest = fopen(piece_path, "wb");
        if (!dest) {
            fclose(src);
            return -EIO;
        }

        fwrite(buffer, 1, n, dest);
        fclose(dest);
        index++;
    }
    fclose(src);
    return index;
}

static int baymax_create(const char *path, mode_t mode, struct fuse_file_info *fi) {
    const char *filename = path + 1;
    char tmp_path[512];
    snprintf(tmp_path, sizeof(tmp_path), "%s/%s.tmp", RELICS_DIR, filename);
    FILE *f = fopen(tmp_path, "wb");
    if (!f) return -EIO;
    fclose(f);
    write_log("CREATE: %s", filename);
    return 0;
}

static int baymax_write(const char *path, const char *buf, size_t size,
                        off_t offset, struct fuse_file_info *fi) {
    const char *filename = path + 1;
    char tmp_path[512];
    snprintf(tmp_path, sizeof(tmp_path), "%s/%s.tmp", RELICS_DIR, filename);
    FILE *f = fopen(tmp_path, "r+b");
    if (!f) f = fopen(tmp_path, "wb");
    if (!f) return -EIO;
    fseek(f, offset, SEEK_SET);
    size_t written = fwrite(buf, 1, size, f);
    fclose(f);
    return written;
}

static int baymax_flush(const char *path, struct fuse_file_info *fi) {
    const char *filename = path + 1;
    char tmp_path[512];
    snprintf(tmp_path, sizeof(tmp_path), "%s/%s.tmp", RELICS_DIR, filename);
    int pieces = split_file_to_pieces(tmp_path, filename);
    if (pieces < 0) return pieces;
    unlink(tmp_path);

    char log_msg[1024];
    snprintf(log_msg, sizeof(log_msg), "WRITE: %s -> ", filename);
    for (int i = 0; i < pieces; i++) {
        char piece[32];
        snprintf(piece, sizeof(piece), "%s.%03d", filename, i);
        strncat(log_msg, piece, sizeof(log_msg) - strlen(log_msg) - 1);
        if (i < pieces - 1) strncat(log_msg, ", ", sizeof(log_msg) - strlen(log_msg) - 1);
    }
    write_log("%s", log_msg);
    return 0;
}

static struct fuse_operations baymax_oper = {
    .getattr = baymax_getattr,
    .readdir = baymax_readdir,
    .open = baymax_open,
    .read = baymax_read,
    .unlink = baymax_unlink,
    .create = baymax_create,
    .write = baymax_write,
    .flush = baymax_flush,
};

int main(int argc, char *argv[]) {
    struct stat st = {0};
    if (stat(RELICS_DIR, &st) == -1) {
        mkdir(RELICS_DIR, 0755);
    }
    write_log("Baymax filesystem started");
    return fuse_main(argc, argv, &baymax_oper, NULL);
}
```
### Dokumentasi
![image](https://github.com/user-attachments/assets/135f0991-cc33-4194-b06e-6a1ef0f3a214)

