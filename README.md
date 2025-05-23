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
## SOAL NOMOR 1
The Shorekeeper adalah sebuah entitas misterius yang memimpin dan menjaga Black Shores secara keseluruhan. Karena Shorekeeper hanya berada di Black Shores, ia biasanya berjalan - jalan di sekitar Black Shores untuk mencari anomali - anomali yang ada untuk mencegah adanya kekacauan ataupun krisis di Black Shores. Semenjak kemunculan Fallacy of No Return, ia semakin ketat dalam melakukan pencarian anomali - anomali yang ada di Black Shores untuk mencegah hal yang sama terjadi lagi.
Suatu hari, saat di Tethys' Deep, Shorekeeper menemukan sebuah anomali yang baru diketahui. Anomali ini berupa sebuah teks acak yang kelihatannya tidak memiliki arti. Namun, ia mempunyai ide untuk mencari arti dari teks acak tersebut. [Author: Haidar / **scar / hemorrhager / 恩赫勒夫**]
a. Pertama, Shorekeeper akan **mengambil** beberapa sampel anomali teks dari link berikut. Pastikan **file zip terhapus** setelah proses **unzip**.
b. Setelah melihat teks - teks yang didapatkan, ia menyadari bahwa format teks tersebut adalah **hexadecimal**. Dengan informasi tersebut, Shorekeeper mencoba untuk mencoba idenya untuk mencari makna dari teks - teks acak tersebut, yaitu dengan **mengubahnya** dari **string hexadecimal** menjadi sebuah **file image**. Bantulah Shorekeeper dengan membuat kode untuk **FUSE** yang dapat **mengubah string hexadecimal** menjadi sebuah **gambar** ketika file text tersebut **dibuka** di **mount directory**. Lalu, letakkan hasil gambar yang didapat ke dalam **directory** bernama **“image”**.
c. Untuk penamaan file hasil konversi dari string ke image adalah **[nama file string]_image_[YYYY-mm-dd]_[HH:MM:SS]**.
Contoh:
1_image_2025-05-11_18:35:26.png
d. Catat setiap **konversi** yang ada ke dalam sebuah log file bernama **conversion.log**. Untuk formatnya adalah sebagai berikut.
[YYYY-mm-dd][HH:MM:SS]: Successfully converted hexadecimal text [nama file string] to [nama file image].
Contoh:
```
[2025-05-11][18:35:26]: Successfully converted hexadecimal text 1.txt to 1_image_2025-05-11_18:35:26.png.
[2025-05-11][18:35:27]: Successfully converted hexadecimal text 2.txt to 2_image_2025-05-11_18:35:27.png.
[2025-05-11][18:35:29]: Successfully converted hexadecimal text 3.txt to 3_image_2025-05-11_18:35:29.png.
[2025-05-11][18:35:32]: Successfully converted hexadecimal text 4.txt to 4_image_2025-05-11_18:35:32.png.
[2025-05-11][18:35:34]: Successfully converted hexadecimal text 5.txt to 5_image_2025-05-11_18:35:34.png.
[2025-05-11][18:35:36]: Successfully converted hexadecimal text 6.txt to 6_image_2025-05-11_18:35:36.png.
[2025-05-11][18:35:38]: Successfully converted hexadecimal text 7.txt to 7_image_2025-05-11_18:35:38.png.
```
Contoh struktur akhir adalah sebagai berikut.
![image](https://github.com/user-attachments/assets/3822beea-1e5b-4651-b49c-a8354fa275f9)

### > Penyelesaian
> [**hexed.c** sebelum revisi](https://github.com/rayahmd/Sisop-4-2025-IT04/blob/main/soal_1/hexed.c)

> **hexed.c** setelah revisi
```
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
#define ANOMALI_DIR "anomali"

static const char *fileId = "1hi_GDdP51Kn2JJMw02WmCOxuc3qrXzh5";
static const char *zipFileName = "anomali.zip";

void downloadFileFromDrive(const char *fileId, const char *outputFile) {
    char command[512];
    snprintf(command, sizeof(command),
             "wget --no-check-certificate 'https://docs.google.com/uc?export=download&id=%s' -O %s",
             fileId, outputFile);
    system(command);
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
    snprintf(command, sizeof(command), "unzip -o %s -d . && mv anomali/anomali/* %s && rm -r anomali", zipFile, ANOMALI_DIR);
    system(command);
    unlink(zipFile);
}

void hexToImage(const char *inputPath, const char *fileContent, size_t contentLength) {
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);

    char *hexStr = malloc(contentLength + 1);
    strncpy(hexStr, fileContent, contentLength);
    hexStr[contentLength] = '\0';

    cleanHex(hexStr);
    if (!isValidHex(hexStr) || strlen(hexStr) % 2 != 0) {
        free(hexStr);
        return;
    }

    char imageDir[256];
    snprintf(imageDir, sizeof(imageDir), "%s/image", ANOMALI_DIR);
    mkdir(imageDir, 0777);

    char outputFileName[256];
    const char *baseName = strrchr(inputPath, '/');
    baseName = baseName ? baseName + 1 : inputPath;
    
    snprintf(outputFileName, sizeof(outputFileName), 
             "%s/image/%.*s_image_%04d-%02d-%02d_%02d:%02d:%02d.png",
             ANOMALI_DIR,
             (int)(strstr(baseName, ".txt") - baseName), baseName,
             tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
             tm->tm_hour, tm->tm_min, tm->tm_sec);

    FILE *outFile = fopen(outputFileName, "wb");
    if (outFile) {
        for (int i = 0; hexStr[i] && hexStr[i + 1]; i += 2) {
            char byteStr[3] = {hexStr[i], hexStr[i + 1], '\0'};
            unsigned char byte = (unsigned char)strtol(byteStr, NULL, 16);
            fwrite(&byte, 1, 1, outFile);
        }
        fclose(outFile);
    }

    char logPath[256];
    snprintf(logPath, sizeof(logPath), "%s/conversion.log", ANOMALI_DIR);
    FILE *logFile = fopen(logPath, "a");
    if (logFile) {
        fprintf(logFile, "[%04d-%02d-%02d][%02d:%02d:%02d]: Successfully converted hexadecimal text %s to %s\n",
                tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
                tm->tm_hour, tm->tm_min, tm->tm_sec, 
                baseName, outputFileName);
        fclose(logFile);
    }

    free(hexStr);
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
    
    if (strstr(path, ".txt")) {
        char realPath[256];
        snprintf(realPath, sizeof(realPath), "%s%s", ANOMALI_DIR, path);
        if (stat(realPath, stbuf) == 0) {
            stbuf->st_mode = S_IFREG | 0644;
            return 0;
        }
    }
    
    if (strcmp(path, "/conversion.log") == 0) {
        char logPath[256];
        snprintf(logPath, sizeof(logPath), "%s/conversion.log", ANOMALI_DIR);
        if (stat(logPath, stbuf) == 0) {
            stbuf->st_mode = S_IFREG | 0644;
        } else {
            stbuf->st_mode = S_IFREG | 0644;
            stbuf->st_size = 0;
        }
        return 0;
    }
    
    if (strstr(path, "/image/") && strstr(path, ".png")) {
        char imagePath[256];
        snprintf(imagePath, sizeof(imagePath), "%s%s", ANOMALI_DIR, path);
        if (stat(imagePath, stbuf) == 0) {
            stbuf->st_mode = S_IFREG | 0644;
        } else {
            stbuf->st_mode = S_IFREG | 0644;
            stbuf->st_size = 0;
        }
        return 0;
    }
    
    return -ENOENT;
}

static int hexed_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                        off_t offset, struct fuse_file_info *fi) {
    filler(buf, ".", NULL, 0);
    filler(buf, "..", NULL, 0);

    if (strcmp(path, "/") == 0) {
        DIR *dp = opendir(ANOMALI_DIR);
        if (dp) {
            struct dirent *de;
            while ((de = readdir(dp)) != NULL) {
                if (strstr(de->d_name, ".txt")) {
                    filler(buf, de->d_name, NULL, 0);
                }
            }
            closedir(dp);
        }
        filler(buf, "image", NULL, 0);
        filler(buf, "conversion.log", NULL, 0);
    } 
    else if (strcmp(path, "/image") == 0) {
        char imageDir[256];
        snprintf(imageDir, sizeof(imageDir), "%s/image", ANOMALI_DIR);
        DIR *dp = opendir(imageDir);
        if (dp) {
            struct dirent *de;
            while ((de = readdir(dp)) != NULL) {
                if (strstr(de->d_name, ".png")) {
                    filler(buf, de->d_name, NULL, 0);
                }
            }
            closedir(dp);
        }
    }
    
    return 0;
}

static int hexed_open(const char *path, struct fuse_file_info *fi) {
    return 0;
}

static int hexed_read(const char *path, char *buf, size_t size, off_t offset,
                     struct fuse_file_info *fi) {
    char realPath[256];
    snprintf(realPath, sizeof(realPath), "%s%s", ANOMALI_DIR, path);

    FILE *file = fopen(realPath, "r");
    if (!file) return -ENOENT;

    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    rewind(file);

    if (offset >= fileSize) {
        fclose(file);
        return 0;
    }

    if (offset + size > fileSize) {
        size = fileSize - offset;
    }

    fseek(file, offset, SEEK_SET);
    size_t bytesRead = fread(buf, 1, size, file);

    if (strstr(path, ".txt") && offset == 0) {
        char *content = malloc(fileSize + 1);
        fseek(file, 0, SEEK_SET);
        fread(content, 1, fileSize, file);
        content[fileSize] = '\0';

        time_t t = time(NULL);
        struct tm *tm = localtime(&t);
        const char *baseName = strrchr(path, '/');
        baseName = baseName ? baseName + 1 : path;

        char outputFileName[256];
        snprintf(outputFileName, sizeof(outputFileName),
                 "%s/image/%.*s_image_%04d-%02d-%02d_%02d:%02d:%02d.png",
                 ANOMALI_DIR,
                 (int)(strstr(baseName, ".txt") - baseName), baseName,
                 tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
                 tm->tm_hour, tm->tm_min, tm->tm_sec);

        if (access(outputFileName, F_OK) != 0) {
            hexToImage(realPath, content, fileSize);
        }

        free(content);
    }

    fclose(file);
    return bytesRead;
}

static struct fuse_operations hexed_oper = {
    .getattr = hexed_getattr,
    .readdir = hexed_readdir,
    .open = hexed_open,
    .read = hexed_read,
};

int main(int argc, char *argv[]) {
    mkdir(ANOMALI_DIR, 0755);
    mkdir(DEFAULT_MOUNT_POINT, 0755);

    downloadFileFromDrive(fileId, zipFileName);
    unzipAndDelete(zipFileName);

    char imageDir[256];
    snprintf(imageDir, sizeof(imageDir), "%s/image", ANOMALI_DIR);
    mkdir(imageDir, 0777);

    char logPath[256];
    snprintf(logPath, sizeof(logPath), "%s/conversion.log", ANOMALI_DIR);
    FILE *logFile = fopen(logPath, "w");
    if (logFile) fclose(logFile);

    char *fuse_argv[] = {
        argv[0],
        "-f",
        DEFAULT_MOUNT_POINT
    };
    return fuse_main(3, fuse_argv, &hexed_oper, NULL);
}
```

### > Penjelasan
**i. Download file zip dari link Google Drive**
> Shorekeeper akan **mengambil** beberapa sampel anomali teks dari link berikut.
```
static const char *fileId = "1hi_GDdP51Kn2JJMw02WmCOxuc3qrXzh5";
...
void downloadFileFromDrive(const char *fileId, const char *outputFile) {
    char command[512];
    snprintf(command, sizeof(command),
             "wget --no-check-certificate 'https://docs.google.com/uc?export=download&id=%s' -O %s",
             fileId, outputFile);
    system(command);
}
```
Mengunduh `anomali.zip` dari Google Drive menggunakan `wget`.

**ii. Hapus file zip setelah proses unzip**
> Pastikan **file zip terhapus** setelah proses **unzip**.
```
void unzipAndDelete(const char *zipFile) {
    char command[512];
    snprintf(command, sizeof(command), "unzip -o %s -d . && mv anomali/anomali/* %s && rm -r anomali", zipFile, ANOMALI_DIR);
    system(command);
    unlink(zipFile);
}
```
File `anomali.zip` dihapus setelah isinya dipindahkan.

**iii. Konversi isi .txt (hexadecimal) menjadi gambar saat dibuka**
> ...mengubahnya dari string hexadecimal menjadi sebuah file image ketika file text tersebut dibuka di mount directory.
```
static int hexed_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
    ...
    if (strstr(path, ".txt") && offset == 0) {
        ...
        if (access(outputFileName, F_OK) != 0) {
            hexToImage(realPath, content, fileSize);
        }
        free(content);
    }
    fclose(file);
    return bytesRead;
}
```
Dalam `hexed_read()`, file `.txt` yang dibuat langsung dikonversi.

```
void hexToImage(const char *inputPath, const char *fileContent, size_t contentLength) {
    ...
    char *hexStr = malloc(contentLength + 1);
    strncpy(hexStr, fileContent, contentLength);
    hexStr[contentLength] = '\0';

    cleanHex(hexStr);
    if (!isValidHex(hexStr) || strlen(hexStr) % 2 != 0) {
        free(hexStr);
        return;
    }
    ...
    FILE *outFile = fopen(outputFileName, "wb");
    if (outFile) {
        for (int i = 0; hexStr[i] && hexStr[i + 1]; i += 2) {
            char byteStr[3] = {hexStr[i], hexStr[i + 1], '\0'};
            unsigned char byte = (unsigned char)strtol(byteStr, NULL, 16);
            fwrite(&byte, 1, 1, outFile);
        }
        fclose(outFile);
    }
    ...
    free(hexStr);
}
```
`hexStr` adalah string hasil bersih dari teks hexadecimal. Pada `for` akan membaca 2 karakter hex sekaligus dan membentuk jadi string dua digit (`byteStr[3]`) lalu diubah ke bentuk byte asli (`strtol(..., 16)`). Terakhir ditulis ke file `.png` menggunakan `fwrite()`.

**iv. Meletakkan hasil gambar ke dalam direktori image**
> ...letakkan hasil gambar yang didapat ke dalam directory bernama 'image'.
```
void hexToImage(const char *inputPath, const char *fileContent, size_t contentLength) {
    ...
    char imageDir[256];
    snprintf(imageDir, sizeof(imageDir), "%s/image", ANOMALI_DIR);
    mkdir(imageDir, 0777);
    ...
}
```
Dengan ini, gambar akan disimpan pada direktori baru yaitu `image`.

**v. Format penamaan gambar**
> Untuk penamaan file hasil konversi dari string ke image adalah **[nama file string]_image_[YYYY-mm-dd]_[HH:MM:SS]**.
```
void hexToImage(const char *inputPath, const char *fileContent, size_t contentLength) {
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    ...
    char outputFileName[256];
    const char *baseName = strrchr(inputPath, '/');
    baseName = baseName ? baseName + 1 : inputPath;
    
    snprintf(outputFileName, sizeof(outputFileName), 
             "%s/image/%.*s_image_%04d-%02d-%02d_%02d:%02d:%02d.png",
             ANOMALI_DIR,
             (int)(strstr(baseName, ".txt") - baseName), baseName,
             tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
             tm->tm_hour, tm->tm_min, tm->tm_sec);
    ...
}
```
Penamaan gambar sesuai dengan format soal.

**vi. Catat setiap konversi ke file conversion.log sesuai format**
> Catat setiap **konversi** yang ada ke dalam sebuah log file bernama **conversion.log**. Untuk formatnya adalah sebagai berikut. [YYYY-mm-dd][HH:MM:SS]: Successfully converted hexadecimal text [nama file string] to [nama file image].
```
void hexToImage(const char *inputPath, const char *fileContent, size_t contentLength) {
    ...
    char logPath[256];
    snprintf(logPath, sizeof(logPath), "%s/conversion.log", ANOMALI_DIR);
    FILE *logFile = fopen(logPath, "a");
    if (logFile) {
        fprintf(logFile, "[%04d-%02d-%02d][%02d:%02d:%02d]: Successfully converted hexadecimal text %s to %s\n",
                tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
                tm->tm_hour, tm->tm_min, tm->tm_sec, 
                baseName, outputFileName);
        fclose(logFile);
    }
    ...
```
File log `conversion.log` mencatat setiap proses konversi dari `.txt` ke `.png` dengan format log yang sudah ditetapkan.

### > Dokumentasi
> Sebelum revisi
![image](https://github.com/user-attachments/assets/fdc319b4-fccc-45e3-a373-2fdac86d3e22)
![image](https://github.com/user-attachments/assets/351a07a2-5dbb-46f8-9ba5-ce5cd0cf1155)

> Setelah revisi
![image](https://github.com/user-attachments/assets/f6c4c2a5-dd3a-4043-94e6-316a87dfe812)
![image](https://github.com/user-attachments/assets/2f9219f4-23d9-4829-a8fa-825fe5606219)
![image](https://github.com/user-attachments/assets/b567c6d7-5eaf-4cad-81fe-2046a3188af3)
![image](https://github.com/user-attachments/assets/721bd5d1-2265-43c4-988b-ccd98a5ea76a)
![image](https://github.com/user-attachments/assets/27add52a-b242-49b5-9972-87253c15abe5)
![image](https://github.com/user-attachments/assets/dd85e2c4-7bb2-4b89-9ecd-4358a4fe6250)
![image](https://github.com/user-attachments/assets/9faf57c7-1a06-4cfe-8e5b-04805e1c8a02)
![image](https://github.com/user-attachments/assets/cba0d21d-1234-495a-90e9-2b2a567f8555)
![image](https://github.com/user-attachments/assets/3085eeca-110f-4553-be86-473bc733bc69)
![image](https://github.com/user-attachments/assets/4a1c07ca-b2dd-4eb3-97be-9f3f1281d8da)
![image](https://github.com/user-attachments/assets/b6d61459-22d2-46aa-aa1a-d3c7eeb25d46)

#### Perbandingan kode sebelum dan sesudah revisi
| Aspek                              | `hexed.c` sebelum revisi                                                               | `hexed.c` setelah revisi                                                                     |
| ---------------------------------- | -------------------------------------------------------------------------------------- | -------------------------------------------------------------------------------------------- |
| **Deklarasi Global**               | `hexStr` global, `outputFileName`, dan `logFile` juga global                           | Semua variabel lokal di `hexToImage`, lebih bersih                                           |
| **Struktur Folder**                | Pakai folder `mnt` untuk hasil mount dan `anomali/` sebagai tempat file .txt dan image | Semua file tetap di dalam `anomali/`, termasuk image dan log                                 |
| **Cara unzip**                     | Unzip ke folder `temp`, lalu dipindah ke `anomali/`, lalu `temp` dihapus               | Unzip langsung ke root, lalu isi `anomali/anomali` dipindah ke `anomali` lalu folder dihapus |
| **Pemanggilan `hexToImage`**       | Hanya pada saat `read()` file `.txt`, langsung konversi ke image                       | Konversi hanya dilakukan kalau file `.png` belum ada (`access(...) != 0`)                    |
| **Logging**                        | File log dibuka dan ditulis di `hexToImage` dengan global pointer `logFile`            | File log dibuka dan ditutup langsung di dalam `hexToImage`                                   |
| **Main `fuse_main`**               | `argc = 2`, mount langsung ke `"mnt"` tanpa opsi                                       | `argc = 3`, mount ke `"mnt"` dengan opsi `-f` (foreground)                                   |
| **Ukuran file dummy di `getattr`** | Semua file `.txt`, `.png`, dan log diberi ukuran `1024` secara manual                  | `stat()` dipanggil untuk membaca ukuran asli file (lebih realistis)                          |
| **Handling direktori `image/`**    | Hanya dibuat jika belum ada                                                            | Selalu dibuat di `main()` dan `hexToImage` juga coba buat ulang                              |
| **Error handling download**        | `system(command)` diperiksa hasilnya, exit jika gagal                                  | Tidak dicek (lebih rawan error silent)                                                       |
| **Fungsi `hexToImage`**            | Membaca file dari disk langsung                                                        | Menerima `fileContent` sebagai parameter dari `read()` (lebih fleksibel dan efisien)         |

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

## SOAL NOMOR 3
**Tidak solve**. Dikarenakan sudah saya coba berkali-kali namun tetap tidak berjalan sesuai soal dan docker yang tidak berfungsi dengan baik. Ditambah dengan keterbatasan waktu saya saat mengerjakan, maka saya tidak dapat menyelesaikan soalnya dengan baik.

## SOAL NOMOR 4
```
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
```
Cuplikan code di atas merupakan STL yang digunakan dalam pengerjaan soal ini. FUSE yang digunakan adalah versi 3.1 yang bisa dilihat pada #define FUSE_USE_VERSION 31. Library <fuse.h> juga digunakan untuk FUSE. Lalu ada juga beberapa library standar C untuk operasi file, direktori, dan error management. libgen untuk manipulasi nama file dan openssl di situ untuk enkripsi AES. zlip dignakan untuk kompresi dan dekompresi data.

```
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
```
Define di atas digunakan untuk definisikan path virtual directory dan ekstensi file khusus seperti .mai, .ccc, .rot, .bin, dan .enc untuk berbagai macam fungsi. PATH_MAX_LEN digunakan untuk menetapkann panjang max path file yaitu 4096 character dan AES_BLOCK_SIZE untuk penetapan ukuran enkripsi AES.

```
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
```
Ensure directory exist untuk mengecek apakah path direktori sudah ada apa belum, jika belum akan dibaut direktorinya serta mengatur permissionnya.

```

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

```
Function mai_create digunakan untuk proses pembuatan file yaitu memetakan path FUSE ke path filesystem yang asli. 

```
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

```

Function ini untuk mengambil atribut file atau direktori. Fungsi ini pertama memeriksa apakah path adalah root (/) atau salah satu direktori virtual khusus seperti /starter, /metro, dll., dan mengatur mode direktori jika cocok. Jika path merupakan subdirektori seperti /starter/filename, ia memetakan ke path asli di filesystem menggunakan fungsi seperti get_starter(), lalu memanggil lstat untuk mendapatkan atribut file sebenarnya. Untuk direktori /heaven, ia mengurangi ukuran file sebesar AES_BLOCK_SIZE karena data enkripsi memiliki header tambahan. Untuk /skystreet, fungsi membaca dan mendekompresi file gzip untuk menentukan ukuran aslinya. 

### DOKUMENTASI
![Image](https://github.com/user-attachments/assets/5952f436-f428-45e3-8120-043711420810)  

[KENDALA] Mohon maaf sampai deadline berakhir, sudah diusahakan untuk perbaikan tetapi yang didapat hanya output permission denied🙏
