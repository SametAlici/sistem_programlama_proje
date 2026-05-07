#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#ifdef _WIN32
    #include <direct.h>
    #include <io.h>
    #define MKDIR(path) _mkdir(path)
    #ifndef S_IRUSR
        #define S_IRUSR 0400
        #define S_IWUSR 0200
        #define S_IXUSR 0100
    #endif
#else
    #include <unistd.h>
    #define MKDIR(path) mkdir(path, 0777)
#endif

#define MAX_FILES 32
#define MAX_TOTAL_SIZE (200 * 1024 * 1024)

// Fonksiyon prototipleri
int is_text_file(const char *filename);
void archive_files(int file_count, char *input_files[], const char *output_archive);
void extract_files(const char *archive_file, const char *output_dir);

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Kullanim: tarsau -b dosya1 dosya2 ... -o arsiv.sau\n");
        printf("          tarsau -a arsiv.sau [hedef_dizin]\n");
        return 1;
    }
    
    if (strcmp(argv[1], "-b") == 0) {
        char *input_files[MAX_FILES];
        int file_count = 0;
        const char *output_archive = "a.sau"; // Varsayilan isim
        
        for (int i = 2; i < argc; i++) {
            if (strcmp(argv[i], "-o") == 0) {
                if (i + 1 < argc) {
                    output_archive = argv[i+1];
                    i++; // argumani atla
                }
            } else {
                if (file_count < MAX_FILES) {
                    input_files[file_count++] = argv[i];
                } else {
                    printf("Giris dosyasi sayisi en fazla 32 olabilir.\n");
                    return 0;
                }
            }
        }
        
        if (file_count == 0) {
            printf("Birlestirilecek dosya belirtilmedi.\n");
            return 0;
        }
        
        archive_files(file_count, input_files, output_archive);
        
    } else if (strcmp(argv[1], "-a") == 0) {
        if (argc < 3 || argc > 4) {
            printf("Kullanim: tarsau -a arsiv.sau [hedef_dizin]\n");
            return 1;
        }
        
        const char *archive_file = argv[2];
        const char *output_dir = (argc == 4) ? argv[3] : "";
        
        // .sau uzanti kontrolu
        const char *ext = strrchr(archive_file, '.');
        if (!ext || strcmp(ext, ".sau") != 0) {
            printf("Arsiv dosyasi uygunsuz veya bozuk!\n");
            return 0;
        }
        
        extract_files(archive_file, output_dir);
        
    } else {
        printf("Gecersiz parametre.\n");
        return 1;
    }
    
    return 0;
}

// Dosyanin metin dosyasi (ASCII) olup olmadigini kontrol eder
int is_text_file(const char *filename) {
    FILE *f = fopen(filename, "rb");
    if (!f) return 0; // Acilamiyorsa gecersiz kabul et
    
    int c;
    while ((c = fgetc(f)) != EOF) {
        // Standart ASCII tablosu 0-127 arasidir.
        if (c > 127 || c < 0) {
            fclose(f);
            return 0;
        }
        // Gorunmeyen bazi kontrol karakterleri disindaki kontrol karakterlerini (or: null byte) reddet.
        // 9: Tab, 10: LF (Yeni satir), 13: CR
        if (c < 32 && c != 9 && c != 10 && c != 13) {
            fclose(f);
            return 0; 
        }
    }
    fclose(f);
    return 1;
}

