#include <iostream>
#include <iomanip>
#include <ctime>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <cctype>  // Untuk isdigit() dan isalpha()

using namespace std;

// ANSI COLOR CODES
#define RESET       "\033[0m"
#define BOLD        "\033[1m"
#define BRED        "\033[91m"
#define BGREEN      "\033[92m"
#define BYELLOW     "\033[93m"
#define BBLUE       "\033[94m"
#define BMAGENTA    "\033[95m"
#define BCYAN       "\033[96m"
#define BWHITE      "\033[97m"
#define BG_MAGENTA  "\033[45m"
#define BG_CYAN     "\033[46m"
#define BG_BLUE     "\033[44m"
#define BG_BLACK    "\033[40m"

// WARNA HARGA
#define BHARGA      "\033[93m"   // BYELLOW — dipakai di SEMUA tampilan harga

// LEBAR KOLOM PRODUK
#define TBL_SEP "+-----+-------------------------+-------------+-----------------+-------+---------------+"
#define TBL_HDR "| %-3s | %-23s | %-11s | %-15s | %-5s | %-13s |\n"
#define TBL_ROW "| %s%-3d%s | %s%-23s%s | %s%-11s%s | %s%-15s%s | %s%-5d%s | %s%-13s%s |\n"

// TAMPILAN

void cetakGaris(string warna, int panjang = 56) {
    cout << warna;
    for (int i = 0; i < panjang; i++) cout << "=";
    cout << RESET << "\n";
}

void cetakGarisTipis(string warna, int panjang = 56) {
    cout << warna;
    for (int i = 0; i < panjang; i++) cout << "-";
    cout << RESET << "\n";
}

void cetakBaris(string warna, string teks, int panjang = 56) {
    int isi = panjang - 2;
    cout << warna << "| " << RESET << left << setw(isi - 2) << teks << warna << " |" << RESET << "\n";
}

void cetakTengah(string bgWarna, string teksWarna, string teks, int panjang = 56) {
    int isi = panjang - 2;
    int pad = (isi - (int)teks.size()) / 2;
    if (pad < 0) pad = 0;
    int padKanan = isi - (int)teks.size() - pad;
    if (padKanan < 0) padKanan = 0;
    cout << bgWarna << "|";
    for (int i = 0; i < pad; i++) cout << " ";
    cout << teksWarna << BOLD << teks << RESET << bgWarna;
    for (int i = 0; i < padKanan; i++) cout << " ";
    cout << "|" << RESET << "\n";
}

void cetakHeader(string judul, string warna, int panjang = 56) {
    cetakGaris(warna, panjang);
    cetakTengah(warna, BWHITE, judul, panjang);
    cetakGaris(warna, panjang);
}

// WAKTU & TRANSAKSI

string getWaktuSekarang() {
    time_t now = time(0);
    struct tm *ltm = localtime(&now);
    char buf[80];
    sprintf(buf, "%02d/%02d/%04d %02d:%02d:%02d",
        ltm->tm_mday, ltm->tm_mon + 1, ltm->tm_year + 1900,
        ltm->tm_hour, ltm->tm_min, ltm->tm_sec);
    return string(buf);
}

string generateNoTransaksi() {
    time_t now = time(0);
    struct tm *ltm = localtime(&now);
    char buf[40];
    sprintf(buf, "TRX%04d%02d%02d%02d%02d%02d",
        ltm->tm_year + 1900, ltm->tm_mon + 1, ltm->tm_mday,
        ltm->tm_hour, ltm->tm_min, ltm->tm_sec);
    return string(buf);
}

const string FILE_PENGGUNA      = "data_pengguna.txt";
const string FILE_RIWAYAT_LOGIN = "riwayat_login.txt";

struct Harga { double satuan; string matauang; };
struct Stok  { int jumlah;   string satuan;   };
struct Produk {
    string nama, merk;
    Harga  harga;
    Stok   stok;
    string jenis;
    int    aktif;
};
struct Pengguna {
    string nama, password, role;
    int    aktif;
};
struct ItemKeranjang {
    int idxProduk;
    int jumlah;
};

int MAKS_PENGGUNA  = 10;
int MAKS_PRODUK    = 100;
int MAKS_KERANJANG = 50;

Pengguna dataPengguna[10];
Produk   daftarProduk[100];
int jumlahPengguna = 0;
int totalProduk    = 0;

// SIMPAN & LOAD PENGGUNA

void simpanPengguna() {
    try {
        ofstream f(FILE_PENGGUNA.c_str());
        if (!f.is_open()) throw runtime_error("Gagal membuka file pengguna.");
        f << jumlahPengguna << "\n";
        for (int i = 0; i < jumlahPengguna; i++)
            f << dataPengguna[i].nama     << "\n"
              << dataPengguna[i].password << "\n"
              << dataPengguna[i].role     << "\n"
              << dataPengguna[i].aktif    << "\n";
        f.close();
    } catch (const runtime_error& e) {
        cout << BRED << "  [!] Error: " << e.what() << RESET << "\n";
    }
}

void loadPengguna() {
    try {
        ifstream f(FILE_PENGGUNA.c_str());
        if (!f.is_open()) return;
        int jml = 0;
        f >> jml; f.ignore();
        for (int i = 0; i < jml && jumlahPengguna < MAKS_PENGGUNA; i++) {
            string nama, password, role; int aktif = 1;
            getline(f, nama); getline(f, password); getline(f, role);
            f >> aktif; f.ignore();
            if (nama.empty() || password.empty() || role.empty()) continue;
            int dup = 0;
            for (int j = 0; j < jumlahPengguna; j++)
                if (dataPengguna[j].nama == nama) { dup = 1; break; }
            if (dup) continue;
            dataPengguna[jumlahPengguna++] = {nama, password, role, aktif};
        }
        f.close();
    } catch (const exception& e) {
        cout << BRED << "  [!] Gagal load pengguna: " << e.what() << RESET << "\n";
    }
}

// RIWAYAT LOGIN

void simpanRiwayatLogin(string namaUser, string role, string status) {
    try {
        ofstream f(FILE_RIWAYAT_LOGIN.c_str(), ios::app);
        if (!f.is_open()) throw runtime_error("Gagal membuka file riwayat login.");
        f << getWaktuSekarang() << "|" << namaUser << "|" << role << "|" << status << "\n";
        f.close();
    } catch (const runtime_error& e) {
        cout << BRED << "  [!] Error: " << e.what() << RESET << "\n";
    }
}

// TABEL RIWAYAT LOGIN
#define LOG_SEP "+---------------------+------------------+-------------+----------+"
#define LOG_HDR "| %-20s | %-17s | %-12s | %-9s|\n"
#define LOG_ROW "| %-20s | %-17s | %s%-12s%s | %s%-9s%s|\n"

void tampilRiwayatLogin() {
    try {
        ifstream f(FILE_RIWAYAT_LOGIN.c_str());
        if (!f.is_open()) throw runtime_error("File riwayat login tidak ditemukan.");

        cout << BCYAN << LOG_SEP << "\n" << RESET;
        printf(LOG_HDR,
            (BYELLOW + string("Waktu") + RESET).c_str(),
            (BGREEN  + string("Nama") + RESET).c_str(),
            (BMAGENTA+ string("Role") + RESET).c_str(),
            (BWHITE  + string("Status") + RESET).c_str());
        cout << BCYAN << LOG_SEP << "\n" << RESET;

        string baris; int ada = 0;
        while (getline(f, baris)) {
            string kolom[4]; int k = 0; string tmp = "";
            for (int i = 0; i < (int)baris.size(); i++) {
                if (baris[i] == '|' && k < 3) { kolom[k++] = tmp; tmp = ""; }
                else tmp += baris[i];
            }
            kolom[k] = tmp;
            if ((int)kolom[1].size() > 17) kolom[1] = kolom[1].substr(0, 14) + "...";
            if ((int)kolom[2].size() > 12) kolom[2] = kolom[2].substr(0, 9)  + "...";
            string sc = (kolom[3].find("BERHASIL") != string::npos) ? BGREEN : BRED;
            cout << BCYAN << "| " << RESET
                 << left << setw(20) << kolom[0]
                 << BCYAN << " | " << RESET
                 << setw(17) << kolom[1]
                 << BCYAN << " | " << BMAGENTA
                 << setw(12) << kolom[2]
                 << BCYAN << " | " << sc
                 << setw(9) << kolom[3]
                 << BCYAN << "|\n" << RESET;
            ada = 1;
        }
        if (!ada) {
            cout << BCYAN << "| " << BYELLOW
                 << left << setw(58) << "(belum ada riwayat login)"
                 << BCYAN << "|\n" << RESET;
        }
        cout << BCYAN << LOG_SEP << "\n" << RESET;
        f.close();
    } catch (const runtime_error& e) {
        cout << BRED << "  [!] " << e.what() << RESET << "\n";
    }
}

