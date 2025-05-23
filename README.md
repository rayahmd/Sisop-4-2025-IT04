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
#### > Revised Code
```
#define FUSE_USE_VERSION 31
#define _XOPEN_SOURCE 700

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
#include <time.h>
#include <libgen.h> // For basename, though not explicitly used by user functions
#include <ctype.h>

#define FRAGMENT_SIZE 1024  // 1KB fragment size
#define LOG_FILE "activity.log"
#define RELICS_DIR "relics"
#define MAX_PATH_LEN 1024

void log_activity(const char *action, const char *details) {
    time_t now;
    time(&now);
    struct tm *tm_info = localtime(&now);
    char timestamp[20];
    strftime(timestamp, 20, "%Y-%m-%d %H:%M:%S", tm_info);
    
    FILE *log = fopen(LOG_FILE, "a");
    if (log) {
        fprintf(log, "[%s] %s: %s\n", timestamp, action, details);
        fclose(log);
    }
}

int is_fragment(const char *filename) {
    const char *ext = strrchr(filename, '.');
    if (!ext || strlen(ext) != 4) return 0; // Extension must be .XXX
    
    // Check if extension is .000 to .999
    return isdigit(ext[1]) && isdigit(ext[2]) && isdigit(ext[3]);
}

char* get_base_name(const char *fragment_name_with_path) {
    // First, get only the filename part if it's a path
    const char *filename_part = strrchr(fragment_name_with_path, '/');
    if (filename_part) {
        filename_part++; // Move past '/'
    } else {
        filename_part = fragment_name_with_path;
    }

    char *dot = strrchr(filename_part, '.');
    if (!dot || !is_fragment(filename_part)) {
         return strdup(filename_part);
    }
    
    size_t len = dot - filename_part;
    char *base = malloc(len + 1);
    if (!base) return NULL; // Allocation check
    strncpy(base, filename_part, len);
    base[len] = '\0';
    return base;
}

int compare_fragments(const void *a, const void *b) {
    // We are comparing full fragment names like "file.001", "file.002"
    return strcmp(*(const char **)a, *(const char **)b);
}

int get_fragments(const char *base, char ***fragments_out) {
    DIR *dir;
    struct dirent *ent;
    int count = 0;
    char **temp_fragments = NULL; // Temporary array for storing fragment names
    int capacity = 10; // Initial capacity for temp_fragments
    
    *fragments_out = NULL;
    
    if ((dir = opendir(RELICS_DIR)) == NULL) {
        perror("opendir relics");
        return 0;
    }
    
    temp_fragments = malloc(capacity * sizeof(char *));
    if (!temp_fragments) {
        closedir(dir);
        return 0; // Allocation failure
    }

    while ((ent = readdir(dir)) != NULL) {
        if (ent->d_type == DT_REG && is_fragment(ent->d_name)) { // Check if it's a regular file and a fragment
            char *frag_base = get_base_name(ent->d_name);
            if (frag_base && strcmp(frag_base, base) == 0) {
                if (count >= capacity) {
                    capacity *= 2;
                    char **new_temp_fragments = realloc(temp_fragments, capacity * sizeof(char *));
                    if (!new_temp_fragments) {
                        // Free previously allocated names and array
                        for (int i=0; i<count; ++i) free(temp_fragments[i]);
                        free(temp_fragments);
                        free(frag_base);
                        closedir(dir);
                        return 0; // Reallocation failure
                    }
                    temp_fragments = new_temp_fragments;
                }
                temp_fragments[count] = strdup(ent->d_name);
                if (!temp_fragments[count]) {
                    // Free previously allocated names and array
                    for (int i=0; i<count; ++i) free(temp_fragments[i]);
                    free(temp_fragments);
                    free(frag_base);
                    closedir(dir);
                    return 0; // strdup failure
                }
                count++;
            }
            free(frag_base);
        }
    }
    closedir(dir);
    
    if (count == 0) {
        free(temp_fragments);
        return 0;
    }
    
    // Sort fragments by their full name (which includes numeric extension)
    qsort(temp_fragments, count, sizeof(char *), compare_fragments);
    
    *fragments_out = temp_fragments; // Transfer ownership
    
    return count;
}

// FUSE getattr implementation
static int baymax_getattr(const char *path, struct stat *stbuf) {
    int res = 0;
    memset(stbuf, 0, sizeof(struct stat));
    
    if (strcmp(path, "/") == 0) {
        stbuf->st_mode = S_IFDIR | 0755;
        stbuf->st_nlink = 2; // Directories usually have at least 2 links (. and parent's entry)
        stbuf->st_mtime = time(NULL); // Set modification time
        stbuf->st_ctime = time(NULL);
        stbuf->st_atime = time(NULL);
        return 0;
    }
    
    // Path is something like "/filename", get_base_name expects "filename"
    char *base = get_base_name(path + 1); 
    if (!base) return -ENOMEM;

    char **fragments;
    int count = get_fragments(base, &fragments);
    
    if (count > 0) {
        stbuf->st_mode = S_IFREG | 0644;
        stbuf->st_nlink = 1;
        stbuf->st_size = 0;
        time_t latest_mtime = 0;
        
        // Calculate total size and find latest modification time
        for (int i = 0; i < count; i++) {
            char fullpath[MAX_PATH_LEN];
            snprintf(fullpath, sizeof(fullpath), "%s/%s", RELICS_DIR, fragments[i]);
            
            struct stat frag_stat;
            if (stat(fullpath, &frag_stat) == 0) {
                stbuf->st_size += frag_stat.st_size;
                if (frag_stat.st_mtime > latest_mtime) {
                    latest_mtime = frag_stat.st_mtime;
                }
            }
            free(fragments[i]);
        }
        free(fragments);
        stbuf->st_mtime = latest_mtime; // Use latest fragment's mtime
        stbuf->st_ctime = latest_mtime; // Ideally ctime should be tracked separately
        stbuf->st_atime = latest_mtime; // atime updated on access
    } else {
        res = -ENOENT;
    }
    
    free(base);
    return res;
}

// FUSE readdir implementation
static int baymax_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                         off_t offset, struct fuse_file_info *fi) {
    (void) offset; // Mark as unused
    (void) fi;     // Mark as unused
    
    if (strcmp(path, "/") != 0) {
        return -ENOENT;
    }
    
    filler(buf, ".", NULL, 0);
    filler(buf, "..", NULL, 0);
    
    DIR *dir;
    struct dirent *ent;
    
    // Use a simple dynamic array to track seen base names to avoid N^2 complexity
    char **seen_bases_arr = NULL;
    int seen_count = 0;
    int seen_capacity = 0;
    
    if ((dir = opendir(RELICS_DIR)) != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            if (ent->d_type == DT_REG && is_fragment(ent->d_name)) {
                char *base = get_base_name(ent->d_name);
                if (!base) continue; // Couldn't get base name
                
                int already_seen = 0;
                for (int i = 0; i < seen_count; i++) {
                    if (strcmp(seen_bases_arr[i], base) == 0) {
                        already_seen = 1;
                        break;
                    }
                }
                
                if (!already_seen) {
                    filler(buf, base, NULL, 0);
                    
                    if (seen_count >= seen_capacity) {
                        seen_capacity = (seen_capacity == 0) ? 10 : seen_capacity * 2;
                        char **new_seen_bases_arr = realloc(seen_bases_arr, seen_capacity * sizeof(char *));
                        if (!new_seen_bases_arr) {
                            // Memory allocation failed, clean up what we have and exit
                            for(int i=0; i<seen_count; ++i) free(seen_bases_arr[i]);
                            free(seen_bases_arr);
                            free(base);
                            closedir(dir);
                            return -ENOMEM;
                        }
                        seen_bases_arr = new_seen_bases_arr;
                    }
                    seen_bases_arr[seen_count] = strdup(base);
                    if (!seen_bases_arr[seen_count]) {
                         // strdup failed
                        for(int i=0; i<seen_count; ++i) free(seen_bases_arr[i]);
                        free(seen_bases_arr);
                        free(base);
                        closedir(dir);
                        return -ENOMEM;
                    }
                    seen_count++;
                }
                free(base);
            }
        }
        closedir(dir);
    } else {
        perror("opendir relics in readdir");
        return -errno; // Return actual error from opendir
    }
    
    // Clean up seen_bases_arr
    for (int i = 0; i < seen_count; i++) {
        free(seen_bases_arr[i]);
    }
    free(seen_bases_arr);
    
    return 0;
}

// FUSE open implementation
static int baymax_open(const char *path, struct fuse_file_info *fi) {
    if (strcmp(path, "/") == 0) { // Should not happen if getattr correctly identifies types
        return -EISDIR;
    }
    
    char *base = get_base_name(path + 1); // skip leading '/'
    if (!base) return -ENOMEM;

    char **fragments;
    int count = get_fragments(base, &fragments);
    
    if (count == 0 && (fi->flags & O_CREAT) == 0) { // File doesn't exist and not creating
        free(base);
        return -ENOENT;
    }
    
    // Log the read operation if opening for reading
    // Note: O_RDWR will also match O_RDONLY here.
    if ((fi->flags & O_ACCMODE) == O_RDONLY || (fi->flags & O_ACCMODE) == O_RDWR) {
        char log_msg[MAX_PATH_LEN];
        snprintf(log_msg, sizeof(log_msg), "%s", path + 1); // Use full virtual path
        log_activity("OPEN (read)", log_msg);
    }
    // Logging for write open could be done here or in create/write
    if ((fi->flags & O_ACCMODE) == O_WRONLY || (fi->flags & O_ACCMODE) == O_RDWR) {
         // If O_TRUNC is set, existing fragments will be cleared in write
    }


    // Clean up fragments array if it was populated
    if (count > 0) {
        for (int i = 0; i < count; i++) {
            free(fragments[i]);
        }
        free(fragments);
    }
    free(base);
    
    // For simplicity, we don't check fi->flags for O_EXCL etc.
    // If O_CREAT is used, actual file fragments are made in write.
    return 0;
}

// FUSE read implementation
static int baymax_read(const char *path, char *buf, size_t size, off_t offset,
                      struct fuse_file_info *fi) {
    (void)fi; // Mark as unused
    char *base = get_base_name(path + 1); // skip leading '/'
    if (!base) return -ENOMEM;

    char **fragments;
    int count = get_fragments(base, &fragments);
    free(base); // Free base name as soon as it's not needed
    
    if (count == 0) {
        return -ENOENT;
    }
    
    size_t total_read_bytes = 0;
    off_t current_file_offset = 0; // Tracks position in the conceptual concatenated file
    
    for (int i = 0; i < count; i++) {
        char fullpath[MAX_PATH_LEN];
        snprintf(fullpath, sizeof(fullpath), "%s/%s", RELICS_DIR, fragments[i]);
        
        int fd = open(fullpath, O_RDONLY);
        if (fd == -1) {
            perror("open fragment for read");
            // Decide if we should error out or try to continue
            // For robustness, maybe skip this fragment, but data will be missing
            continue; 
        }
        
        struct stat frag_stat;
        if (fstat(fd, &frag_stat) == -1) {
            perror("fstat fragment");
            close(fd);
            continue;
        }
        size_t frag_size = frag_stat.st_size;

        // If current fragment is entirely before the requested offset, skip it
        if (offset >= current_file_offset + frag_size) {
            current_file_offset += frag_size;
            close(fd);
            continue;
        }

        // Calculate offset within this fragment
        off_t frag_internal_offset = 0;
        if (offset > current_file_offset) {
            frag_internal_offset = offset - current_file_offset;
        }

        // Calculate how much to read from this fragment
        size_t bytes_to_read_from_frag = frag_size - frag_internal_offset;
        if (bytes_to_read_from_frag > size - total_read_bytes) { // Don't read more than requested 'size' or buffer capacity
            bytes_to_read_from_frag = size - total_read_bytes;
        }

        if (bytes_to_read_from_frag > 0) {
            ssize_t R = pread(fd, buf + total_read_bytes, bytes_to_read_from_frag, frag_internal_offset);
            if (R == -1) {
                perror("pread fragment");
                close(fd);
                // Error during read, clean up and return what's read so far or an error
                for (int j = i; j < count; j++) free(fragments[j]); // Free remaining fragment names
                free(fragments);
                return -EIO; 
            }
            total_read_bytes += R;
        }
        
        close(fd);
        current_file_offset += frag_size;

        // If buffer is full or we've read enough
        if (total_read_bytes >= size) {
            break;
        }
    }
    
    // Clean up fragments array
    for (int i = 0; i < count; i++) {
        free(fragments[i]);
    }
    free(fragments);
    
    return total_read_bytes;
}

// FUSE create implementation
static int baymax_create(const char *path, mode_t mode, struct fuse_file_info *fi) {
    // This function is called when a new file is created with open(O_CREAT).
    // We don't need to create physical fragments here;
    // baymax_write will handle fragment creation.
    // We can log the creation event.
    // Note: 'path' is the full virtual path like "/newfile.txt"
    (void)mode; // mode is not used for fragment creation strategy here

    char log_msg[MAX_PATH_LEN];
    snprintf(log_msg, sizeof(log_msg), "%s", path + 1);
    log_activity("CREATE", log_msg);
    
    // The file handle (fi) is passed to write, so we don't need to store anything specific here.
    // If we needed to handle exclusive creation (O_EXCL), we'd check if fragments exist.
    // For simplicity, allow overwriting semantics handled by write.
    return 0; 
}

// FUSE write implementation
static int baymax_write(const char *path, const char *buf, size_t size, off_t offset,
                       struct fuse_file_info *fi) {
    (void)fi; // fi is not used in this simple write implementation
    
    char *base_name_from_path = get_base_name(path + 1); // skip leading '/'
    if (!base_name_from_path) return -ENOMEM;

    // This simple write implementation assumes overwrite or append based on offset.
    // For a full overwrite (offset=0), existing fragments should be deleted first.
    // If offset > 0 and implies append or partial overwrite, it's much more complex.
    // This implementation will simplify: if offset is 0, it's a full overwrite.
    // Otherwise, it's not supported for simplicity.
    // A more robust implementation would handle partial writes and appends by
    // reading existing fragments, modifying them, and potentially re-fragmenting.

    if (offset != 0) {
        // Support for writing at arbitrary offsets is complex with fixed-size fragments.
        // It would require reading existing fragments, merging, and re-splitting.
        // For this example, we only support full overwrites (offset=0) or creating new files.
        log_activity("WRITE_DENIED", "Non-zero offset writes not supported");
        free(base_name_from_path);
        return -EPERM; // Or -EINVAL or -ENOSYS
    }

    // If writing from offset 0 (overwrite or new file), delete existing fragments first.
    char **existing_frags;
    int existing_count = get_fragments(base_name_from_path, &existing_frags);
    if (existing_count > 0) {
        for (int i = 0; i < existing_count; i++) {
            char fullpath[MAX_PATH_LEN];
            snprintf(fullpath, sizeof(fullpath), "%s/%s", RELICS_DIR, existing_frags[i]);
            unlink(fullpath); // Delete the physical fragment file
            free(existing_frags[i]);
        }
        free(existing_frags);
    }

    size_t bytes_written_total = 0;
    int fragment_index = 0;
    char log_details[MAX_PATH_LEN * 2] = {0}; // For logging fragment names
    char temp_frag_name_log[MAX_PATH_LEN / 4];

    // Write data in new fragments
    while (bytes_written_total < size) {
        char fragment_file_name[MAX_PATH_LEN];
        snprintf(fragment_file_name, sizeof(fragment_file_name), "%s.%03d", base_name_from_path, fragment_index);

        char full_frag_path[MAX_PATH_LEN];
        snprintf(full_frag_path, sizeof(full_frag_path), "%s/%s", RELICS_DIR, fragment_file_name);

        int fd = open(full_frag_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd == -1) {
            perror("Failed to create/open fragment for write");
            // If one fragment fails, it's problematic. Clean up and error out.
            // (Cleanup of already written fragments for this operation could be added)
            free(base_name_from_path);
            return -EIO; 
        }

        size_t bytes_to_write_this_frag = FRAGMENT_SIZE;
        if (bytes_written_total + FRAGMENT_SIZE > size) {
            bytes_to_write_this_frag = size - bytes_written_total;
        }
        
        ssize_t written_now = write(fd, buf + bytes_written_total, bytes_to_write_this_frag);
        close(fd);

        if (written_now == -1) {
            perror("Failed to write to fragment");
            // (Cleanup)
            free(base_name_from_path);
            return -EIO;
        }
        if ((size_t)written_now != bytes_to_write_this_frag) {
            // Short write, disk full?
            // (Cleanup)
            log_activity("WRITE_ERROR", "Short write to fragment");
            free(base_name_from_path);
            return -EIO;
        }

        bytes_written_total += written_now;
        
        // Append to log details
        if (fragment_index == 0) {
            snprintf(temp_frag_name_log, sizeof(temp_frag_name_log), "%s", fragment_file_name);
        } else if (fragment_index < 3 || (bytes_written_total == size)) { // Show first few and last
             snprintf(temp_frag_name_log + strlen(temp_frag_name_log), sizeof(temp_frag_name_log) - strlen(temp_frag_name_log),
                     ", %s", fragment_file_name);
        } else if (fragment_index == 3 && (bytes_written_total < size) ) { // Indicate more with "..."
             strncat(temp_frag_name_log, ", ...", sizeof(temp_frag_name_log) - strlen(temp_frag_name_log) -1);
        }
        if (strlen(log_details) == 0) strncpy(log_details, temp_frag_name_log, sizeof(log_details)-1);


        fragment_index++;
    }

    // Log the write operation
    char final_log_msg[MAX_PATH_LEN * 3];
    snprintf(final_log_msg, sizeof(final_log_msg), "%s (%d fragments: %s)", 
             base_name_from_path, fragment_index, log_details);
    log_activity("WRITE", final_log_msg);

    free(base_name_from_path);
    return bytes_written_total; // Return total bytes written
}

// FUSE truncate implementation
static int baymax_truncate(const char *path, off_t size) {
    (void)path; // Unused
    (void)size; // Unused
    // Truncate is complex with fragmented files.
    // A full implementation would involve:
    // - If new size is 0: delete all fragments.
    // - If new size > old size: extend last fragment or add new empty fragments (padding with zeros).
    // - If new size < old size: delete trailing fragments and possibly truncate the new last fragment.
    // For simplicity, this operation is not supported.
    log_activity("TRUNCATE_DENIED", path);
    return -EPERM; // Operation not permitted
}

// FUSE release implementation (called when file is closed)
static int baymax_release(const char *path, struct fuse_file_info *fi) {
    (void)path; // Unused
    (void)fi;   // Unused
    // This is called when the last reference to an open file is closed.
    // Good place for cleanup if open() allocated resources.
    // Nothing to do in this simple filesystem.
    return 0;
}

// FUSE unlink implementation (file deletion)
static int baymax_unlink(const char *path) {
    char *base = get_base_name(path + 1); // skip leading '/'
    if (!base) return -ENOMEM;

    char **fragments;
    int count = get_fragments(base, &fragments);
    
    if (count == 0) {
        free(base);
        return -ENOENT; // File (no fragments) doesn't exist
    }
    
    char log_msg_detail[MAX_PATH_LEN] = {0};
    if (count == 1) {
        snprintf(log_msg_detail, sizeof(log_msg_detail), "Deleted fragment: %s", fragments[0]);
    } else if (count > 1) {
        snprintf(log_msg_detail, sizeof(log_msg_detail), "Deleted fragments %s ... %s (%d total)",
                 fragments[0], fragments[count-1], count);
    }
    log_activity("DELETE", log_msg_detail);
    
    // Delete all fragments associated with the base name
    for (int i = 0; i < count; i++) {
        char fullpath[MAX_PATH_LEN];
        snprintf(fullpath, sizeof(fullpath), "%s/%s", RELICS_DIR, fragments[i]);
        if (unlink(fullpath) == -1) {
            perror("unlink fragment");
            // If unlinking one fragment fails, it's an issue.
            // Log it and potentially return an error. For now, try to continue.
        }
        free(fragments[i]);
    }
    free(fragments);
    free(base);
    
    return 0;
}

// FUSE rename implementation
// Corrected signature for FUSE 2.x API
static int baymax_rename(const char *from, const char *to) {
    (void)from; // Mark as unused
    (void)to;   // Mark as unused
    // Renaming virtual files would mean renaming all their underlying fragment files.
    // e.g., "oldname.000, oldname.001" -> "newname.000, newname.001"
    // Also need to handle if 'to' already exists (overwrite semantics).
    // This is complex. For simplicity, not implemented.
    // The 'flags' argument (RENAME_NOREPLACE, RENAME_EXCHANGE) is part of FUSE 3 API.
    // If using FUSE 2, this function signature is int (*rename)(const char*, const char*);
    char log_msg[MAX_PATH_LEN*2 + 20];
    snprintf(log_msg, sizeof(log_msg), "Rename attempt from %s to %s denied", from, to);
    log_activity("RENAME_DENIED", log_msg);
    return -EPERM; // Operation not permitted
}


// FUSE operations structure
static struct fuse_operations baymax_oper = {
    .getattr    = baymax_getattr,
    .readdir    = baymax_readdir,
    .open       = baymax_open,
    .read       = baymax_read,
    .create     = baymax_create,
    .write      = baymax_write,
    .truncate   = baymax_truncate,
    .release    = baymax_release,
    .unlink     = baymax_unlink,
    .rename     = baymax_rename, // This line (448 in original) caused the error
};

int main(int argc, char *argv[]) {
    umask(0); // Set umask to allow created files/dirs to have permissions set by mode
    
    // Ensure relics directory exists
    struct stat st_relics = {0};
    if (stat(RELICS_DIR, &st_relics) == -1) {
        if (mkdir(RELICS_DIR, 0755) == -1) {
            perror("Failed to create relics directory");
            return 1;
        }
    }
    
    // Initialize log file (check if writable)
    FILE *log_check = fopen(LOG_FILE, "a");
    if (!log_check) {
        perror("Failed to open/create log file");
        // Decide if this is fatal. For now, continue, logging will just fail silently.
    } else {
        fclose(log_check);
    }
    
    log_activity("FILESYSTEM_START", "Baymax FUSE started");
    int ret = fuse_main(argc, argv, &baymax_oper, NULL);
    log_activity("FILESYSTEM_STOP", "Baymax FUSE stopped");
    
    return ret;
}
```

