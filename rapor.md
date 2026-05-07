# TARSAR: Arşivleme Programı Geliştirme Raporu

**Proje:** Bilgisayar Mühendisliği Sistem Programlama Projesi  
**Grup Üyeleri:**  
- G221210004
- G221210026

**GitHub Proje Linki:**  
https://github.com/SametAlici/sistem_programlama_proje

## 1. Proje Özeti
Bu projede Linux (Unix) işletim sistemlerinde çalışan, `tar` veya `zip` programlarına benzer şekilde birden çok metin dosyasını sıkıştırma yapmaksızın tek bir dosyada birleştirebilen ve birleştirilen bu dosyayı istenen bir dizine tekrar çıkartabilen `tarsau` isimli C uygulaması geliştirilmiştir.

## 2. Tasarım ve Mimari Kararlar
- **Modüler Yapı:** Projenin temel iskeleti `-b` (arşivleme) ve `-a` (çıkarma) işlemlerini iki ayrı modül fonksiyonuna bölecek şekilde tasarlandı (`archive_files` ve `extract_files`).
- **Veri Yapısı:** `.sau` formatının organizasyon kısmı için, her dosyanın ismini, erişim izinlerini (`st_mode & 0777`) ve bayt cinsinden boyutunu içeren virgülle ayrılmış veri yapısı kullanılmıştır. Boyut ve ofset hesaplamaları POSIX `stat` sistemi ile yapılmıştır.
- **Metin Dosyası Kontrolü:** Dosyanın yalnızca ASCII metin içerdiğini doğrulamak için bir `is_text_file` fonksiyonu geliştirilmiş, ASCII tablosundaki 0-127 sınırları ve uygun boşluk (whitespace) karakterleri filtrelenmiştir.
- **Hata Yönetimi:** Hatalı format, geçersiz boyut (200MB üzeri) veya var olmayan dosya girişlerinde uygulamanın çökmeyeceği ve istenen "uygunsuz veya bozuk" hata çıktılarını fırlatarak temiz çıkış (exit(0)) yapacağı şekilde kurgulanmıştır.

## 3. Ekran Çıktıları ve Test Sonuçları
Aşağıda örnek senaryolar test edilmiş ve başarılı sonuçlar elde edilmiştir:

**Arşivleme:**
```bash
> ./tarsau -b t1.txt t2.txt -o a.sau
Dosyalar birlestirildi.
```

**Arşiv Çıkarma:**
```bash
> ./tarsau -a a.sau d1
d1 dizininde t1.txt ve t2.txt dosyalari acildi.
```

**Uyumsuz Format Testi:**
```bash
> ./tarsau -b binary_dosya.bin
binary_dosya.bin giris dosyasinin formati uyumsuzdur!
```

## 4. Kaynak Kod ve Derleme
Proje `Makefile` üzerinden `make` komutu kullanılarak derlenebilmektedir. Kaynak kodlar Linux C standartlarına uygundur.