void menuAdmin(string namaUser);
void menuPelanggan(string namaUser);
int  doLogin(Pengguna* data, int jml);

// ===================== UTILITAS =====================

string angkaKeStr(long long n) {
    if (n == 0) return "0";
    string s = "";
    while (n > 0) { s = char('0' + n % 10) + s; n /= 10; }
    return s;
}

string rupiahFormat(double angka) {
    string s = angkaKeStr((long long)angka);
    int n = s.size(); string hasil = "";
    for (int i = 0; i < n; i++) {
        if (i > 0 && (n - i) % 3 == 0) hasil += ".";
        hasil += s[i];
    }
    return "Rp " + hasil;
}

int validAngka(string s) {
    if (s.empty()) return 0;
    for (int i = 0; i < (int)s.size(); i++)
        if (s[i] < '0' || s[i] > '9') return 0;
    return 1;
}

// ===================== VALIDASI BARU =====================
// Cek apakah string mengandung angka
bool containsDigit(const string& s) {
    for (char c : s) {
        if (isdigit(c)) return true;
    }
    return false;
}

// Cek apakah string mengandung huruf
bool containsLetter(const string& s) {
    for (char c : s) {
        if (isalpha(c)) return true;
    }
    return false;
}

int bacaAngka(string s) {
    if (s.empty()) throw invalid_argument("Input tidak boleh kosong.");
    if (!validAngka(s)) throw invalid_argument("Input harus berupa angka positif (tanpa huruf).");
    int hasil = 0;
    for (int i = 0; i < (int)s.size(); i++) hasil = hasil * 10 + (s[i] - '0');
    return hasil;
}

double bacaHarga(string s) {
    if (s.empty()) throw invalid_argument("Harga tidak boleh kosong.");
    if (!validAngka(s)) throw invalid_argument("Harga harus berupa angka positif (tanpa huruf).");
    double h = 0;
    for (int i = 0; i < (int)s.size(); i++) h = h * 10 + (s[i] - '0');
    if (h <= 0) throw out_of_range("Harga harus lebih dari 0.");
    return h;
}

// ===================== HELPER: right-align harga dalam kolom =====================
// Mencetak harga right-aligned dalam lebar `w`, dengan warna BHARGA
void cetakHargaKolom(string harga, int w) {
    // right-align: padding kiri
    int pad = w - (int)harga.size();
    if (pad < 0) pad = 0;
    cout << BHARGA;
    for (int i = 0; i < pad; i++) cout << " ";
    cout << harga << RESET;
}

// ===================== TABEL PRODUK =====================
// Kolom: No(3) | Nama(23) | Merk(11) | Harga(15) | Stok(5) | Jenis(13)
// Harga RIGHT-aligned, warna BHARGA (kuning)
// Stok sedikit (<=10) tetap BRED

void cetakHeaderTabel() {
    cout << BCYAN << "+-----+-------------------------+-------------+-----------------+-------+---------------+\n";
    cout << "| " << BYELLOW   << left  << setw(3)  << "No"
         << BCYAN << " | " << BGREEN   << left  << setw(23) << "Nama Produk"
         << BCYAN << " | " << BMAGENTA << left  << setw(11) << "Merk"
         << BCYAN << " | " << BHARGA   << right << setw(15) << "Harga"   // right-align header harga
         << BCYAN << " | " << BWHITE   << left  << setw(5)  << "Stok"
         << BCYAN << " | " << BBLUE    << left  << setw(13) << "Jenis"
         << BCYAN << " |\n";
    cout << "+-----+-------------------------+-------------+-----------------+-------+---------------+\n" << RESET;
}

void cetakBarisProdukTabel(int no, Produk &p) {
    string nama = p.nama;
    if ((int)nama.size() > 23) nama = nama.substr(0, 20) + "...";
    string merk = p.merk;
    if ((int)merk.size() > 11) merk = merk.substr(0, 8) + "...";
    string jenis = p.jenis;
    if ((int)jenis.size() > 13) jenis = jenis.substr(0, 10) + "...";
    string harga = rupiahFormat(p.harga.satuan);
    if ((int)harga.size() > 15) harga = harga.substr(0, 12) + "...";
    // Stok sedikit (<= 10) tetap BRED, lainnya BGREEN
    string sc = (p.stok.jumlah <= 10) ? BRED : BGREEN;

    cout << BCYAN << "| " << BYELLOW   << left  << setw(3)  << no
         << BCYAN << " | " << BWHITE   << left  << setw(23) << nama
         << BCYAN << " | " << BMAGENTA << left  << setw(11) << merk
         << BCYAN << " | ";
    cetakHargaKolom(harga, 15);   // harga RIGHT-aligned, BHARGA
    cout << BCYAN << " | " << sc       << left  << setw(5)  << p.stok.jumlah
         << BCYAN << " | " << BBLUE    << left  << setw(13) << jenis
         << BCYAN << " |\n" << RESET;
}

void cetakFooterTabel() {
    cout << BCYAN << "+-----+-------------------------+-------------+-----------------+-------+---------------+\n" << RESET;
}

int tampilTabel(int stockOnly) {
    cetakHeaderTabel();
    int no = 0, adaData = 0;
    for (int i = 0; i < totalProduk; i++) {
        if (daftarProduk[i].aktif == 0) continue;
        if (stockOnly == 1 && daftarProduk[i].stok.jumlah == 0) continue;
        no++;
        cetakBarisProdukTabel(no, daftarProduk[i]);
        adaData = 1;
    }
    if (!adaData) {
        cout << BCYAN << "|  " << BYELLOW << left << setw(86) << "(belum ada data produk)"
             << BCYAN << "|\n" << RESET;
    }
    cetakFooterTabel();
    return no;
}

// ===================== SORTING =====================

void selectionSortHargaAsc(Produk arr[], int n) {
    for (int i = 0; i < n - 1; i++) {
        int idxMin = i;
        for (int j = i + 1; j < n; j++)
            if (arr[j].harga.satuan < arr[idxMin].harga.satuan) idxMin = j;
        if (idxMin != i) { Produk t = arr[i]; arr[i] = arr[idxMin]; arr[idxMin] = t; }
    }
}
void bubbleSortNamaDesc(Produk arr[], int n) {
    for (int i = 0; i < n - 1; i++)
        for (int j = 0; j < n - i - 1; j++)
            if (arr[j].nama < arr[j+1].nama) { Produk t = arr[j]; arr[j] = arr[j+1]; arr[j+1] = t; }
}
void insertionSortStokAsc(Produk arr[], int n) {
    for (int i = 1; i < n; i++) {
        Produk k = arr[i]; int j = i - 1;
        while (j >= 0 && arr[j].stok.jumlah > k.stok.jumlah) { arr[j+1] = arr[j]; j--; }
        arr[j+1] = k;
    }
}

void tampilDenganSorting(int mode) {
    Produk tmp[100]; int jml = 0;
    for (int i = 0; i < totalProduk; i++)
        if (daftarProduk[i].aktif) tmp[jml++] = daftarProduk[i];
    if (jml == 0) { cout << BYELLOW << "  Belum ada data produk.\n" << RESET; return; }
    if      (mode == 1) bubbleSortNamaDesc(tmp, jml);
    else if (mode == 2) selectionSortHargaAsc(tmp, jml);
    else if (mode == 3) insertionSortStokAsc(tmp, jml);
    cetakHeaderTabel();
    for (int i = 0; i < jml; i++) cetakBarisProdukTabel(i + 1, tmp[i]);
    cetakFooterTabel();
}

int pilihNomor(string aksi) {
    int peta[100], no = 0;
    for (int i = 0; i < totalProduk; i++)
        if (daftarProduk[i].aktif) peta[no++] = i;
    tampilTabel(0);
    if (no == 0) { cout << BYELLOW << "  Belum ada produk.\n" << RESET; return -1; }
    try {
        string pilih;
        cout << BGREEN << "\n  " << aksi << " produk nomor berapa? " << BYELLOW;
        getline(cin, pilih); cout << RESET;
        int nomor = bacaAngka(pilih);
        if (nomor < 1 || nomor > no)
            throw out_of_range("Pilih antara 1 sampai " + angkaKeStr(no) + ".");
        return peta[nomor - 1];
    } catch (const invalid_argument& e) { cout << BRED << "  [!] " << e.what() << RESET << "\n"; }
    catch (const out_of_range& e)       { cout << BRED << "  [!] " << e.what() << RESET << "\n"; }
    return -1;
}