#### > Penjelasan
#### A. Membuat Sistem File Virtual dengan FUSE
Kode Terkait:
```
#define FUSE_USE_VERSION 31
#define _XOPEN_SOURCE 700
#include <fuse.h>
// ... header lainnya ...

static struct fuse_operations baymax_oper = {
    .getattr = baymax_getattr,
    .readdir = baymax_readdir,
    .open = baymax_open,
    .read = baymax_read,
    .create = baymax_create,
    .write = baymax_write,
    .truncate = baymax_truncate,
    .release = baymax_release,
    .unlink = baymax_unlink,
    .rename = baymax_rename,
};

int main(int argc, char *argv[]) {
    umask(0);
    mkdir(RELICS_DIR, 0755); // Buat direktori relics jika belum ada
    return fuse_main(argc, argv, &baymax_oper, NULL); // Mount FUSE
}
```
Penjelasan:

- Struktur `fuse_operations`: Mendefinisikan callback untuk operasi filesystem (baca, tulis, hapus, dll.).

- `fuse_main`: Fungsi utama untuk mount filesystem ke mount_dir.

- `relics/`: Direktori fisik penyimpan fragmen (dibuat otomatis jika belum ada).


#### B. Menyajikan File Utuh dari Fragmen
Kode Terkait:
```
// Gabungkan fragmen saat membaca file
static int baymax_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
    char *base = get_base_name(path + 1);
    char **fragments;
    int count = get_fragments(base, &fragments);
    
    // Baca setiap fragmen dan gabungkan
    for (int i = 0; i < count; i++) {
        char fullpath[MAX_PATH_LEN];
        snprintf(fullpath, sizeof(fullpath), "%s/%s", RELICS_DIR, fragments[i]);
        FILE *f = fopen(fullpath, "rb");
        fread(buf + total_read, 1, frag_size, f); // Gabungkan ke buffer
        fclose(f);
    }
    // ... cleanup ...
    return total_read; // Kembalikan data utuh
}
```
Penjelasan:

