# Vocal Lab 🎙️

**Vocal Lab** adalah plugin *vocal chain* modern yang dirancang untuk memberikan hasil vokal standar industri musik secara instan. Menggabungkan kemudahan UI berbasis *macro* untuk workflow yang cepat, dengan fleksibilitas DSP *Advanced Mode* bagi para *audio engineer* yang membutuhkan kontrol presisi.

**Dikembangkan oleh:** CiptaSuara & Soba Studio  
**Format:** VST3, AU (AudioUnit), Standalone  
**OS Support:** Windows (64-bit) & macOS  

---

## ✨ Fitur Utama

### 🎛️ Macro Mode (Easy Workflow)
Hanya dengan 4 knob utama, Anda bisa mendapatkan karakter vokal yang diinginkan tanpa pusing memikirkan parameter kompleks:
- **INTENSITY:** Mengontrol dinamika vokal (Noise Gate + Compressor threshold & ratio secara otomatis).
- **COLOR:** Memberikan karakter analog, saturasi tabung (drive), harmonic excitement, dan pelebaran stereo tipis.
- **SPACE:** Mengatur dimensi ruangan vokal melalui perpaduan *Stereo Delay* dan *Reverb* yang di-tuning khusus untuk vokal.
- **TUNE:** (*Fitur placeholder/makro untuk kontrol tonal*)

### 🎚️ Advanced Mode (Pro Workflow)
Klik tombol **ADVANCED** untuk membuka panel bawah berisi 9 parameter DSP presisi:
- **GATE THR:** Noise gate threshold (-80 dB s/d 0 dB)
- **COMP THR:** Compressor threshold (-60 dB s/d 0 dB)
- **RATIO:** Compressor ratio (1:1 s/d 20:1)
- **DRIVE:** Saturasi tipe *tanh soft-clipping* (0 s/d 18 dB)
- **AIR GAIN:** High-shelf EQ boost @ 12kHz untuk vokal yang "mahal" (0 s/d 12 dB)
- **CHORUS:** Chorus mix untuk penebalan vokal (0 s/d 100%)
- **DELAY:** Delay mix dengan timing 250ms (0 s/d 100%)
- **REV WET:** Reverb wet level (0 s/d 100%)
- **REV SIZE:** Reverb room size (0 s/d 1)

### 🎭 Artist Presets
Sudah dilengkapi dengan *built-in preset* yang mereplikasi gaya mixing vokal artis-artis papan atas:
- Default
- The Weekday
- Billie Eyelash
- Travis S.
- Adella
- Drizzy
- Ariana Venti
- Heruwa (Shaggydog)

---

## 📥 Cara Instalasi

### Windows
1. Jalankan `VocalLab_Installer_v1.0.0.exe`.
2. Centang format yang Anda butuhkan (**VST3 Plugin** dan/atau **Standalone**).
3. Untuk VST3, buka DAW Anda (FL Studio, Studio One, Ableton, Reaper) dan lakukan *Plugin Scan*.
4. Masukkan "Vocal Lab" ke dalam *insert* / *fx channel* vokal Anda.

### macOS
1. Ekstrak file `VocalLab_Mac_Release.zip`.
2. Pindahkan file `.vst3` ke direktori: `/Library/Audio/Plug-Ins/VST3/`
3. Pindahkan file `.component` (AU) ke direktori: `/Library/Audio/Plug-Ins/Components/`
4. Pindahkan aplikasi `.app` (Standalone) ke folder `/Applications/`.
5. Buka DAW (Logic Pro, Ableton, dll), lakukan rescan jika diperlukan, dan gunakan pluginnya!

---

## 🛠️ Kompilasi dari Source Code (Bagi Developer)

Plugin ini dibangun menggunakan framework **JUCE 8** dan **CMake**.

### Prasyarat:
- CMake (>= 3.22)
- C++20 Compiler (MSVC / Xcode)
- Git

### Build Instructions:
```bash
# Clone repo
git clone https://github.com/your-username/VocalLab.git
cd VocalLab

# Konfigurasi CMake
cmake -B build -DCMAKE_BUILD_TYPE=Release

# Build Project
cmake --build build --config Release
```

*(Note: JUCE akan otomatis di-download oleh CMake via FetchContent saat konfigurasi pertama kali).*

---

### ©️ Copyright & Lisensi
Dikembangkan secara penuh oleh **CiptaSuara & Soba Studio**.  
All rights reserved.