// ===================== FITUR ADMIN =====================

void tambahProduk() {
    cetakHeader("TAMBAH PRODUK BARU", BGREEN);
    if (totalProduk >= MAKS_PRODUK) { cout << BRED << "  [!] Data produk sudah penuh!\n" << RESET; return; }
    try {
        Produk p; string hargaStr, stokStr;
        
        // Input Nama - tidak boleh ada angka
        cout << BCYAN << "  Nama produk : " << BWHITE; 
        getline(cin, p.nama);
        if (p.nama.empty()) throw invalid_argument("Nama produk tidak boleh kosong.");
        if (containsDigit(p.nama)) throw invalid_argument("Nama produk tidak boleh mengandung angka!");
        
        // Input Merk - tidak boleh ada angka
        cout << BCYAN << "  Merk        : " << BWHITE; 
        getline(cin, p.merk);
        if (p.merk.empty()) throw invalid_argument("Merk tidak boleh kosong.");
        if (containsDigit(p.merk)) throw invalid_argument("Merk tidak boleh mengandung angka!");
        
        // Input Harga - harus angka
        cout << BCYAN << "  Harga (Rp)  : " << BHARGA; 
        getline(cin, hargaStr);
        if (hargaStr.empty()) throw invalid_argument("Harga tidak boleh kosong.");
        if (containsLetter(hargaStr)) throw invalid_argument("Harga harus berupa angka, tidak boleh ada huruf!");
        
        // Input Stok - harus angka
        cout << BCYAN << "  Stok        : " << BWHITE; 
        getline(cin, stokStr);
        if (stokStr.empty()) throw invalid_argument("Stok tidak boleh kosong.");
        if (containsLetter(stokStr)) throw invalid_argument("Stok harus berupa angka, tidak boleh ada huruf!");
        
        // Input Jenis - tidak boleh ada angka
        cout << BCYAN << "  Jenis       : " << BBLUE; 
        getline(cin, p.jenis); 
        cout << RESET;
        if (p.jenis.empty()) throw invalid_argument("Jenis tidak boleh kosong.");
        if (containsDigit(p.jenis)) throw invalid_argument("Jenis tidak boleh mengandung angka!");
        
        // Konversi dan simpan
        p.harga.satuan = bacaHarga(hargaStr);
        p.harga.matauang = "IDR";
        p.stok.jumlah = bacaAngka(stokStr);
        p.stok.satuan = "pcs";
        p.aktif = 1;
        daftarProduk[totalProduk++] = p;
        cout << BGREEN << "  [OK] Produk '" << p.nama << "' berhasil ditambahkan!\n" << RESET;
    } catch (const invalid_argument& e) { cout << BRED << "  [!] " << e.what() << RESET << "\n"; }
    catch (const out_of_range& e)       { cout << BRED << "  [!] " << e.what() << RESET << "\n"; }
}

void editProduk() {
    cetakHeader("EDIT DATA PRODUK", BYELLOW);
    int idx = pilihNomor("Edit");
    if (idx == -1) return;
    try {
        Produk &p = daftarProduk[idx];
        cout << BYELLOW << "\n  Edit produk (Enter = tidak berubah)\n" << RESET;
        string nb, mb, hb, sb, jb;
        
        cout << BCYAN << "  Nama  [" << BWHITE   << p.nama << BCYAN << "]: " << BWHITE; 
        getline(cin, nb);
        if (!nb.empty()) {
            if (containsDigit(nb)) throw invalid_argument("Nama tidak boleh mengandung angka!");
            p.nama = nb;
        }
        
        cout << BCYAN << "  Merk  [" << BMAGENTA << p.merk << BCYAN << "]: " << BWHITE; 
        getline(cin, mb);
        if (!mb.empty()) {
            if (containsDigit(mb)) throw invalid_argument("Merk tidak boleh mengandung angka!");
            p.merk = mb;
        }
        
        cout << BCYAN << "  Harga [" << BHARGA   << rupiahFormat(p.harga.satuan) << BCYAN << "]: " << BHARGA; 
        getline(cin, hb);
        if (!hb.empty()) { 
            if (containsLetter(hb)) throw invalid_argument("Harga harus berupa angka!");
            try { p.harga.satuan = bacaHarga(hb); } 
            catch(const exception& e) { cout << BRED << "  [!] Harga tidak diubah.\n" << RESET; } 
        }
        
        cout << BCYAN << "  Stok  [" << BGREEN   << p.stok.jumlah << BCYAN << "]: " << BWHITE; 
        getline(cin, sb);
        if (!sb.empty()) { 
            if (containsLetter(sb)) throw invalid_argument("Stok harus berupa angka!");
            try { p.stok.jumlah = bacaAngka(sb); } 
            catch(const exception& e) { cout << BRED << "  [!] Stok tidak diubah.\n" << RESET; } 
        }
        
        cout << BCYAN << "  Jenis [" << BBLUE    << p.jenis << BCYAN << "]: " << BWHITE; 
        getline(cin, jb); 
        cout << RESET;
        if (!jb.empty()) {
            if (containsDigit(jb)) throw invalid_argument("Jenis tidak boleh mengandung angka!");
            p.jenis = jb;
        }
        
        cout << BGREEN << "  [OK] Data berhasil diupdate!\n" << RESET;
    } catch (const exception& e) { cout << BRED << "  [!] " << e.what() << RESET << "\n"; }
}

void nonaktifkanProduk(int *s, string nama) {
    if (!s) throw runtime_error("Pointer null!");
    *s = 0;
    cout << BGREEN << "  [OK] Produk '" << nama << "' dihapus!\n" << RESET;
}

void hapusProduk() {
    cetakHeader("HAPUS PRODUK", BRED);
    int idx = pilihNomor("Hapus");
    if (idx == -1) return;
    try {
        string k;
        while (true) {
            cout << BYELLOW << "  Yakin hapus '" << BWHITE << daftarProduk[idx].nama << BYELLOW << "'? (y/n): " << BWHITE;
            getline(cin, k); cout << RESET;
            if (k=="y"||k=="Y"||k=="n"||k=="N") break;
            cout << BRED << "  Input tidak valid!\n" << RESET;
        }
        if (k=="y"||k=="Y") nonaktifkanProduk(&daftarProduk[idx].aktif, daftarProduk[idx].nama);
        else cout << BYELLOW << "  Dibatalkan.\n" << RESET;
    } catch (const runtime_error& e) { cout << BRED << "  [!] " << e.what() << RESET << "\n"; }
}

// ===================== FITUR PELANGGAN =====================

void cariNama() {
    cetakHeader("CARI PRODUK BY JENIS", BCYAN);
    int jml = 0;
    for (int i = 0; i < totalProduk; i++) if (daftarProduk[i].aktif) jml++;
    if (jml == 0) { cout << BYELLOW << "  Belum ada produk.\n" << RESET; return; }
    cout << BCYAN << "  Jenis tersedia : " << RESET;
    cout << BMAGENTA << "Serum" << RESET << ", " << BMAGENTA << "Toner" << RESET << ", "
         << BMAGENTA << "Moisturizer" << RESET << ", " << BMAGENTA << "Face Wash" << RESET << ", "
         << BMAGENTA << "Sunscreen" << RESET << "\n";
    cout << BCYAN << "  (ketik " << BYELLOW << "semua" << BCYAN << " untuk tampilkan semua jenis)\n" << RESET;
    cout << "\n";
    try {
        string keyword;
        cout << BGREEN << "  Cari jenis skincare: " << BYELLOW;
        getline(cin, keyword); cout << RESET;
        if (keyword.empty()) throw invalid_argument("Input tidak boleh kosong.");
        string kwLower = keyword;
        for (int i = 0; i < (int)kwLower.size(); i++) kwLower[i] = tolower(kwLower[i]);
        bool cariSemua = (kwLower == "semua");
        cetakHeaderTabel();
        int ketemu = 0;
        for (int i = 0; i < totalProduk; i++) {
            if (!daftarProduk[i].aktif) continue;
            string jpLower = daftarProduk[i].jenis;
            for (int j = 0; j < (int)jpLower.size(); j++) jpLower[j] = tolower(jpLower[j]);
            bool cocok = cariSemua || (jpLower.find(kwLower) != string::npos);
            if (!cocok) continue;
            ketemu++;
            cetakBarisProdukTabel(ketemu, daftarProduk[i]);
        }
        if (ketemu == 0) {
            cout << BCYAN << "|  " << BRED << left << setw(86)
                 << ("(tidak ada produk untuk jenis \"" + keyword + "\")")
                 << BCYAN << "|\n" << RESET;
            cetakFooterTabel();
            cout << BYELLOW << "\n  Mungkin maksud kamu salah satu ini:\n" << RESET;
            string jenisUnik[20]; int jmlJenis = 0;
            for (int i = 0; i < totalProduk; i++) {
                if (!daftarProduk[i].aktif) continue;
                bool sudahAda = false;
                for (int j = 0; j < jmlJenis; j++)
                    if (jenisUnik[j] == daftarProduk[i].jenis) { sudahAda = true; break; }
                if (!sudahAda) jenisUnik[jmlJenis++] = daftarProduk[i].jenis;
            }
            for (int i = 0; i < jmlJenis; i++)
                cout << BMAGENTA << "    -> " << BWHITE << jenisUnik[i] << "\n" << RESET;
        } else {
            cetakFooterTabel();
            cout << BGREEN << "  [OK] Ditemukan " << ketemu << " produk untuk jenis \""
                 << keyword << "\".\n" << RESET;
        }
    } catch (const invalid_argument& e) { cout << BRED << "  [!] " << e.what() << RESET << "\n"; }
}