- `get_fragments`: Mengumpulkan semua fragmen (.000 hingga .013) untuk file tertentu.

- `baymax_read`: Membaca fragmen secara berurutan dan menggabungkannya di memori sebelum dikembalikan ke pengguna.

- Ilusi file utuh: Pengguna melihat `Baymax.jpeg` di `mount_dir`, meski fisiknya terpecah di `relics/`.

#### C. Memecah File Baru ke Fragmen 1KB
Kode :
```
static int baymax_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
    // Hitung jumlah fragmen needed
    int fragment_count = (size + FRAGMENT_SIZE - 1) / FRAGMENT_SIZE;

    // Tulis per fragmen
    for (int i = 0; i < fragment_count; i++) {
        char fragment_name[MAX_PATH_LEN];
        snprintf(fragment_name, sizeof(fragment_name), "%s.%03d", base_name, i);
        FILE *f = fopen(fragment_name, "wb");
        fwrite(buf + (i * FRAGMENT_SIZE), 1, frag_size, f); // Tulis 1KB per fragmen
        fclose(f);
    }
    // ... logging ...
}
```
Penjelasan:

- Pemecahan otomatis: File baru langsung dipecah ke `relics/` dengan format `[nama].000`, `[nama].001`, dst.

- Ukuran tetap: Setiap fragmen maksimal 1KB (FRAGMENT_SIZE).