// Dosyalari arsivleme islemi
void archive_files(int file_count, char *input_files[], const char *output_archive) {
    long total_size = 0;
    struct stat file_stats[MAX_FILES];
    
    for (int i = 0; i < file_count; i++) {
        if (stat(input_files[i], &file_stats[i]) != 0) {
            printf("%s dosyasi bulunamadi veya erisilemiyor!\n", input_files[i]);
            exit(0);
        }
        
        if (!is_text_file(input_files[i])) {
            printf("%s giris dosyasinin formati uyumsuzdur!\n", input_files[i]);
            exit(0);
        }
        
        total_size += file_stats[i].st_size;
    }
    
    if (total_size > MAX_TOTAL_SIZE) {
        printf("Giris dosyalarinin toplam boyutu 200 MB'i gecemez.\n");
        exit(0);
    }
    
    // Header stringini olustur
    char records[16384] = "";
    for (int i = 0; i < file_count; i++) {
        char buffer[512];
        const char *basename = input_files[i];
        
        // Sadece dosya ismini al (dizin yollarini temizle)
        const char *last_slash = strrchr(basename, '/');
        if (!last_slash) last_slash = strrchr(basename, '\\');
        if (last_slash) basename = last_slash + 1;
        
        int perms = file_stats[i].st_mode & 0777; // Sadece erisim izinlerini al
        sprintf(buffer, "|%s,%04o,%ld", basename, perms, (long)file_stats[i].st_size);
        strcat(records, buffer);
    }
    strcat(records, "|");
    
    int header_size = 10 + strlen(records);
    
    FILE *out = fopen(output_archive, "wb");
    if (!out) {
        printf("Arsiv dosyasi olusturulamadi!\n");
        exit(0);
    }
    
    // İlk 10 byte boyut bilgisi (ornek: 0000000150) ve ardiindan kayitlar
    fprintf(out, "%010d%s", header_size, records);
    
    // Dosya iceriklerini sirayla yaz
    for (int i = 0; i < file_count; i++) {
        FILE *in = fopen(input_files[i], "rb");
        if (in) {
            char buffer[4096];
            size_t bytes;
            while ((bytes = fread(buffer, 1, sizeof(buffer), in)) > 0) {
                fwrite(buffer, 1, bytes, out);
            }
            fclose(in);
        }
    }
    
    fclose(out);
    printf("Dosyalar birlestirildi.\n");
}

// Dosyalari acma islemi
void extract_files(const char *archive_file, const char *output_dir) {
    FILE *in = fopen(archive_file, "rb");
    if (!in) {
        printf("Arsiv dosyasi uygunsuz veya bozuk!\n");
        exit(0);
    }
    
    char size_buf[11];
    if (fread(size_buf, 1, 10, in) != 10) {
        printf("Arsiv dosyasi uygunsuz veya bozuk!\n");
        fclose(in);
        exit(0);
    }
    size_buf[10] = '\0';
    
    int header_size = atoi(size_buf);
    if (header_size <= 10) {
        printf("Arsiv dosyasi uygunsuz veya bozuk!\n");
        fclose(in);
        exit(0);
    }
    
    int records_len = header_size - 10;
    char *records = malloc(records_len + 1);
    if (fread(records, 1, records_len, in) != (size_t)records_len) {
        printf("Arsiv dosyasi uygunsuz veya bozuk!\n");
        free(records);
        fclose(in);
        exit(0);
    }
    records[records_len] = '\0';
    
    char path_prefix[1024] = "";
    if (output_dir && strlen(output_dir) > 0) {
        MKDIR(output_dir);
        strcpy(path_prefix, output_dir);
        strcat(path_prefix, "/"); // Platformdan bagimsiz dosya ayirici
    }
    
    typedef struct {
        char name[256];
        int perms;
        long size;
    } FileRec;
    FileRec files[MAX_FILES];
    int count = 0;
    
    char *token = strtok(records, "|");
    while (token != NULL) {
        if (sscanf(token, "%255[^,],%o,%ld", files[count].name, &files[count].perms, &files[count].size) == 3) {
            count++;
        }
        token = strtok(NULL, "|");
    }
    
    for (int i = 0; i < count; i++) {
        char full_path[1024];
        snprintf(full_path, sizeof(full_path), "%s%s", path_prefix, files[i].name);
        
        FILE *out = fopen(full_path, "wb");
        if (out) {
            long bytes_left = files[i].size;
            char buffer[4096];
            while (bytes_left > 0) {
                size_t to_read = (size_t)bytes_left < sizeof(buffer) ? (size_t)bytes_left : sizeof(buffer);
                size_t read_bytes = fread(buffer, 1, to_read, in);
                if (read_bytes == 0) break;
                fwrite(buffer, 1, read_bytes, out);
                bytes_left -= read_bytes;
            }
            fclose(out);
            
            // Izinleri uygula
            #ifndef _WIN32
            chmod(full_path, files[i].perms);
            #else
            _chmod(full_path, files[i].perms);
            #endif
        }
    }
    
    free(records);
    fclose(in);
    
    // Cikti metnini olustur
    char names_str[16384] = "";
    for (int i = 0; i < count; i++) {
        if (i > 0) {
            if (i == count - 1) {
                strcat(names_str, " ve ");
            } else {
                strcat(names_str, ", ");
            }
        }
        strcat(names_str, files[i].name);
    }
    
    const char *dir_name = (output_dir && strlen(output_dir) > 0) ? output_dir : "gecerli";
    printf("%s dizininde %s dosyalari acildi.\n", dir_name, names_str);
}