void cariHarga() {
    cetakHeader("CARI PRODUK BY HARGA", BYELLOW);
    Produk tmp[100]; int jml = 0;
    for (int i = 0; i < totalProduk; i++) if (daftarProduk[i].aktif) tmp[jml++] = daftarProduk[i];
    if (jml == 0) { cout << BYELLOW << "  Belum ada produk.\n" << RESET; return; }
    selectionSortHargaAsc(tmp, jml);
    try {
        string inp;
        cout << BGREEN << "  Masukkan harga (Rp): " << BHARGA;
        getline(cin, inp); cout << RESET;
        double target = bacaHarga(inp);
        int low = 0, high = jml - 1, posisi = -1;
        while (low <= high) {
            int mid = low + (high - low) / 2;
            if (tmp[mid].harga.satuan == target) { posisi = mid; break; }
            else if (tmp[mid].harga.satuan < target) low = mid + 1;
            else high = mid - 1;
        }
        if (posisi == -1) {
            cout << BRED << "  Harga " << rupiahFormat(target) << " tidak ada.\n" << RESET;
            double selMin = -1, hTerdekat = 0;
            for (int i = 0; i < jml; i++) {
                double s = tmp[i].harga.satuan - target; if (s<0) s=-s;
                if (selMin<0||s<selMin) { selMin=s; hTerdekat=tmp[i].harga.satuan; }
            }
            cout << BYELLOW << "  Produk harga terdekat (" << BHARGA << rupiahFormat(hTerdekat) << BYELLOW << "):\n" << RESET;
            cetakHeaderTabel();
            int no = 0;
            for (int i = 0; i < jml; i++)
                if (tmp[i].harga.satuan == hTerdekat) cetakBarisProdukTabel(++no, tmp[i]);
        } else {
            cetakHeaderTabel();
            int no = 0;
            for (int i = 0; i < jml; i++)
                if (tmp[i].harga.satuan == target) cetakBarisProdukTabel(++no, tmp[i]);
            cout << BGREEN << "  [OK] Ditemukan " << no << " produk.\n" << RESET;
        }
        cetakFooterTabel();
    } catch (const invalid_argument& e) { cout << BRED << "  [!] " << e.what() << RESET << "\n"; }
    catch (const out_of_range& e)       { cout << BRED << "  [!] " << e.what() << RESET << "\n"; }
}

// ===================== MENU LIHAT PRODUK =====================

void menuKelolaLihatProduk() {
    string p;
    while (true) {
        cetakHeader("TAMPILAN PRODUK", BCYAN);
        cout << BYELLOW << "  -- Lihat --\n" << RESET;
        cout << BMAGENTA << "  [1]" << BWHITE << " Semua Produk (Default)\n" << RESET;
        cout << BYELLOW << "  -- Urutkan --\n" << RESET;
        cout << BMAGENTA << "  [2]" << BWHITE << " Nama Z -> A\n" << RESET;
        cout << BMAGENTA << "  [3]" << BWHITE << " Harga Termurah -> Termahal\n" << RESET;
        cout << BMAGENTA << "  [4]" << BWHITE << " Stok Sedikit -> Terbanyak\n" << RESET;
        cout << BYELLOW << "  -- Cari --\n" << RESET;
        cout << BMAGENTA << "  [5]" << BWHITE << " Cari by Jenis\n" << RESET;
        cout << BMAGENTA << "  [6]" << BWHITE << " Cari by Harga\n" << RESET;
        cout << BRED     << "  [0]" << BWHITE << " Kembali\n" << RESET;
        cetakGaris(BCYAN);
        cout << BGREEN << "  Pilih: " << BYELLOW; getline(cin, p); cout << RESET;
        if      (p=="1") { cout << BYELLOW << "\n  [Semua Produk]\n" << RESET; tampilTabel(0); }
        else if (p=="2") { cout << BYELLOW << "\n  [Nama Z -> A]\n" << RESET; tampilDenganSorting(1); }
        else if (p=="3") { cout << BYELLOW << "\n  [Harga Termurah]\n" << RESET; tampilDenganSorting(2); }
        else if (p=="4") { cout << BYELLOW << "\n  [Stok Sedikit]\n" << RESET; tampilDenganSorting(3); }
        else if (p=="5") cariNama();
        else if (p=="6") cariHarga();
        else if (p=="0") return;
        else cout << BRED << "  [!] Pilihan tidak valid.\n" << RESET;
    }
}

// ===================== KERANJANG BELANJA =====================
// Kolom: No(3) | Nama Produk(20) | Qty(5) | Harga Satuan(15) | Subtotal(15)
// Harga Satuan dan Subtotal RIGHT-aligned, warna BHARGA/BGREEN

void tampilKeranjang(ItemKeranjang keranjang[], int jmlKeranjang) {
    cetakHeader("KERANJANG BELANJA", BMAGENTA);
    cout << BMAGENTA << "+-----+----------------------+-------+-----------------+-----------------+\n";
    cout << "| " << BYELLOW  << left  << setw(3)  << "No"
         << BMAGENTA << " | " << BGREEN   << left  << setw(20) << "Nama Produk"
         << BMAGENTA << " | " << BCYAN    << right << setw(5)  << "Qty"
         << BMAGENTA << " | " << BHARGA   << right << setw(15) << "Harga Satuan"   // kuning, right
         << BMAGENTA << " | " << BGREEN   << right << setw(15) << "Subtotal"       // hijau, right
         << BMAGENTA << " |\n";
    cout << "+-----+----------------------+-------+-----------------+-----------------+\n" << RESET;

    double grandTotal = 0;
    for (int i = 0; i < jmlKeranjang; i++) {
        Produk &p = daftarProduk[keranjang[i].idxProduk];
        double sub = p.harga.satuan * keranjang[i].jumlah;
        grandTotal += sub;
        string nama = p.nama;
        if ((int)nama.size() > 20) nama = nama.substr(0, 17) + "...";
        string hargaStr = rupiahFormat(p.harga.satuan);
        if ((int)hargaStr.size() > 15) hargaStr = hargaStr.substr(0, 12) + "...";
        string subStr = rupiahFormat(sub);
        if ((int)subStr.size() > 15) subStr = subStr.substr(0, 12) + "...";

        cout << BMAGENTA << "| " << BYELLOW  << left  << setw(3)  << (i + 1)
             << BMAGENTA << " | " << BWHITE   << left  << setw(20) << nama
             << BMAGENTA << " | " << BCYAN    << right << setw(5)  << keranjang[i].jumlah
             << BMAGENTA << " | " << BHARGA   << right << setw(15) << hargaStr    // kuning
             << BMAGENTA << " | " << BGREEN   << right << setw(15) << subStr      // hijau
             << BMAGENTA << " |\n" << RESET;
    }
    cout << BMAGENTA << "+-----+----------------------+-------+-----------------+-----------------+\n" << RESET;
    cout << BOLD << BWHITE << "  TOTAL: " << BHARGA << rupiahFormat(grandTotal) << RESET << "\n";
    cetakGaris(BMAGENTA);
}