#### D. Menghapus Semua Fragmen saat File Dihapus
Kode Terkait:
```
static int baymax_unlink(const char *path) {
    char *base = get_base_name(path + 1);
    char **fragments;
    int count = get_fragments(base, &fragments);
    
    // Hapus semua fragmen
    for (int i = 0; i < count; i++) {
        char fullpath[MAX_PATH_LEN];
        snprintf(fullpath, sizeof(fullpath), "%s/%s", RELICS_DIR, fragments[i]);
        unlink(fullpath); // Hapus file fisik
    }
    // ... logging ...
}
```
Penjelasan:

Sinkronisasi penghapusan: Menghapus file di `mount_dir` akan menghapus semua fragmen terkait di `relics/`.

- `Logging`: Mencatat nama fragmen yang terhapus.

#### E. Mencatat Aktivitas di activity.log
Kode:
```
void log_activity(const char *action, const char *details) {
    time_t now = time(NULL);
    char timestamp[20];
    strftime(timestamp, 20, "%Y-%m-%d %H:%M:%S", localtime(&now));
    
    FILE *log = fopen(LOG_FILE, "a");
    fprintf(log, "[%s] %s: %s\n", timestamp, action, details);
    fclose(log);
}

// Contoh logging di operasi baca
static int baymax_open(const char *path, struct fuse_file_info *fi) {
    if (fi->flags & O_RDONLY) {
        log_activity("READ", path + 1); // Catat aktivitas baca
    }
    // ...
}
```
Penjelasan:

- Format log: [timestamp] ACTION: detail (misal: [2025-05-11 10:24:01] READ: Baymax.jpeg).

- Aktivitas tercatat:

- `READ`: Saat file dibuka.

- `WRITE`: Saat file baru dibuat (beserta daftar fragmen).

- `DELETE`: Saat file dihapus (beserta range fragmen).

- `COPY`: Tertangkap secara tidak langsung melalui operasi baca-tulis.


### Dokumentasi
![image](https://github.com/user-attachments/assets/135f0991-cc33-4194-b06e-6a1ef0f3a214)
![image](https://github.com/user-attachments/assets/f3a50a0e-8d23-48c0-b4d7-22b766196557)


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
