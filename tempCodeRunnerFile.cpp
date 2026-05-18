#include <iostream>
#include <iomanip>
#include <ctime>
#include <fstream>
#include <sstream>
#include <stdexcept>

using namespace std;

// ===================== ANSI COLOR CODES =====================
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

// ===================== HELPER TAMPILAN =====================
// Lebar standar semua kotak = 52 karakter isi + 2 border = 54

void cetakGaris(string warna, int panjang = 54) {
    cout << warna;
    for (int i = 0; i < panjang; i++) cout << "=";
    cout << RESET << "\n";
}

void cetakGarisTipis(string warna, int panjang = 54) {
    cout << warna;
    for (int i = 0; i < panjang; i++) cout << "-";
    cout << RESET << "\n";
}

// Cetak baris teks di dalam kotak, lebar isi = panjang-2
void cetakBaris(string warna, string teks, int panjang = 54) {
    int isi = panjang - 2;
    cout << warna << "| " << RESET << left << setw(isi - 2) << teks << warna << " |" << RESET << "\n";
}

// Cetak judul tengah di dalam kotak
void cetakTengah(string bgWarna, string teksWarna, string teks, int panjang = 54) {
    int isi = panjang - 2; // ruang dalam border
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

void cetakHeader(string judul, string warna, int panjang = 54) {
    cetakGaris(warna, panjang);
    cetakTengah(warna, BWHITE, judul, panjang);
    cetakGaris(warna, panjang);
}

// ===================== WAKTU & TRANSAKSI =====================

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

// ===================== SIMPAN & LOAD PENGGUNA =====================

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

// ===================== RIWAYAT LOGIN =====================

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

void tampilRiwayatLogin() {
    try {
        ifstream f(FILE_RIWAYAT_LOGIN.c_str());
        if (!f.is_open()) throw runtime_error("File riwayat login tidak ditemukan.");
        cout << BCYAN << "+---------------------+-----------------+------------+----------+\n";
        cout << "| " << BYELLOW << "Waktu               " << BCYAN
             << "| " << BGREEN  << "Nama            " << BCYAN
             << "| " << BMAGENTA<< "Role       " << BCYAN
             << "| " << BWHITE  << "Status   " << BCYAN << "|\n";
        cout << "+---------------------+-----------------+------------+----------+\n" << RESET;
        string baris; int ada = 0;
        while (getline(f, baris)) {
            string kolom[4]; int k = 0; string tmp = "";
            for (int i = 0; i < (int)baris.size(); i++) {
                if (baris[i] == '|' && k < 3) { kolom[k++] = tmp; tmp = ""; }
                else tmp += baris[i];
            }
            kolom[k] = tmp;
            string sc = (kolom[3].find("BERHASIL") != string::npos) ? BGREEN : BRED;
            cout << BCYAN << "| " << RESET << left << setw(20) << kolom[0]
                 << BCYAN << "| " << RESET << setw(16) << kolom[1]
                 << BCYAN << "| " << BMAGENTA << setw(11) << kolom[2]
                 << BCYAN << "| " << sc << setw(9) << kolom[3]
                 << BCYAN << "|\n" << RESET;
            ada = 1;
        }
        if (!ada)
            cout << BCYAN << "|         " << BYELLOW << "(belum ada riwayat login)"
                 << BCYAN << "                      |\n";
        cout << BCYAN << "+---------------------+-----------------+------------+----------+\n" << RESET;
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

int bacaAngka(string s) {
    if (s.empty()) throw invalid_argument("Input tidak boleh kosong.");
    if (!validAngka(s)) throw invalid_argument("Input harus berupa angka positif.");
    int hasil = 0;
    for (int i = 0; i < (int)s.size(); i++) hasil = hasil * 10 + (s[i] - '0');
    return hasil;
}

double bacaHarga(string s) {
    if (s.empty()) throw invalid_argument("Harga tidak boleh kosong.");
    if (!validAngka(s)) throw invalid_argument("Harga harus berupa angka positif.");
    double h = 0;
    for (int i = 0; i < (int)s.size(); i++) h = h * 10 + (s[i] - '0');
    if (h <= 0) throw out_of_range("Harga harus lebih dari 0.");
    return h;
}

// ===================== TAMPILAN TABEL PRODUK =====================
// Lebar tabel: 4+26+12+14+5+13 + border = 82

void cetakHeaderTabel() {
    cout << BCYAN << "+----+----------------------+----------+---------------+-----+-------------+\n";
    cout << "| " << BYELLOW << "No " << BCYAN
         << "| " << BGREEN  << "Nama Produk          " << BCYAN
         << "| " << BMAGENTA<< "Merk      " << BCYAN
         << "| " << BRED    << "Harga          " << BCYAN
         << "| " << BWHITE  << "Stok" << BCYAN
         << "| " << BBLUE   << "Jenis       " << BCYAN << "|\n";
    cout << "+----+----------------------+----------+---------------+-----+-------------+\n" << RESET;
}

int tampilTabel(int stockOnly) {
    cetakHeaderTabel();
    int no = 0, adaData = 0;
    for (int i = 0; i < totalProduk; i++) {
        if (daftarProduk[i].aktif == 0) continue;
        if (stockOnly == 1 && daftarProduk[i].stok.jumlah == 0) continue;
        no++;
        string nama = daftarProduk[i].nama;
        if ((int)nama.size() > 21) nama = nama.substr(0, 18) + "...";
        string merk = daftarProduk[i].merk;
        if ((int)merk.size() > 9) merk = merk.substr(0, 7) + "..";
        string sc = (daftarProduk[i].stok.jumlah <= 10) ? BRED : BGREEN;
        cout << BCYAN << "| " << BYELLOW << left << setw(3) << no
             << BCYAN << "| " << BWHITE  << setw(21) << nama
             << BCYAN << "| " << BMAGENTA<< setw(9)  << merk
             << BCYAN << "| " << BRED    << setw(14) << rupiahFormat(daftarProduk[i].harga.satuan)
             << BCYAN << "| " << sc      << setw(4)  << daftarProduk[i].stok.jumlah
             << BCYAN << "| " << BBLUE   << setw(12) << daftarProduk[i].jenis
             << BCYAN << "|\n" << RESET;
        adaData = 1;
    }
    if (!adaData)
        cout << BCYAN << "|        " << BYELLOW << "(belum ada data produk)"
             << BCYAN << "                              |\n";
    cout << BCYAN << "+----+----------------------+----------+---------------+-----+-------------+\n" << RESET;
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
    for (int i = 0; i < jml; i++) {
        string nama = tmp[i].nama;
        if ((int)nama.size() > 21) nama = nama.substr(0, 18) + "...";
        string merk = tmp[i].merk;
        if ((int)merk.size() > 9) merk = merk.substr(0, 7) + "..";
        string sc = (tmp[i].stok.jumlah <= 10) ? BRED : BGREEN;
        cout << BCYAN << "| " << BYELLOW << left << setw(3) << (i+1)
             << BCYAN << "| " << BWHITE  << setw(21) << nama
             << BCYAN << "| " << BMAGENTA<< setw(9)  << merk
             << BCYAN << "| " << BRED    << setw(14) << rupiahFormat(tmp[i].harga.satuan)
             << BCYAN << "| " << sc      << setw(4)  << tmp[i].stok.jumlah
             << BCYAN << "| " << BBLUE   << setw(12) << tmp[i].jenis
             << BCYAN << "|\n" << RESET;
    }
    cout << BCYAN << "+----+----------------------+----------+---------------+-----+-------------+\n" << RESET;
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
        cout << BCYAN << "  Nama produk : " << BWHITE; getline(cin, p.nama);
        cout << BCYAN << "  Merk        : " << BWHITE; getline(cin, p.merk);
        cout << BCYAN << "  Harga (Rp)  : " << BYELLOW; getline(cin, hargaStr);
        cout << BCYAN << "  Stok        : " << BWHITE; getline(cin, stokStr);
        cout << BCYAN << "  Jenis       : " << BBLUE; getline(cin, p.jenis); cout << RESET;
        if (p.nama.empty() || p.merk.empty() || p.jenis.empty())
            throw invalid_argument("Nama, merk, dan jenis tidak boleh kosong.");
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
        cout << BCYAN << "  Nama  [" << BWHITE << p.nama << BCYAN << "]: " << BWHITE; getline(cin, nb);
        if (!nb.empty()) p.nama = nb;
        cout << BCYAN << "  Merk  [" << BMAGENTA << p.merk << BCYAN << "]: " << BWHITE; getline(cin, mb);
        if (!mb.empty()) p.merk = mb;
        cout << BCYAN << "  Harga [" << BRED << rupiahFormat(p.harga.satuan) << BCYAN << "]: " << BYELLOW; getline(cin, hb);
        if (!hb.empty()) { try { p.harga.satuan = bacaHarga(hb); } catch(const exception& e) { cout << BRED << "  [!] Harga tidak diubah.\n" << RESET; } }
        cout << BCYAN << "  Stok  [" << BGREEN << p.stok.jumlah << BCYAN << "]: " << BWHITE; getline(cin, sb);
        if (!sb.empty()) { try { p.stok.jumlah = bacaAngka(sb); } catch(const exception& e) { cout << BRED << "  [!] Stok tidak diubah.\n" << RESET; } }
        cout << BCYAN << "  Jenis [" << BBLUE << p.jenis << BCYAN << "]: " << BWHITE; getline(cin, jb); cout << RESET;
        if (!jb.empty()) p.jenis = jb;
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
    cout << "\n";
    cout << BMAGENTA << "  [1]" << BWHITE << " Serum        " << BMAGENTA << "[2]" << BWHITE << " Toner\n";
    cout << BMAGENTA << "  [3]" << BWHITE << " Moisturizer  " << BMAGENTA << "[4]" << BWHITE << " Face Wash\n";
    cout << BMAGENTA << "  [5]" << BWHITE << " Sunscreen    " << BMAGENTA << "[6]" << BWHITE << " Cleanser\n";
    cout << BMAGENTA << "  [7]" << BWHITE << " Mask         " << BMAGENTA << "[8]" << BWHITE << " Semua Jenis\n" << RESET;
    cout << "\n" << BGREEN << "  Pilih: " << BYELLOW;
    try {
        string pil; getline(cin, pil); cout << RESET;
        string keyword = "";
        if      (pil=="1") keyword="serum";
        else if (pil=="2") keyword="toner";
        else if (pil=="3") keyword="moisturizer";
        else if (pil=="4") keyword="face wash";
        else if (pil=="5") keyword="sunscreen";
        else if (pil=="6") keyword="cleanser";
        else if (pil=="7") keyword="mask";
        else if (pil=="8") keyword="";
        else throw invalid_argument("Pilih angka 1-8.");
        cetakHeaderTabel();
        int ketemu = 0;
        for (int i = 0; i < totalProduk; i++) {
            if (!daftarProduk[i].aktif) continue;
            string jp = daftarProduk[i].jenis;
            for (int j = 0; j < (int)jp.size(); j++) jp[j] = tolower(jp[j]);
            if (keyword != "" && jp != keyword) continue;
            ketemu++;
            string nama = daftarProduk[i].nama;
            if ((int)nama.size() > 21) nama = nama.substr(0, 18) + "...";
            string merk = daftarProduk[i].merk;
            if ((int)merk.size() > 9) merk = merk.substr(0, 7) + "..";
            cout << BCYAN << "| " << BYELLOW << left << setw(3) << ketemu
                 << BCYAN << "| " << BWHITE  << setw(21) << nama
                 << BCYAN << "| " << BMAGENTA<< setw(9)  << merk
                 << BCYAN << "| " << BRED    << setw(14) << rupiahFormat(daftarProduk[i].harga.satuan)
                 << BCYAN << "| " << BGREEN  << setw(4)  << daftarProduk[i].stok.jumlah
                 << BCYAN << "| " << BBLUE   << setw(12) << daftarProduk[i].jenis
                 << BCYAN << "|\n" << RESET;
        }
        if (ketemu == 0)
            cout << BCYAN << "|     " << BYELLOW << "(tidak ada produk untuk jenis ini)"
                 << BCYAN << "                 |\n";
        cout << BCYAN << "+----+----------------------+----------+---------------+-----+-------------+\n" << RESET;
        if (ketemu > 0) cout << BGREEN << "  [OK] Ditemukan " << ketemu << " produk.\n" << RESET;
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
        cout << BGREEN << "  Masukkan harga (Rp): " << BYELLOW;
        getline(cin, inp); cout << RESET;
        double target = bacaHarga(inp);
        int low = 0, high = jml - 1, posisi = -1;
        while (low <= high) {
            int mid = low + (high - low) / 2;
            if (tmp[mid].harga.satuan == target) { posisi = mid; break; }
            else if (tmp[mid].harga.satuan < target) low = mid + 1;
            else high = mid - 1;
        }
        auto cetakBarisProduk = [](Produk &p, int no) {
            string nama = p.nama; if ((int)nama.size()>21) nama=nama.substr(0,18)+"...";
            string merk = p.merk; if ((int)merk.size()>9)  merk=merk.substr(0,7)+"..";
            string sc = (p.stok.jumlah<=10)?BRED:BGREEN;
            cout << BCYAN << "| " << BYELLOW << left << setw(3) << no
                 << BCYAN << "| " << BWHITE  << setw(21) << nama
                 << BCYAN << "| " << BMAGENTA<< setw(9)  << merk
                 << BCYAN << "| " << BRED    << setw(14) << rupiahFormat(p.harga.satuan)
                 << BCYAN << "| " << sc      << setw(4)  << p.stok.jumlah
                 << BCYAN << "| " << BBLUE   << setw(12) << p.jenis
                 << BCYAN << "|\n" << RESET;
        };
        if (posisi == -1) {
            cout << BRED << "  Harga " << rupiahFormat(target) << " tidak ada.\n" << RESET;
            double selMin = -1, hTerdekat = 0;
            for (int i = 0; i < jml; i++) {
                double s = tmp[i].harga.satuan - target; if (s<0) s=-s;
                if (selMin<0||s<selMin) { selMin=s; hTerdekat=tmp[i].harga.satuan; }
            }
            cout << BYELLOW << "  Produk harga terdekat (" << rupiahFormat(hTerdekat) << "):\n" << RESET;
            cetakHeaderTabel();
            int no = 0;
            for (int i = 0; i < jml; i++)
                if (tmp[i].harga.satuan == hTerdekat) cetakBarisProduk(tmp[i], ++no);
        } else {
            cetakHeaderTabel();
            int no = 0;
            for (int i = 0; i < jml; i++)
                if (tmp[i].harga.satuan == target) cetakBarisProduk(tmp[i], ++no);
            cout << BGREEN << "  [OK] Ditemukan " << no << " produk.\n" << RESET;
        }
        cout << BCYAN << "+----+----------------------+----------+---------------+-----+-------------+\n" << RESET;
    } catch (const invalid_argument& e) { cout << BRED << "  [!] " << e.what() << RESET << "\n"; }
    catch (const out_of_range& e)       { cout << BRED << "  [!] " << e.what() << RESET << "\n"; }
}

// ===================== MENU KELOLA PRODUK =====================

void menuKelolaLihatProduk() {
    string p;
    while (true) {
        cetakHeader("KELOLA TAMPILAN PRODUK", BCYAN);
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

void tampilKeranjang(ItemKeranjang keranjang[], int jmlKeranjang) {
    cetakHeader("KERANJANG BELANJA", BMAGENTA);
    cout << BMAGENTA << "+----+--------------------+-----+---------------+---------------+\n";
    cout << "| " << BYELLOW << "No " << BMAGENTA
         << "| " << BGREEN  << "Nama Produk        " << BMAGENTA
         << "| " << BCYAN   << "Qty " << BMAGENTA
         << "| " << BRED    << "Harga Satuan  " << BMAGENTA
         << "| " << BWHITE  << "Subtotal      " << BMAGENTA << "|\n";
    cout << "+----+--------------------+-----+---------------+---------------+\n" << RESET;
    double grandTotal = 0;
    for (int i = 0; i < jmlKeranjang; i++) {
        Produk &p = daftarProduk[keranjang[i].idxProduk];
        double sub = p.harga.satuan * keranjang[i].jumlah;
        grandTotal += sub;
        string nama = p.nama; if ((int)nama.size()>19) nama=nama.substr(0,16)+"...";
        cout << BMAGENTA << "| " << BYELLOW << left << setw(3) << (i+1)
             << BMAGENTA << "| " << BWHITE  << setw(19) << nama
             << BMAGENTA << "| " << BCYAN   << setw(4)  << keranjang[i].jumlah
             << BMAGENTA << "| " << BRED    << setw(14) << rupiahFormat(p.harga.satuan)
             << BMAGENTA << "| " << BGREEN  << setw(14) << rupiahFormat(sub)
             << BMAGENTA << "|\n" << RESET;
    }
    cout << BMAGENTA << "+----+--------------------+-----+---------------+---------------+\n" << RESET;
    cout << BOLD << BWHITE << "  TOTAL: " << BGREEN << rupiahFormat(grandTotal) << RESET << "\n";
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
    cout << BCYAN << "+----+----------------------+----------+---------------+-----+-------------+\n" << RESET;
    for (int i = 0; i < jml; i++) {
        peta[i] = idxAsli[i];
        string nama = tmp[i].nama; if ((int)nama.size()>21) nama=nama.substr(0,18)+"...";
        string merk2= tmp[i].merk; if ((int)merk2.size()>9) merk2=merk2.substr(0,7)+"..";
        string sc = (tmp[i].stok.jumlah<=10)?BRED:BGREEN;
        cout << BCYAN << "| " << BYELLOW << left << setw(3) << (i+1)
             << BCYAN << "| " << BWHITE  << setw(21) << nama
             << BCYAN << "| " << BMAGENTA<< setw(9)  << merk2
             << BCYAN << "| " << BRED    << setw(14) << rupiahFormat(tmp[i].harga.satuan)
             << BCYAN << "| " << sc      << setw(4)  << tmp[i].stok.jumlah
             << BCYAN << "| " << BBLUE   << setw(12) << tmp[i].jenis
             << BCYAN << "|\n" << RESET;
    }
    if (jml==0) cout << BCYAN << "|     " << BYELLOW << "(belum ada produk untuk merk ini)" << BCYAN << "                 |\n";
    cout << BCYAN << "+----+----------------------+----------+---------------+-----+-------------+\n" << RESET;
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
    } catch (const length_error& e)     { cout << BRED << "  [!] " << e.what() << RESET << "\n"; }
    catch (const invalid_argument& e)  { cout << BRED << "  [!] " << e.what() << RESET << "\n"; }
    catch (const out_of_range& e)      { cout << BRED << "  [!] " << e.what() << RESET << "\n"; }
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
            cout << BGREEN << "  Jumlah bayar (Rp): " << BYELLOW; getline(cin, ib); cout << RESET;
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
        cetakHeader("STRUK PEMBELIAN - GLOW UP STORE", BGREEN);
        cout << BCYAN << "  No. Transaksi : " << BWHITE << nt << "\n";
        cout << BCYAN << "  Waktu         : " << BWHITE << wt << "\n";
        cout << BCYAN << "  Pelanggan     : " << BMAGENTA << namaUser << "\n" << RESET;
        cetakGarisTipis(BGREEN);
        for (int i=0;i<jmlKeranjang;i++) {
            Produk &p=daftarProduk[keranjang[i].idxProduk];
            double sub=p.harga.satuan*keranjang[i].jumlah;
            string nama=p.nama; if((int)nama.size()>20) nama=nama.substr(0,17)+"...";
            cout << "  " << BWHITE << left << setw(20) << nama
                 << BYELLOW << " x" << setw(3) << keranjang[i].jumlah
                 << BGREEN  << setw(14) << rupiahFormat(sub) << "\n" << RESET;
        }
        cetakGarisTipis(BGREEN);
        cout << BOLD << BWHITE << "  TOTAL         : " << BGREEN << rupiahFormat(total) << "\n" << RESET;
        if (adaKembali) {
            cout << BCYAN << "  Dibayar       : " << BWHITE << rupiahFormat(bayar) << "\n";
            cout << BCYAN << "  Kembalian     : " << BYELLOW << rupiahFormat(kembali) << "\n" << RESET;
        }
        cetakGarisTipis(BGREEN);
        cout << BCYAN << "  Metode        : " << BMAGENTA << metode << "\n";
        cout << BCYAN << "  Status        : " << BGREEN << status << "\n" << RESET;
        cetakGaris(BGREEN);
        cout << BOLD << BMAGENTA << "  Makasih udah belanja! Semoga cocok :)\n" << RESET;
        cetakGaris(BGREEN);
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
        cout << BMAGENTA << "  [9]" << BWHITE << " Ganti urutan\n" << RESET;
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
        if (p=="9") {
            int sb=menuSortingPelanggan(); if (sb==-1) continue;
            modeSort=sb;
            if (!merkAktif.empty()) noProduk=tampilProdukMerk(merkAktif, peta, modeSort);
            continue;
        }
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
                else                          menuPelanggan(data[idx].nama);
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
        cout << BMAGENTA << "  [4]" << BWHITE << " Kelola Produk\n";
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

    daftarProduk[totalProduk++]={"Lightening Serum",       "Wardah",   {55000, "IDR"},{40,"pcs"},"Serum",      1};
    daftarProduk[totalProduk++]={"UV Shield Sunscreen",    "Wardah",   {48000, "IDR"},{35,"pcs"},"Sunscreen",  1};
    daftarProduk[totalProduk++]={"Hydrating Toner",        "Wardah",   {38000, "IDR"},{30,"pcs"},"Toner",      1};
    daftarProduk[totalProduk++]={"Face Wash Aloe",         "Wardah",   {30000, "IDR"},{25,"pcs"},"Face Wash",  1};
    daftarProduk[totalProduk++]={"Daily Moisturizer",      "Wardah",   {52000, "IDR"},{20,"pcs"},"Moisturizer",1};
    daftarProduk[totalProduk++]={"Brightening Serum",      "Glad2Glow",{62000, "IDR"},{25,"pcs"},"Serum",      1};
    daftarProduk[totalProduk++]={"Exfo Toner AHA BHA",     "Glad2Glow",{57000, "IDR"},{20,"pcs"},"Toner",      1};
    daftarProduk[totalProduk++]={"Moisturizer Gel",        "Glad2Glow",{45000, "IDR"},{30,"pcs"},"Moisturizer",1};
    daftarProduk[totalProduk++]={"Gentle Face Wash",       "Glad2Glow",{35000, "IDR"},{20,"pcs"},"Face Wash",  1};
    daftarProduk[totalProduk++]={"Hydra Moisturizer",      "Glad2Glow",{50000, "IDR"},{18,"pcs"},"Moisturizer",1};
    daftarProduk[totalProduk++]={"Sun Protection SPF30",   "Emina",    {40000, "IDR"},{50,"pcs"},"Sunscreen",  1};
    daftarProduk[totalProduk++]={"Bright Stuff Toner",     "Emina",    {35000, "IDR"},{45,"pcs"},"Toner",      1};
    daftarProduk[totalProduk++]={"Acne Care Serum",        "Emina",    {42000, "IDR"},{28,"pcs"},"Serum",      1};
    daftarProduk[totalProduk++]={"Bright Stuff Face Wash", "Emina",    {28000, "IDR"},{40,"pcs"},"Face Wash",  1};
    daftarProduk[totalProduk++]={"Ms Pimple Moisturizer",  "Emina",    {36000, "IDR"},{30,"pcs"},"Moisturizer",1};
    daftarProduk[totalProduk++]={"Hyalucera Moisturizer",  "Originote",{75000, "IDR"},{22,"pcs"},"Moisturizer",1};
    daftarProduk[totalProduk++]={"Ceramide Cleanser",      "Originote",{68000, "IDR"},{18,"pcs"},"Cleanser",   1};
    daftarProduk[totalProduk++]={"Niacinamide Serum",      "Originote",{72000, "IDR"},{20,"pcs"},"Serum",      1};
    daftarProduk[totalProduk++]={"Mild Cleanser Face Wash","Originote",{60000, "IDR"},{15,"pcs"},"Face Wash",  1};
    daftarProduk[totalProduk++]={"Barrier Moisturizer",    "Originote",{80000, "IDR"},{12,"pcs"},"Moisturizer",1};
    daftarProduk[totalProduk++]={"5x Ceramide Barrier",    "Skintific",{115000,"IDR"},{15,"pcs"},"Moisturizer",1};
    daftarProduk[totalProduk++]={"SymWhite Serum",         "Skintific",{125000,"IDR"},{12,"pcs"},"Serum",      1};
    daftarProduk[totalProduk++]={"Mugwort Clay Mask",      "Skintific",{98000, "IDR"},{18,"pcs"},"Mask",       1};
    daftarProduk[totalProduk++]={"Acne Facial Wash",       "Skintific",{95000, "IDR"},{14,"pcs"},"Face Wash",  1};
    daftarProduk[totalProduk++]={"Ceramide Moisturizer",   "Skintific",{120000,"IDR"},{10,"pcs"},"Moisturizer",1};

    // ===== SPLASH SCREEN =====
    cout << "\n";
    cetakGaris(BMAGENTA);
    cout << BG_MAGENTA << BOLD << BWHITE << "|     SELAMAT DATANG DI GLOW UP STORE!       |" << RESET << "\n";
    cout << BG_MAGENTA << BOLD << BWHITE << "|       Toko Skincare Terpercaya No.1        |" << RESET << "\n";
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
            cout << BG_MAGENTA << BOLD << BWHITE << "|   Terima kasih! Sampai jumpa di Glow Up!   |" << RESET << "\n";
            cetakGaris(BMAGENTA);
            berjalan=0;
        }
        else cout << BRED << "  [!] Pilihan tidak valid.\n" << RESET;
    }
    return 0;
}