void kurangiStok(int *s, int jumlah) {
    if (!s) throw runtime_error("Pointer stok null!");
    if (jumlah <= 0) throw invalid_argument("Jumlah harus > 0.");
    if (*s < jumlah) throw out_of_range("Stok tidak mencukupi!");
    *s -= jumlah;
}

int tampilProdukMerk(string merk, int peta[], int modeSort) {
    Produk tmp[100]; int idxAsli[100]; int jml = 0;
    for (int i = 0; i < totalProduk; i++) {
        if (!daftarProduk[i].aktif) continue;
        string md = daftarProduk[i].merk, mc = merk;
        for (int j = 0; j < (int)md.size(); j++) md[j] = tolower(md[j]);
        for (int j = 0; j < (int)mc.size(); j++) mc[j] = tolower(mc[j]);
        if (md != mc) continue;
        tmp[jml] = daftarProduk[i]; idxAsli[jml] = i; jml++;
    }
    if (modeSort==1) bubbleSortNamaDesc(tmp, jml);
    else if (modeSort==2) selectionSortHargaAsc(tmp, jml);
    else if (modeSort==3) insertionSortStokAsc(tmp, jml);
    for (int i = 0; i < jml; i++)
        for (int k = 0; k < totalProduk; k++)
            if (daftarProduk[k].nama==tmp[i].nama && daftarProduk[k].merk==tmp[i].merk && daftarProduk[k].aktif)
                { idxAsli[i]=k; break; }
    string label = "KATALOG: " + merk;
    cetakHeader(label, BCYAN);
    if      (modeSort==1) cout << BCYAN << "  [Urutan: Nama Z -> A]\n" << RESET;
    else if (modeSort==2) cout << BCYAN << "  [Urutan: Harga Termurah]\n" << RESET;
    else if (modeSort==3) cout << BCYAN << "  [Urutan: Stok Sedikit]\n" << RESET;
    cetakHeaderTabel();
    for (int i = 0; i < jml; i++) {
        peta[i] = idxAsli[i];
        cetakBarisProdukTabel(i + 1, tmp[i]);
    }
    if (jml == 0) {
        cout << BCYAN << "|  " << BYELLOW << left << setw(86) << "(belum ada produk untuk merk ini)"
             << BCYAN << "|\n" << RESET;
    }
    cetakFooterTabel();
    return jml;
}

int menuSortingPelanggan() {
    string p;
    while (true) {
        cetakHeader("PILIH URUTAN TAMPILAN", BBLUE);
        cout << BMAGENTA << "  [1]" << BWHITE << " Pilih merek & Keranjang\n" << RESET;
        cout << BMAGENTA << "  [2]" << BWHITE << " Harga Termurah -> Termahal\n" << RESET;
        cout << BRED     << "  [0]" << BWHITE << " Kembali\n" << RESET;
        cetakGaris(BBLUE);
        cout << BGREEN << "  Pilih: " << BYELLOW; getline(cin, p); cout << RESET;
        if      (p=="1") { cout << BCYAN << "  -> Nama Z -> A\n" << RESET; return 1; }
        else if (p=="2") { cout << BCYAN << "  -> Harga Termurah\n" << RESET; return 2; }
        else if (p=="0") return -1;
        else cout << BRED << "  [!] Tidak valid!\n" << RESET;
    }
}

void tambahKeKeranjangDariMerk(ItemKeranjang keranjang[], int &jmlKeranjang, int peta[], int no) {
    try {
        if (jmlKeranjang >= MAKS_KERANJANG)
            throw length_error("Keranjang penuh!");
        string pp, pj;
        cout << BGREEN << "  Nomor produk (0=batal): " << BYELLOW;
        getline(cin, pp); cout << RESET;
        if (pp=="0") { cout << BYELLOW << "  Dibatalkan.\n" << RESET; return; }
        int npm = bacaAngka(pp);
        if (npm<1||npm>no) throw out_of_range("Nomor di luar jangkauan.");
        int idx = peta[npm-1];
        Produk &p = daftarProduk[idx];
        int diKeranjang = 0;
        for (int i=0;i<jmlKeranjang;i++) if(keranjang[i].idxProduk==idx) diKeranjang+=keranjang[i].jumlah;
        int tersedia = p.stok.jumlah - diKeranjang;
        if (tersedia<=0) throw out_of_range("Stok habis di keranjang!");
        cout << BGREEN << "  Jumlah '" << BWHITE << p.nama << BGREEN << "' (stok: " << tersedia << "): " << BYELLOW;
        getline(cin, pj); cout << RESET;
        int jml = bacaAngka(pj);
        if (jml<=0) throw invalid_argument("Jumlah harus > 0.");
        if (jml>tersedia) throw out_of_range("Melebihi stok tersedia!");
        int ada=0;
        for (int i=0;i<jmlKeranjang;i++) if(keranjang[i].idxProduk==idx){keranjang[i].jumlah+=jml;ada=1;break;}
        if (!ada) {keranjang[jmlKeranjang].idxProduk=idx; keranjang[jmlKeranjang].jumlah=jml; jmlKeranjang++;}
        cout << BGREEN << "  [OK] " << p.nama << " x" << jml << " masuk keranjang!\n" << RESET;
    } catch (const length_error& e)    { cout << BRED << "  [!] " << e.what() << RESET << "\n"; }
    catch (const invalid_argument& e)  { cout << BRED << "  [!] " << e.what() << RESET << "\n"; }
    catch (const out_of_range& e)      { cout << BRED << "  [!] " << e.what() << RESET << "\n"; }
}

// ===================== STRUK BELANJA =====================
// Lebar struk: 62 karakter total (isi 60)
// Border: "+------------------------------------------------------------+"
// Layout : "| label(16)  : value(40) |"
#define STRUK_W      62
#define STRUK_INNER  60
#define STRUK_SEP    "+------------------------------------------------------------+"
#define STRUK_ITEM_W 24   // lebar kolom nama produk di struk
#define STRUK_QTY_W   5   // lebar kolom qty
#define STRUK_SUB_W  16   // lebar kolom subtotal (right-aligned)

// Cetak baris label:value di struk, rata kiri, value color
void cetakStrukBaris(string label, string value, string labelColor, string valueColor) {
    // "| label           : value                              |"
    // label fixed 14 chars, ": " = 2, value sisa = STRUK_INNER - 14 - 2 - 2 (spasi kiri kanan)
    int valueW = STRUK_INNER - 4 - 16;  // 4 = "| " + " |", 16 = label(14) + ": "
    cout << BCYAN << "| " << RESET
         << labelColor << left << setw(14) << label << RESET
         << BCYAN << ": " << RESET
         << valueColor << left << setw(valueW) << value << RESET
         << BCYAN << " |\n" << RESET;
}

void prosesCheckout(ItemKeranjang keranjang[], int &jmlKeranjang, string namaUser) {
    if (jmlKeranjang==0) { cout << BRED << "  [!] Keranjang kosong!\n" << RESET; return; }
    tampilKeranjang(keranjang, jmlKeranjang);
    string k;
    while (true) {
        cout << BYELLOW << "  Lanjut bayar? (y/n): " << BWHITE;
        getline(cin, k); cout << RESET;
        if (k=="y"||k=="Y"||k=="n"||k=="N") break;
        cout << BRED << "  [!] Input tidak valid!\n" << RESET;
    }
    if (k!="y"&&k!="Y") { cout << BYELLOW << "  Checkout dibatalkan.\n" << RESET; return; }
    try {
        string metode="", status="", pm;
        double bayar=0, kembali=0; int adaKembali=0;
        cout << "\n" << BYELLOW << "  -- Metode Pembayaran --\n" << RESET;
        cout << BMAGENTA << "  [1]" << BWHITE << " Tunai\n" << RESET;
        cout << BMAGENTA << "  [2]" << BWHITE << " Transfer Bank\n" << RESET;
        cout << BMAGENTA << "  [3]" << BWHITE << " QRIS\n" << RESET;
        cout << BMAGENTA << "  [4]" << BWHITE << " Kartu Debit/Kredit\n" << RESET;
        cout << BGREEN << "  Pilih: " << BYELLOW; getline(cin, pm); cout << RESET;
        double total=0;
        for (int i=0;i<jmlKeranjang;i++)
            total += daftarProduk[keranjang[i].idxProduk].harga.satuan * keranjang[i].jumlah;
        if (pm=="1") {
            metode="Tunai"; string ib;
            cout << BGREEN << "  Jumlah bayar (Rp): " << BHARGA; getline(cin, ib); cout << RESET;
            bayar = bacaHarga(ib);
            if (bayar<total) throw runtime_error("Uang kurang " + rupiahFormat(total-bayar) + "!");
            kembali=bayar-total; adaKembali=1; status="LUNAS";
        } else if (pm=="2") { metode="Transfer Bank"; status="LUNAS (Transfer)"; }
        else if (pm=="3")   { metode="QRIS";          status="LUNAS (QRIS)"; }
        else if (pm=="4")   { metode="Kartu Debit/Kredit"; status="LUNAS (Kartu)"; }
        else throw invalid_argument("Metode tidak valid.");
        for (int i=0;i<jmlKeranjang;i++)
            kurangiStok(&daftarProduk[keranjang[i].idxProduk].stok.jumlah, keranjang[i].jumlah);

        string wt = getWaktuSekarang(), nt = generateNoTransaksi();

        // ===== CETAK STRUK =====
        cout << "\n";
        cout << BGREEN << STRUK_SEP << "\n" << RESET;

        // -- Judul tengah --
        {
            string judul = "STRUK PEMBELIAN - GLOW UP STORE";
            int pad = (STRUK_INNER - (int)judul.size()) / 2;
            int padR = STRUK_INNER - (int)judul.size() - pad;
            cout << BGREEN << "|";
            for(int i=0;i<pad;i++) cout<<" ";
            cout << BOLD << BWHITE << judul << RESET << BGREEN;
            for(int i=0;i<padR;i++) cout<<" ";
            cout << "|\n" << RESET;
        }
        cout << BGREEN << STRUK_SEP << "\n" << RESET;

        cetakStrukBaris("No. Transaksi", nt,       BCYAN,    BWHITE);
        cetakStrukBaris("Waktu",         wt,       BCYAN,    BWHITE);
        cetakStrukBaris("Pelanggan",     namaUser, BCYAN,    BMAGENTA);

        cout << BGREEN << STRUK_SEP << "\n" << RESET;

        // -- Header kolom item --
        // "| Produk(24) | Qty(5) | Subtotal(16) |"
        // total: 2 + 24 + 3 + 5 + 3 + 16 + 2 = 55... sesuaikan
        // border: "| " + nama(24) + " | " + qty(5) + " | " + sub(16) + " |" = 2+24+3+5+3+16+2 = 55 -> kurang
        // pakai STRUK_ITEM_W=24, STRUK_QTY_W=5, STRUK_SUB_W=16 -> 2+24+3+5+3+16+2=55, perlu 60
        // Tambah padding: nama=28, qty=5, sub=17 -> 2+28+3+5+3+17+2=60 ✓
        #define SW_NAMA 28
        #define SW_QTY   5
        #define SW_SUB  17
        // header separator khusus item
        cout << BCYAN << "+";
        for(int i=0;i<SW_NAMA+2;i++) cout<<"-";
        cout << "+";
        for(int i=0;i<SW_QTY+2;i++) cout<<"-";
        cout << "+";
        for(int i=0;i<SW_SUB+2;i++) cout<<"-";
        cout << "+\n" << RESET;

        // header row
        {
            // "Produk" center dalam SW_NAMA
            string hP="Produk", hQ="Qty", hS="Subtotal";
            int pPL=(SW_NAMA-(int)hP.size())/2, pPR=SW_NAMA-(int)hP.size()-pPL;
            int pQL=(SW_QTY-(int)hQ.size())/2,  pQR=SW_QTY-(int)hQ.size()-pQL;
            int pSL=(SW_SUB-(int)hS.size())/2,  pSR=SW_SUB-(int)hS.size()-pSL;
            cout << BCYAN << "| " << BGREEN;
            for(int i=0;i<pPL;i++) cout<<" ";
            cout<<hP;
            for(int i=0;i<pPR;i++) cout<<" ";
            cout << BCYAN << " | " << BYELLOW;
            for(int i=0;i<pQL;i++) cout<<" ";
            cout<<hQ;
            for(int i=0;i<pQR;i++) cout<<" ";
            cout << BCYAN << " | " << BHARGA;    // kuning untuk header Subtotal
            for(int i=0;i<pSL;i++) cout<<" ";
            cout<<hS;
            for(int i=0;i<pSR;i++) cout<<" ";
            cout << BCYAN << " |\n" << RESET;
        }
        cout << BCYAN << "+";
        for(int i=0;i<SW_NAMA+2;i++) cout<<"-";
        cout << "+";
        for(int i=0;i<SW_QTY+2;i++) cout<<"-";
        cout << "+";
        for(int i=0;i<SW_SUB+2;i++) cout<<"-";
        cout << "+\n" << RESET;

        // -- Baris item --
        int totalQty = 0;
        for (int i=0;i<jmlKeranjang;i++) {
            Produk &p=daftarProduk[keranjang[i].idxProduk];
            double sub=p.harga.satuan*keranjang[i].jumlah;
            totalQty += keranjang[i].jumlah;
            string nama=p.nama;
            if((int)nama.size()>SW_NAMA) nama=nama.substr(0,SW_NAMA-3)+"...";
            string subStr=rupiahFormat(sub);
            if((int)subStr.size()>SW_SUB) subStr=subStr.substr(0,SW_SUB-3)+"...";
            // nama left, qty right, subtotal right
            cout << BCYAN << "| " << RESET
                 << BWHITE << left  << setw(SW_NAMA) << nama  << RESET
                 << BCYAN << " | " << RESET
                 << BYELLOW << right << setw(SW_QTY)  << keranjang[i].jumlah << RESET
                 << BCYAN << " | " << RESET
                 << BHARGA << right << setw(SW_SUB)  << subStr << RESET   // kuning, right
                 << BCYAN << " |\n" << RESET;
        }

        // -- Garis bawah item + baris TOTAL --
        cout << BCYAN << "+";
        for(int i=0;i<SW_NAMA+2;i++) cout<<"-";
        cout << "+";
        for(int i=0;i<SW_QTY+2;i++) cout<<"-";
        cout << "+";
        for(int i=0;i<SW_SUB+2;i++) cout<<"-";
        cout << "+\n" << RESET;

        {
            string totalStr = rupiahFormat(total);
            if((int)totalStr.size()>SW_SUB) totalStr=totalStr.substr(0,SW_SUB-3)+"...";
            cout << BCYAN << "| " << RESET
                 << BOLD << BWHITE  << left  << setw(SW_NAMA) << "TOTAL" << RESET
                 << BCYAN << " | " << RESET
                 << BOLD << BYELLOW << right << setw(SW_QTY)  << totalQty << RESET
                 << BCYAN << " | " << RESET
                 << BOLD << BHARGA  << right << setw(SW_SUB)  << totalStr << RESET   // kuning bold
                 << BCYAN << " |\n" << RESET;
        }

        cout << BGREEN << STRUK_SEP << "\n" << RESET;

        // -- Dibayar & Kembalian (hanya tunai) --
        if (adaKembali) {
            string bayarStr   = rupiahFormat(bayar);
            string kembaliStr = rupiahFormat(kembali);
            cetakStrukBaris("Dibayar",   bayarStr,   BCYAN, BHARGA);    // kuning
            cetakStrukBaris("Kembalian", kembaliStr, BCYAN, BGREEN);    // hijau
            cout << BGREEN << STRUK_SEP << "\n" << RESET;
        }

        cetakStrukBaris("Metode", metode, BCYAN, BMAGENTA);
        cetakStrukBaris("Status", status, BCYAN, BGREEN);
        cout << BGREEN << STRUK_SEP << "\n" << RESET;

        // -- Pesan penutup tengah --
        {
            string pesan = "Makasih udah belanja! Semoga cocok :)";
            int pad  = (STRUK_INNER - (int)pesan.size()) / 2;
            int padR = STRUK_INNER - (int)pesan.size() - pad;
            cout << BGREEN << "|";
            for(int i=0;i<pad;i++) cout<<" ";
            cout << BOLD << BMAGENTA << pesan << RESET << BGREEN;
            for(int i=0;i<padR;i++) cout<<" ";
            cout << "|\n" << RESET;
        }
        cout << BGREEN << STRUK_SEP << "\n" << RESET;

        jmlKeranjang=0;
    } catch (const invalid_argument& e) { cout << BRED << "  [!] " << e.what() << "\n  Checkout dibatalkan.\n" << RESET; }
    catch (const runtime_error& e)      { cout << BRED << "  [!] " << e.what() << "\n  Checkout dibatalkan.\n" << RESET; }
    catch (const out_of_range& e)       { cout << BRED << "  [!] " << e.what() << "\n  Checkout dibatalkan.\n" << RESET; }
}

// ===================== MENU MERK + KERANJANG =====================

void menuMerkDanKeranjang(string namaUser) {
    ItemKeranjang keranjang[50]; int jmlKeranjang=0;
    int modeSort = menuSortingPelanggan();
    if (modeSort==-1) return;
    string merkAktif=""; int peta[100]; int noProduk=0;
    string p;
    while (true) {
        cetakHeader("PILIH MERK & KERANJANG", BMAGENTA);
        cout << BCYAN << "  Urutan   : " << BWHITE;
        if (modeSort==1) cout << "Nama Z -> A\n";
        else if (modeSort==2) cout << "Harga Termurah\n";
        else cout << "Default\n";
        if (!merkAktif.empty())
            cout << BCYAN << "  Merk     : " << BWHITE << merkAktif << " (" << noProduk << " produk)\n";
        cout << BCYAN << "  Keranjang: " << BGREEN << jmlKeranjang << BCYAN << " item\n" << RESET;
        cout << "\n";
        cout << BYELLOW << "  -- Pilih Merk --\n" << RESET;
        cout << BMAGENTA << "  [1]" << BWHITE << " Wardah     " << BMAGENTA << "[2]" << BWHITE << " Glad2Glow\n";
        cout << BMAGENTA << "  [3]" << BWHITE << " Emina      " << BMAGENTA << "[4]" << BWHITE << " Originote\n";
        cout << BMAGENTA << "  [5]" << BWHITE << " Skintific\n" << RESET;
        cout << BYELLOW << "  -- Keranjang --\n" << RESET;
        cout << BMAGENTA << "  [6]" << BWHITE << " Lihat keranjang\n";
        cout << BMAGENTA << "  [7]" << BWHITE << " Hapus item\n";
        cout << BMAGENTA << "  [8]" << BWHITE << " Checkout\n";
        cout << BRED     << "  [0]" << BWHITE << " Kembali\n" << RESET;
        cetakGaris(BMAGENTA);
        cout << BGREEN << "  Pilih: " << BYELLOW; getline(cin, p); cout << RESET;
        string mb="";
        if (p=="1") mb="Wardah"; else if (p=="2") mb="Glad2Glow";
        else if (p=="3") mb="Emina"; else if (p=="4") mb="Originote";
        else if (p=="5") mb="Skintific";
        if (!mb.empty()) {
            merkAktif=mb;
            noProduk=tampilProdukMerk(merkAktif, peta, modeSort);
            if (noProduk>0) {
                string t;
                while(true) {
                    cout << BYELLOW << "  Tambah ke keranjang? (y/n): " << BWHITE;
                    getline(cin, t); cout << RESET;
                    if (t=="y"||t=="Y"||t=="n"||t=="N") break;
                    cout << BRED << "  [!] Input tidak valid!\n" << RESET;
                }
                if (t=="y"||t=="Y") tambahKeKeranjangDariMerk(keranjang, jmlKeranjang, peta, noProduk);
            }
            continue;
        }
        if (p=="6") {
            if (jmlKeranjang==0) { cout << BYELLOW << "  Keranjang kosong.\n" << RESET; continue; }
            tampilKeranjang(keranjang, jmlKeranjang); continue;
        }
        if (p=="7") {
            if (jmlKeranjang==0) { cout << BRED << "  [!] Keranjang kosong!\n" << RESET; continue; }
            tampilKeranjang(keranjang, jmlKeranjang);
            try {
                string hs;
                cout << BGREEN << "  Hapus nomor berapa? " << BYELLOW; getline(cin, hs); cout << RESET;
                int ih = bacaAngka(hs)-1;
                if (ih<0||ih>=jmlKeranjang) throw out_of_range("Nomor di luar jangkauan.");
                string nh = daftarProduk[keranjang[ih].idxProduk].nama;
                string kh;
                while(true) {
                    cout << BYELLOW << "  Hapus '" << BWHITE << nh << BYELLOW << "'? (y/n): " << BWHITE;
                    getline(cin, kh); cout << RESET;
                    if (kh=="y"||kh=="Y"||kh=="n"||kh=="N") break;
                    cout << BRED << "  [!] Input tidak valid!\n" << RESET;
                }
                if (kh=="n"||kh=="N") { cout << BYELLOW << "  Dibatalkan.\n" << RESET; continue; }
                for (int i=ih;i<jmlKeranjang-1;i++) keranjang[i]=keranjang[i+1];
                jmlKeranjang--;
                cout << BGREEN << "  [OK] '" << nh << "' dihapus dari keranjang.\n" << RESET;
            } catch (const invalid_argument& e) { cout << BRED << "  [!] " << e.what() << RESET << "\n"; }
            catch (const out_of_range& e)       { cout << BRED << "  [!] " << e.what() << RESET << "\n"; }
            continue;
        }
        if (p=="8") { prosesCheckout(keranjang, jmlKeranjang, namaUser); continue; }

        if (p=="0") return;
        cout << BRED << "  [!] Pilihan tidak valid!\n" << RESET;
    }
}

// ===================== LOGIN & REGISTER =====================

void doRegister() {
    if (jumlahPengguna>=MAKS_PENGGUNA) { cout << BRED << "  [!] Slot pengguna penuh!\n" << RESET; return; }
    cetakHeader("DAFTAR AKUN BARU", BCYAN);
    try {
        string nama, pass;
        cout << BCYAN << "  Nama     : " << BWHITE; getline(cin, nama);
        if (nama.empty()) throw invalid_argument("Nama tidak boleh kosong.");
        for (int i=0;i<jumlahPengguna;i++)
            if (dataPengguna[i].nama==nama) throw runtime_error("Nama sudah dipakai!");
        cout << BCYAN << "  Password : " << BWHITE; getline(cin, pass); cout << RESET;
        if (pass.empty()) throw invalid_argument("Password tidak boleh kosong.");
        dataPengguna[jumlahPengguna++]={nama, pass, "pelanggan", 1};
        simpanPengguna();
        cout << BGREEN << "  [OK] Akun '" << nama << "' berhasil dibuat!\n" << RESET;
    } catch (const invalid_argument& e) { cout << BRED << "  [!] " << e.what() << RESET << "\n"; }
    catch (const runtime_error& e)      { cout << BRED << "  [!] " << e.what() << RESET << "\n"; }
}

int doLogin(Pengguna* data, int jml) {
    int coba=0;
    while (coba<3) {
        cetakHeader("LOGIN GLOW UP STORE", BGREEN);
        try {
            string n, pw;
            cout << BCYAN << "  Nama     : " << BWHITE; getline(cin, n);
            cout << BCYAN << "  Password : " << BWHITE; getline(cin, pw); cout << RESET;
            if (n.empty()||pw.empty()) throw invalid_argument("Nama dan password tidak boleh kosong.");
            int idx=-1;
            for (int i=0;i<jml;i++)
                if (data[i].aktif&&data[i].nama==n&&data[i].password==pw) { idx=i; break; }
            if (idx!=-1) {
                cout << BGREEN << "\n  [OK] Login berhasil! Halo, " << BMAGENTA << data[idx].nama << BGREEN << "!\n" << RESET;
                simpanRiwayatLogin(data[idx].nama, data[idx].role, "BERHASIL");
                if (data[idx].role=="admin") menuAdmin(data[idx].nama);
                else menuPelanggan(data[idx].nama);
                return 1;
            }
            coba++;
            simpanRiwayatLogin(n, "-", "GAGAL");
            throw runtime_error("Nama atau password salah.");
        } catch (const invalid_argument& e) { cout << BRED << "  [!] " << e.what() << RESET << "\n"; coba++; }
        catch (const runtime_error& e) {
            cout << BRED << "\n  [!] " << e.what() << RESET << "\n";
            if (coba<3) cout << BYELLOW << "  Sisa percobaan: " << (3-coba) << RESET << "\n";
        }
    }
    cout << BRED << "\n  [!] Terlalu banyak percobaan.\n" << RESET;
    return 0;
}

// ===================== MENU ADMIN & PELANGGAN =====================

void menuAdmin(string namaUser) {
    string p;
    while (true) {
        cetakHeader("GLOW UP STORE - ADMIN", BYELLOW);
        cout << BYELLOW << "  Halo, " << BMAGENTA << namaUser << BYELLOW << "!\n\n" << RESET;
        cout << BMAGENTA << "  [1]" << BWHITE << " Tambah Produk\n";
        cout << BMAGENTA << "  [2]" << BWHITE << " Edit Produk\n";
        cout << BMAGENTA << "  [3]" << BWHITE << " Hapus Produk\n";
        cout << BMAGENTA << "  [4]" << BWHITE << " Lihat Produk\n";
        cout << BMAGENTA << "  [5]" << BWHITE << " Riwayat Login\n" << RESET;
        cout << BRED     << "  [0]" << BWHITE << " Logout\n" << RESET;
        cetakGaris(BYELLOW);
        cout << BGREEN << "  Pilih (0-5): " << BYELLOW; getline(cin, p); cout << RESET;
        if      (p=="1") tambahProduk();
        else if (p=="2") editProduk();
        else if (p=="3") hapusProduk();
        else if (p=="4") menuKelolaLihatProduk();
        else if (p=="5") { cetakHeader("RIWAYAT LOGIN", BCYAN); tampilRiwayatLogin(); }
        else if (p=="0") { cout << BGREEN << "  Sampai jumpa!\n" << RESET; return; }
        else cout << BRED << "  [!] Pilihan tidak valid.\n" << RESET;
    }
}

void menuPelanggan(string namaUser) {
    string p;
    while (true) {
        cetakHeader("GLOW UP STORE - PELANGGAN", BMAGENTA);
        cout << BMAGENTA << "  Halo, " << BCYAN << namaUser << BMAGENTA << "!\n\n" << RESET;
        cout << BMAGENTA << "  [1]" << BWHITE << " Pilih Merk Skincare\n";
        cout << BMAGENTA << "  [2]" << BWHITE << " Cari by Jenis\n" << RESET;
        cout << BRED     << "  [0]" << BWHITE << " Keluar\n" << RESET;
        cetakGaris(BMAGENTA);
        cout << BGREEN << "  Pilih (0-2): " << BYELLOW; getline(cin, p); cout << RESET;
        if      (p=="1") menuMerkDanKeranjang(namaUser);
        else if (p=="2") cariNama();
        else if (p=="0") { cout << BGREEN << "  Makasih sudah mampir! Sampai jumpa :)\n" << RESET; return; }
        else cout << BRED << "  [!] Pilihan tidak valid.\n" << RESET;
    }
}

// ===================== MAIN =====================

int main() {
    loadPengguna();
    int adminAda=0, tamuAda=0;
    for (int i=0;i<jumlahPengguna;i++) {
        if (dataPengguna[i].nama=="admin") adminAda=1;
        if (dataPengguna[i].nama=="Tamu")  tamuAda=1;
    }
    if (!adminAda) dataPengguna[jumlahPengguna++]={"admin","123","admin",1};
    if (!tamuAda)  dataPengguna[jumlahPengguna++]={"Tamu","000","pelanggan",1};

    // ===== WARDAH =====
    daftarProduk[totalProduk++]={"Lightening Serum",       "Wardah",   {55000, "IDR"},{40,"pcs"},"Serum",      1};
    daftarProduk[totalProduk++]={"Hydrating Toner",        "Wardah",   {38000, "IDR"},{30,"pcs"},"Toner",      1};
    daftarProduk[totalProduk++]={"Daily Moisturizer",      "Wardah",   {52000, "IDR"},{20,"pcs"},"Moisturizer",1};
    daftarProduk[totalProduk++]={"Face Wash Aloe",         "Wardah",   {30000, "IDR"},{25,"pcs"},"Face Wash",  1};
    daftarProduk[totalProduk++]={"UV Shield Sunscreen",    "Wardah",   {48000, "IDR"},{35,"pcs"},"Sunscreen",  1};
    // ===== GLAD2GLOW =====
    daftarProduk[totalProduk++]={"Brightening Serum",      "Glad2Glow",{62000, "IDR"},{25,"pcs"},"Serum",      1};
    daftarProduk[totalProduk++]={"Exfo Toner AHA BHA",     "Glad2Glow",{57000, "IDR"},{20,"pcs"},"Toner",      1};
    daftarProduk[totalProduk++]={"Moisturizer Gel",        "Glad2Glow",{45000, "IDR"},{30,"pcs"},"Moisturizer",1};
    daftarProduk[totalProduk++]={"Gentle Face Wash",       "Glad2Glow",{35000, "IDR"},{20,"pcs"},"Face Wash",  1};
    daftarProduk[totalProduk++]={"Daily Sun Protect SPF35","Glad2Glow",{53000, "IDR"},{22,"pcs"},"Sunscreen",  1};
    // ===== EMINA =====
    daftarProduk[totalProduk++]={"Acne Care Serum",        "Emina",    {42000, "IDR"},{28,"pcs"},"Serum",      1};
    daftarProduk[totalProduk++]={"Bright Stuff Toner",     "Emina",    {35000, "IDR"},{45,"pcs"},"Toner",      1};
    daftarProduk[totalProduk++]={"Ms Pimple Moisturizer",  "Emina",    {36000, "IDR"},{30,"pcs"},"Moisturizer",1};
    daftarProduk[totalProduk++]={"Bright Stuff Face Wash", "Emina",    {28000, "IDR"},{40,"pcs"},"Face Wash",  1};
    daftarProduk[totalProduk++]={"Sun Protection SPF30",   "Emina",    {40000, "IDR"},{50,"pcs"},"Sunscreen",  1};
    // ===== ORIGINOTE =====
    daftarProduk[totalProduk++]={"Niacinamide Serum",      "Originote",{72000, "IDR"},{20,"pcs"},"Serum",      1};
    daftarProduk[totalProduk++]={"Hyal Toner Essence",     "Originote",{65000, "IDR"},{18,"pcs"},"Toner",      1};
    daftarProduk[totalProduk++]={"Hyalucera Moisturizer",  "Originote",{75000, "IDR"},{22,"pcs"},"Moisturizer",1};
    daftarProduk[totalProduk++]={"Mild Cleanser FaceWash", "Originote",{60000, "IDR"},{15,"pcs"},"Face Wash",  1};
    daftarProduk[totalProduk++]={"UV Barrier Sunscreen",   "Originote",{78000, "IDR"},{16,"pcs"},"Sunscreen",  1};
    // ===== SKINTIFIC =====
    daftarProduk[totalProduk++]={"SymWhite Serum",         "Skintific",{125000,"IDR"},{12,"pcs"},"Serum",      1};
    daftarProduk[totalProduk++]={"AHA BHA PHA Toner",      "Skintific",{105000,"IDR"},{14,"pcs"},"Toner",      1};
    daftarProduk[totalProduk++]={"5x Ceramide Barrier",    "Skintific",{115000,"IDR"},{15,"pcs"},"Moisturizer",1};
    daftarProduk[totalProduk++]={"Acne Facial Wash",       "Skintific",{95000, "IDR"},{14,"pcs"},"Face Wash",  1};
    daftarProduk[totalProduk++]={"AMPM Sunscreen SPF50",   "Skintific",{110000,"IDR"},{13,"pcs"},"Sunscreen",  1};

    cout << "\n";
    cetakGaris(BMAGENTA);
    cetakTengah(BG_MAGENTA, BWHITE, "SELAMAT DATANG DI GLOW UP STORE!");
    cetakTengah(BG_MAGENTA, BWHITE, "Toko Skincare Terpercaya No.1");
    cetakGaris(BMAGENTA);
    cout << "\n";

    string pilihan; int berjalan=1;
    while (berjalan) {
        cetakHeader("MENU UTAMA", BCYAN);
        cout << BMAGENTA << "  [1]" << BWHITE << " Register\n";
        cout << BMAGENTA << "  [2]" << BWHITE << " Login\n" << RESET;
        cout << BRED     << "  [0]" << BWHITE << " Keluar\n" << RESET;
        cetakGaris(BCYAN);
        cout << BGREEN << "  Pilih (0-2): " << BYELLOW; getline(cin, pilihan); cout << RESET;
        if      (pilihan=="1") doRegister();
        else if (pilihan=="2") { if (doLogin(dataPengguna, jumlahPengguna)==0) berjalan=0; }
        else if (pilihan=="0") {
            cetakGaris(BMAGENTA);
            cetakTengah(BG_MAGENTA, BWHITE, "Terima kasih! Keep Glowing Bestie!");
            cetakGaris(BMAGENTA);
            berjalan=0;
        }
        else cout << BRED << "  [!] Pilihan tidak valid.\n" << RESET;
    }
    return 0;
}