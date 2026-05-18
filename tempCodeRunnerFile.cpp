#include <iostream>
#include <iomanip>
#include <ctime>
#include <fstream>
#include <sstream>

using namespace std;

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

struct Harga {
    double satuan;
    string matauang;
};
struct Stok {
    int    jumlah;
    string satuan;
};
struct Produk {
    string nama;
    string merk;
    Harga  harga;
    Stok   stok;
    string jenis;
    int    aktif;
};
struct Pengguna {
    string nama;
    string password;
    string role;
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
    ofstream f(FILE_PENGGUNA.c_str());
    if (!f.is_open()) { cout << "[!] Gagal menyimpan data pengguna.\n"; return; }
    f << jumlahPengguna << "\n";
    for (int i = 0; i < jumlahPengguna; i++) {
        f << dataPengguna[i].nama     << "\n";
        f << dataPengguna[i].password << "\n";
        f << dataPengguna[i].role     << "\n";
        f << dataPengguna[i].aktif    << "\n";
    }
    f.close();
}

void loadPengguna() {
    ifstream f(FILE_PENGGUNA.c_str());
    if (!f.is_open()) return;

    int jml = 0;
    f >> jml;
    f.ignore();

    for (int i = 0; i < jml && jumlahPengguna < MAKS_PENGGUNA; i++) {
        string nama, password, role;
        int aktif = 1;

        getline(f, nama);
        getline(f, password);
        getline(f, role);
        f >> aktif;
        f.ignore();

        if (nama.empty() || password.empty() || role.empty()) continue;

        int duplikat = 0;
        for (int j = 0; j < jumlahPengguna; j++)
            if (dataPengguna[j].nama == nama) { duplikat = 1; break; }
        if (duplikat) continue;

        dataPengguna[jumlahPengguna].nama     = nama;
        dataPengguna[jumlahPengguna].password = password;
        dataPengguna[jumlahPengguna].role     = role;
        dataPengguna[jumlahPengguna].aktif    = aktif;
        jumlahPengguna++;
    }
    f.close();
}

// ===================== RIWAYAT LOGIN =====================

void simpanRiwayatLogin(string namaUser, string role, string status) {
    ofstream f(FILE_RIWAYAT_LOGIN.c_str(), ios::app);
    if (!f.is_open()) return;
    f << getWaktuSekarang() << "|" << namaUser << "|" << role << "|" << status << "\n";
    f.close();
}

void tampilRiwayatLogin() {
    ifstream f(FILE_RIWAYAT_LOGIN.c_str());
    if (!f.is_open()) { cout << "Belum ada riwayat login.\n"; return; }

    cout << "\n+---------------------+------------------+-------------+----------+\n";
    cout << "| Waktu               | Nama             | Role        | Status   |\n";
    cout << "+---------------------+------------------+-------------+----------+\n";

    string baris;
    int ada = 0;
    while (getline(f, baris)) {
        string kolom[4];
        int k = 0;
        string tmp = "";
        for (int i = 0; i < (int)baris.size(); i++) {
            if (baris[i] == '|' && k < 3) { kolom[k++] = tmp; tmp = ""; }
            else tmp += baris[i];
        }
        kolom[k] = tmp;
        cout << "| " << left << setw(20) << kolom[0]
             << "| " << setw(17) << kolom[1]
             << "| " << setw(12) << kolom[2]
             << "| " << setw(9)  << kolom[3] << "|\n";
        ada = 1;
    }
    if (!ada)
        cout << "|              (belum ada riwayat login)                    |\n";
    cout << "+---------------------+------------------+-------------+----------+\n";
    f.close();
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
    int n = s.size();
    string hasil = "";
    for (int i = 0; i < n; i++) {
        if (i > 0 && (n - i) % 3 == 0) hasil += ".";
        hasil += s[i];
    }
    return "Rp " + hasil;
}

int validAngka(string s) {
    if (s.size() == 0) return 0;
    for (int i = 0; i < (int)s.size(); i++)
        if (s[i] < '0' || s[i] > '9') return 0;
    return 1;
}

int bacaAngka(string s) {
    int hasil = 0;
    for (int i = 0; i < (int)s.size(); i++)
        hasil = hasil * 10 + (s[i] - '0');
    return hasil;
}

double bacaHarga(string s) {
    double h = 0;
    for (int i = 0; i < (int)s.size(); i++) h = h * 10 + (s[i] - '0');
    return h;
}

// ===================== TAMPILAN TABEL =====================

int tampilTabel(int stockOnly) {
    cout << "\n+-----+---------------------------+-------------+----------------+------+--------------+\n";
    cout << "| No  | Nama Produk               | Merk         | Harga          | Stok | Jenis        |\n";
    cout << "+-----+---------------------------+-------------+----------------+------+--------------+\n";

    int no = 0, adaData = 0;
    for (int i = 0; i < totalProduk; i++) {
        if (daftarProduk[i].aktif == 0) continue;
        if (stockOnly == 1 && daftarProduk[i].stok.jumlah == 0) continue;
        no++;
        adaData = 1;

        string nama = daftarProduk[i].nama;
        if ((int)nama.size() > 25) nama = nama.substr(0, 22) + "...";

        cout << "| " << left << setw(4) << no
             << "| " << setw(26) << nama
             << "| " << setw(12) << daftarProduk[i].merk
             << "| " << setw(15) << rupiahFormat(daftarProduk[i].harga.satuan)
             << "| " << setw(5)  << daftarProduk[i].stok.jumlah
             << "| " << setw(13) << daftarProduk[i].jenis << "|\n";
    }
    if (adaData == 0)
        cout << "|                        (belum ada data produk)                                   |\n";
    cout << "+-----+---------------------------+-------------+----------------+------+--------------+\n";
    return no;
}

// ===================== SORTING =====================

void selectionSortHargaAsc(Produk arr[], int n) {
    for (int i = 0; i < n - 1; i++) {
        int idxMin = i;
        for (int j = i + 1; j < n; j++)
            if (arr[j].harga.satuan < arr[idxMin].harga.satuan)
                idxMin = j;
        if (idxMin != i) {
            Produk tmp  = arr[i];
            arr[i]      = arr[idxMin];
            arr[idxMin] = tmp;
        }
    }
}

void bubbleSortNamaDesc(Produk arr[], int n) {
    for (int i = 0; i < n - 1; i++)
        for (int j = 0; j < n - i - 1; j++)
            if (arr[j].nama < arr[j + 1].nama) {
                Produk tmp = arr[j];
                arr[j]     = arr[j + 1];
                arr[j + 1] = tmp;
            }
}

void insertionSortStokAsc(Produk arr[], int n) {
    for (int i = 1; i < n; i++) {
        Produk kunci = arr[i];
        int j = i - 1;
        while (j >= 0 && arr[j].stok.jumlah > kunci.stok.jumlah) {
            arr[j + 1] = arr[j];
            j--;
        }
        arr[j + 1] = kunci;
    }
}

void tampilDenganSorting(int mode) {
    Produk tmp[100];
    int jml = 0;
    for (int i = 0; i < totalProduk; i++)
        if (daftarProduk[i].aktif == 1) tmp[jml++] = daftarProduk[i];

    if (jml == 0) { cout << "Belum ada data produk.\n"; return; }

    if      (mode == 1) bubbleSortNamaDesc(tmp, jml);
    else if (mode == 2) selectionSortHargaAsc(tmp, jml);
    else if (mode == 3) insertionSortStokAsc(tmp, jml);

    cout << "\n+-----+---------------------------+-------------+----------------+------+--------------+\n";
    cout << "| No  | Nama Produk               | Merk         | Harga          | Stok | Jenis        |\n";
    cout << "+-----+---------------------------+-------------+----------------+------+--------------+\n";
    for (int i = 0; i < jml; i++) {
        string nama = tmp[i].nama;
        if ((int)nama.size() > 25) nama = nama.substr(0, 22) + "...";
        cout << "| " << left << setw(4) << (i + 1)
             << "| " << setw(26) << nama
             << "| " << setw(12) << tmp[i].merk
             << "| " << setw(15) << rupiahFormat(tmp[i].harga.satuan)
             << "| " << setw(5)  << tmp[i].stok.jumlah
             << "| " << setw(13) << tmp[i].jenis << "|\n";
    }
    cout << "+-----+---------------------------+-------------+----------------+------+--------------+\n";
}

int pilihNomor(string aksi) {
    int peta[100], no = 0;
    for (int i = 0; i < totalProduk; i++)
        if (daftarProduk[i].aktif == 1) peta[no++] = i;

    tampilTabel(0);
    if (no == 0) { cout << "Belum ada produk nih...\n"; return -1; }

    string pilih;
    cout << "\nMau " << aksi << " produk nomor berapa? ";
    getline(cin, pilih);

    if (validAngka(pilih) == 0) { cout << "Nomornya gak valid!\n"; return -1; }
    int nomor = bacaAngka(pilih);
    if (nomor < 1 || nomor > no) { cout << "Nomornya di luar jangkauan!\n"; return -1; }

    return peta[nomor - 1];
}

// ===================== FITUR ADMIN =====================

void tambahProduk() {
    cout << "\n--- Tambah Produk Baru ---\n";
    if (totalProduk >= MAKS_PRODUK) { cout << "Data produk udah penuh nih!\n"; return; }

    Produk p;
    string hargaStr, stokStr;

    cout << "Nama produk : "; getline(cin, p.nama);
    cout << "Merk        : "; getline(cin, p.merk);
    cout << "Harga (Rp)  : "; getline(cin, hargaStr);
    cout << "Stok        : "; getline(cin, stokStr);
    cout << "Jenis       : "; getline(cin, p.jenis);

    if (validAngka(hargaStr) == 0 || validAngka(stokStr) == 0) {
        cout << "Harga sama stok harus angka ya!\n";
        return;
    }

    p.harga.satuan   = bacaHarga(hargaStr);
    p.harga.matauang = "IDR";
    p.stok.jumlah    = bacaAngka(stokStr);
    p.stok.satuan    = "pcs";
    p.aktif          = 1;

    daftarProduk[totalProduk++] = p;
    cout << "Produk '" << p.nama << "' udah ditambahin!\n";
}

void editProduk() {
    cout << "\n--- Edit Data Produk ---\n";
    int idx = pilihNomor("edit");
    if (idx == -1) return;

    Produk &p = daftarProduk[idx];
    cout << "\nEdit '" << p.nama << "' (langsung Enter = gak berubah)\n\n";

    string namaBaru, merkBaru, hargaBaru, stokBaru, jenisBaru;

    cout << "Nama baru  [" << p.nama  << "]: "; getline(cin, namaBaru);
    if (namaBaru.size() > 0) p.nama = namaBaru;

    cout << "Merk baru  [" << p.merk  << "]: "; getline(cin, merkBaru);
    if (merkBaru.size() > 0) p.merk = merkBaru;

    cout << "Harga baru [" << rupiahFormat(p.harga.satuan) << "]: "; getline(cin, hargaBaru);
    if (hargaBaru.size() > 0) {
        if (validAngka(hargaBaru) == 1) p.harga.satuan = bacaHarga(hargaBaru);
        else cout << "Harga gak valid, gak diubah.\n";
    }

    cout << "Stok baru  [" << p.stok.jumlah << "]: "; getline(cin, stokBaru);
    if (stokBaru.size() > 0) {
        if (validAngka(stokBaru) == 1) p.stok.jumlah = bacaAngka(stokBaru);
        else cout << "Stok gak valid, gak diubah.\n";
    }

    cout << "Jenis baru [" << p.jenis << "]: "; getline(cin, jenisBaru);
    if (jenisBaru.size() > 0) p.jenis = jenisBaru;

    cout << "Data berhasil diupdate!\n";
}

void nonaktifkanProduk(int *statusAktif, string namaProduk) {
    *statusAktif = 0;
    cout << "Produk '" << namaProduk << "' udah dihapus!\n";
}

void hapusProduk() {
    cout << "\n--- Hapus Produk ---\n";
    int idx = pilihNomor("hapus");
    if (idx == -1) return;

    string konfirm;
    while (true) {
        cout << "\nYakin mau hapus '" << daftarProduk[idx].nama << "'? (y/n): ";
        getline(cin, konfirm);
        if (konfirm == "y" || konfirm == "Y" || konfirm == "n" || konfirm == "N") break;
        cout << "Input gak valid! Masukkan y atau n aja ya.\n";
    }

    if (konfirm == "y" || konfirm == "Y")
        nonaktifkanProduk(&daftarProduk[idx].aktif, daftarProduk[idx].nama);
    else
        cout << "Penghapusan dibatalkan.\n";
}

// ===================== FITUR PELANGGAN =====================

void cariNama() {
    cout << "\n--- Cari Produk by Jenis ---\n";

    int jml = 0;
    for (int i = 0; i < totalProduk; i++)
        if (daftarProduk[i].aktif == 1) jml++;
    if (jml == 0) { cout << "Belum ada produk.\n"; return; }

    cout << "\n  Jenis yang tersedia:\n";
    cout << "  [1] Serum\n";
    cout << "  [2] Toner\n";
    cout << "  [3] Moisturizer\n";
    cout << "  [4] Face Wash\n";
    cout << "  [5] Sunscreen\n";
    cout << "  [6] Cleanser\n";
    cout << "  [7] Mask\n";
    cout << "  [8] Semua Jenis\n";
    cout << "\n  Pilih jenis: ";

    string pil;
    getline(cin, pil);

    string keyword = "";
    if      (pil == "1") keyword = "serum";
    else if (pil == "2") keyword = "toner";
    else if (pil == "3") keyword = "moisturizer";
    else if (pil == "4") keyword = "face wash";
    else if (pil == "5") keyword = "sunscreen";
    else if (pil == "6") keyword = "cleanser";
    else if (pil == "7") keyword = "mask";
    else if (pil == "8") keyword = "";
    else { cout << "Pilihan gak valid!\n"; return; }

    cout << "\n+-----+-------------------------+-------------+----------------+-------+--------------+\n";
    cout << "| No  | Nama Produk             | Merk        | Harga          | Stok  | Jenis        |\n";
    cout << "+-----+-------------------------+-------------+----------------+-------+--------------+\n";

    int ketemu = 0;
    for (int i = 0; i < totalProduk; i++) {
        if (daftarProduk[i].aktif == 0) continue;
        string jenisProduk = daftarProduk[i].jenis;
        for (int j = 0; j < (int)jenisProduk.size(); j++)
            jenisProduk[j] = tolower(jenisProduk[j]);
        if (keyword == "" || jenisProduk == keyword) {
            ketemu++;
            string nama = daftarProduk[i].nama;
            if ((int)nama.size() > 23) nama = nama.substr(0, 20) + "...";
            string merk = daftarProduk[i].merk;
            if ((int)merk.size() > 11) merk = merk.substr(0, 8) + "...";
            cout << "| " << left << setw(4) << ketemu
                 << "| " << setw(24) << nama
                 << "| " << setw(12) << merk
                 << "| " << setw(15) << rupiahFormat(daftarProduk[i].harga.satuan)
                 << "| " << setw(6)  << daftarProduk[i].stok.jumlah
                 << "| " << setw(13) << daftarProduk[i].jenis << "|\n";
        }
    }
    if (ketemu == 0)
        cout << "|           (tidak ada produk untuk jenis ini)                              |\n";
    cout << "+-----+-------------------------+-------------+----------------+-------+--------------+\n";
    if (ketemu > 0) cout << "Ditemukan " << ketemu << " produk.\n";
}

void cariHarga() {
    cout << "\n--- Cari Produk by Harga ---\n";

    Produk tmp[100];
    int jml = 0;
    for (int i = 0; i < totalProduk; i++)
        if (daftarProduk[i].aktif == 1) tmp[jml++] = daftarProduk[i];

    if (jml == 0) { cout << "Belum ada produk.\n"; return; }

    selectionSortHargaAsc(tmp, jml);

    string inputHarga;
    cout << "Masukkan harga yang dicari (Rp): ";
    getline(cin, inputHarga);

    if (validAngka(inputHarga) == 0) { cout << "Harganya harus angka ya!\n"; return; }

    double target = bacaHarga(inputHarga);

    int low = 0, high = jml - 1, posisi = -1;
    while (low <= high) {
        int mid = low + (high - low) / 2;
        if (tmp[mid].harga.satuan == target) { posisi = mid; break; }
        else if (tmp[mid].harga.satuan < target) low = mid + 1;
        else high = mid - 1;
    }

    if (posisi == -1) {
        cout << "Produk dengan harga " << rupiahFormat(target) << " gak ada.\n";

        double selisihMin = -1;
        double hargaTerdekat = 0;
        for (int i = 0; i < jml; i++) {
            double selisih = tmp[i].harga.satuan - target;
            if (selisih < 0) selisih = -selisih;
            if (selisihMin < 0 || selisih < selisihMin) {
                selisihMin = selisih;
                hargaTerdekat = tmp[i].harga.satuan;
            }
        }

        cout << "\nBerikut produk dengan harga terdekat (" << rupiahFormat(hargaTerdekat) << "):\n";
        cout << "\n+-----+-------------------------+------------------+----------------+-------+--------------+\n";
        cout << "| No  | Nama Produk             | Merk              | Harga          | Stok  | Jenis        |\n";
        cout << "+-----+-------------------------+------------------+----------------+-------+--------------+\n";
        int no = 0;
        for (int i = 0; i < jml; i++) {
            if (tmp[i].harga.satuan == hargaTerdekat) {
                no++;
                cout << "| " << left << setw(4) << no
                     << "| " << setw(24) << tmp[i].nama
                     << "| " << setw(17) << tmp[i].merk
                     << "| " << setw(15) << rupiahFormat(tmp[i].harga.satuan)
                     << "| " << setw(6)  << tmp[i].stok.jumlah
                     << "| " << setw(13) << tmp[i].jenis << "|\n";
            }
        }
        cout << "+-----+-------------------------+------------------+----------------+-------+--------------+\n";
        return;
    }

    cout << "\n+-----+-------------------------+------------------+----------------+-------+--------------+\n";
    cout << "| No  | Nama Produk             | Merk              | Harga          | Stok  | Jenis        |\n";
    cout << "+-----+-------------------------+------------------+----------------+-------+--------------+\n";
    int no = 0;
    for (int i = 0; i < jml; i++) {
        if (tmp[i].harga.satuan == target) {
            no++;
            cout << "| " << left << setw(4) << no
                 << "| " << setw(24) << tmp[i].nama
                 << "| " << setw(17) << tmp[i].merk
                 << "| " << setw(15) << rupiahFormat(tmp[i].harga.satuan)
                 << "| " << setw(6)  << tmp[i].stok.jumlah
                 << "| " << setw(13) << tmp[i].jenis << "|\n";
        }
    }
    cout << "+-----+-------------------------+------------------+----------------+-------+--------------+\n";
    cout << "Ditemukan " << no << " produk dengan harga " << rupiahFormat(target) << ".\n";
}

// ===================== MENU GABUNGAN: LIHAT, URUTKAN, CARI PRODUK =====================

void menuKelolaLihatProduk() {
    string pilihan;
    while (true) {
        cout << "\n==================================================\n";
        cout << "         KELOLA TAMPILAN PRODUK - ADMIN\n";
        cout << "==================================================\n";
        cout << "  --- Lihat Produk ---\n";
        cout << "  [1] Tampilkan Semua Produk (Default)\n";
        cout << "\n";
        cout << "  --- Urutkan Produk ---\n";
        cout << "  [2] Urut Nama Z -> A\n";
        cout << "  [3] Urut Harga Termurah -> Termahal\n";
        cout << "  [4] Urut Stok Sedikit -> Terbanyak\n";
        cout << "\n";
        cout << "  --- Cari Produk ---\n";
        cout << "  [5] Cari by Nama\n";
        cout << "  [6] Cari by Harga\n";
        cout << "\n";
        cout << "  [0] Kembali ke Menu Admin\n";
        cout << "==================================================\n";
        cout << "  Pilih: ";
        getline(cin, pilihan);

        if (pilihan == "1") {
            cout << "\n=== SEMUA PRODUK (Urutan Default) ===";
            tampilTabel(0);
        }
        else if (pilihan == "2") {
            cout << "\n=== Urut Nama Z -> A ===";
            tampilDenganSorting(1);
        }
        else if (pilihan == "3") {
            cout << "\n=== Urut Harga Termurah -> Termahal ===";
            tampilDenganSorting(2);
        }
        else if (pilihan == "4") {
            cout << "\n=== Urut Stok Sedikit -> Terbanyak ===";
            tampilDenganSorting(3);
        }
        else if (pilihan == "5") {
            cariNama();
        }
        else if (pilihan == "6") {
            cariHarga();
        }
        else if (pilihan == "0") {
            return;
        }
        else {
            cout << "Pilihan gak valid, coba lagi ya!\n";
        }
    }
}

// ===================== KERANJANG BELANJA =====================

void tampilKeranjang(ItemKeranjang keranjang[], int jmlKeranjang) {
    cout << "\n========================================\n";
    cout << "         KERANJANG BELANJA\n";
    cout << "========================================\n";
    cout << "+----+----------------------+----------+----------------+----------------+\n";
    cout << "| No | Nama Produk          | Qty      | Harga Satuan   | Subtotal       |\n";
    cout << "+----+----------------------+----------+----------------+----------------+\n";

    double grandTotal = 0;
    for (int i = 0; i < jmlKeranjang; i++) {
        Produk &p  = daftarProduk[keranjang[i].idxProduk];
        double sub = p.harga.satuan * keranjang[i].jumlah;
        grandTotal += sub;
        cout << "| " << left << setw(3) << (i + 1)
             << "| " << setw(21) << p.nama
             << "| " << setw(9)  << keranjang[i].jumlah
             << "| " << setw(15) << rupiahFormat(p.harga.satuan)
             << "| " << setw(15) << rupiahFormat(sub) << "|\n";
    }
    cout << "+----+----------------------+----------+----------------+----------------+\n";
    cout << "  TOTAL BELANJA : " << rupiahFormat(grandTotal) << "\n";
    cout << "========================================\n";
}

void kurangiStok(int *stokPtr, int jumlah) {
    *stokPtr -= jumlah;
}

// ===================== TAMPIL PRODUK PER MERK + SORTING =====================

int tampilProdukMerk(string merk, int peta[], int modeSort) {
    Produk tmp[100];
    int    idxAsli[100];
    int    jml = 0;

    for (int i = 0; i < totalProduk; i++) {
        if (daftarProduk[i].aktif == 0) continue;

        string merkData = daftarProduk[i].merk;
        string merkCari = merk;
        for (int j = 0; j < (int)merkData.size(); j++) merkData[j] = tolower(merkData[j]);
        for (int j = 0; j < (int)merkCari.size(); j++) merkCari[j] = tolower(merkCari[j]);
        if (merkData != merkCari) continue;

        tmp[jml]     = daftarProduk[i];
        idxAsli[jml] = i;
        jml++;
    }

    if      (modeSort == 1) bubbleSortNamaDesc(tmp, jml);
    else if (modeSort == 2) selectionSortHargaAsc(tmp, jml);
    else if (modeSort == 3) insertionSortStokAsc(tmp, jml);

    for (int i = 0; i < jml; i++) {
        for (int k = 0; k < totalProduk; k++) {
            if (daftarProduk[k].nama == tmp[i].nama &&
                daftarProduk[k].merk == tmp[i].merk &&
                daftarProduk[k].aktif == 1) {
                idxAsli[i] = k;
                break;
            }
        }
    }

    cout << "\n";
    cout << "  +-------------------------------------------------+\n";
    cout << "  |         KATALOG MERK: ";
    string judulMerk = merk;
    int pad = 25 - (int)judulMerk.size();
    cout << judulMerk;
    for (int i = 0; i < pad; i++) cout << " ";
    cout << "|\n";
    cout << "  +-------------------------------------------------+\n";

    if (modeSort == 1)
        cout << "  [ Diurutkan: Nama Z -> A ]\n";
    else if (modeSort == 2)
        cout << "  [ Diurutkan: Harga Termurah -> Termahal ]\n";
    else if (modeSort == 3)
        cout << "  [ Diurutkan: Stok Sedikit -> Terbanyak ]\n";
    else
        cout << "  [ Urutan: Default ]\n";

    cout << "\n";
    cout << "+-----+-------------------------+----------------+-------+--------------+\n";
    cout << "| No  | Nama Produk             | Harga          | Stok  | Jenis        |\n";
    cout << "+-----+-------------------------+----------------+-------+--------------+\n";

    for (int i = 0; i < jml; i++) {
        peta[i] = idxAsli[i];
        cout << "| " << left << setw(4) << (i + 1)
             << "| " << setw(24) << tmp[i].nama
             << "| " << setw(15) << rupiahFormat(tmp[i].harga.satuan)
             << "| " << setw(6)  << tmp[i].stok.jumlah
             << "| " << setw(13) << tmp[i].jenis << "|\n";
    }
    if (jml == 0)
        cout << "|         (belum ada produk untuk merk ini)                      |\n";
    cout << "+-----+-------------------------+----------------+-------+--------------+\n";

    return jml;
}

// ===================== MENU SORTING UNTUK PELANGGAN =====================

int menuSortingPelanggan() {
    string pilihan;
    while (true) {
        cout << "\n==================================================\n";
        cout << "     PILIH URUTAN TAMPILAN PRODUK\n";
        cout << "==================================================\n";
        cout << "  Produk di setiap merk akan ditampilkan\n";
        cout << "  sesuai urutan yang kamu pilih di bawah ini:\n\n";
        cout << "  [1] Nama        : Pilih merek & Keranjang\n";
        cout << "  [2] Harga       : Termurah -> Termahal\n";
        cout << "  [0] Kembali\n";
        cout << "==================================================\n";
        cout << "Pilih urutan: ";
        getline(cin, pilihan);

        if (pilihan == "1") {
            cout << "  -> Produk akan diurutkan: Nama Z -> A\n";
            return 1;
        } else if (pilihan == "2") {
            cout << "  -> Produk akan diurutkan: Harga Termurah -> Termahal\n";
            return 2;
        } else if (pilihan == "0") {
            return -1;
        } else {
            cout << "Pilihan gak valid! Coba lagi ya.\n";
        }
    }
}

void tambahKeKeranjangDariMerk(ItemKeranjang keranjang[], int &jmlKeranjang,
                                 int peta[], int no) {
    if (jmlKeranjang >= MAKS_KERANJANG) {
        cout << "Keranjang udah penuh!\n";
        return;
    }

    string pilihanProduk, pilihanJumlah;
    cout << "\nMau tambah produk nomor berapa? (0 = batal): ";
    getline(cin, pilihanProduk);

    if (pilihanProduk == "0") { cout << "Dibatalkan.\n"; return; }

    if (validAngka(pilihanProduk) == 0 || bacaAngka(pilihanProduk) < 1 || bacaAngka(pilihanProduk) > no) {
        cout << "Nomornya gak valid!\n";
        return;
    }

    int    idx = peta[bacaAngka(pilihanProduk) - 1];
    Produk &p  = daftarProduk[idx];

    int stokDiKeranjang = 0;
    for (int i = 0; i < jmlKeranjang; i++)
        if (keranjang[i].idxProduk == idx)
            stokDiKeranjang += keranjang[i].jumlah;

    int stokTersedia = p.stok.jumlah - stokDiKeranjang;
    if (stokTersedia <= 0) {
        cout << "Stok produk ini udah habis dimasukin ke keranjang!\n";
        return;
    }

    cout << "Mau beli berapa " << p.nama << "? (stok tersedia: " << stokTersedia << "): ";
    getline(cin, pilihanJumlah);

    if (validAngka(pilihanJumlah) == 0 || bacaAngka(pilihanJumlah) <= 0) {
        cout << "Jumlahnya gak valid!\n";
        return;
    }

    int jml = bacaAngka(pilihanJumlah);
    if (jml > stokTersedia) {
        cout << "Jumlahnya melebihi stok yang tersedia (" << stokTersedia << ")!\n";
        return;
    }

    int sudahAda = 0;
    for (int i = 0; i < jmlKeranjang; i++) {
        if (keranjang[i].idxProduk == idx) {
            keranjang[i].jumlah += jml;
            sudahAda = 1;
            break;
        }
    }
    if (sudahAda == 0) {
        keranjang[jmlKeranjang].idxProduk = idx;
        keranjang[jmlKeranjang].jumlah    = jml;
        jmlKeranjang++;
    }

    cout << p.nama << " x" << jml << " berhasil ditambahkan ke keranjang!\n";
}

void prosesCheckout(ItemKeranjang keranjang[], int &jmlKeranjang, string namaUser) {
    if (jmlKeranjang == 0) { cout << "Keranjang masih kosong, belum bisa checkout!\n"; return; }

    tampilKeranjang(keranjang, jmlKeranjang);

    string konfirm;
    while (true) {
        cout << "\nLanjut bayar? (y/n): ";
        getline(cin, konfirm);
        if (konfirm == "y" || konfirm == "Y" || konfirm == "n" || konfirm == "N") break;
        cout << "Input gak valid! Masukkan y atau n aja ya.\n";
    }
    if (konfirm != "y" && konfirm != "Y") { cout << "Checkout dibatalkan.\n"; return; }

    string metodePembayaran = "";
    string statusPembayaran = "";
    string pilihanMetode;
    double jumlahBayar  = 0;
    double kembalian    = 0;
    int    adaKembalian = 0;

    cout << "\n--- Pilih Metode Pembayaran ---\n";
    cout << "  [1] Tunai (Cash)\n";
    cout << "  [2] Transfer Bank\n";
    cout << "  [3] QRIS\n";
    cout << "  [4] Kartu Debit/Kredit\n";
    cout << "Pilih metode: ";
    getline(cin, pilihanMetode);

    double grandTotal = 0;
    for (int i = 0; i < jmlKeranjang; i++)
        grandTotal += daftarProduk[keranjang[i].idxProduk].harga.satuan * keranjang[i].jumlah;

    if (pilihanMetode == "1") {
        metodePembayaran = "Tunai (Cash)";
        string inputBayar;
        cout << "Jumlah uang yang dibayar (Rp): ";
        getline(cin, inputBayar);
        if (validAngka(inputBayar) == 0) { cout << "Jumlah bayar gak valid! Checkout dibatalkan.\n"; return; }
        jumlahBayar = bacaHarga(inputBayar);
        if (jumlahBayar < grandTotal) {
            cout << "Uang kurang " << rupiahFormat(grandTotal - jumlahBayar) << "! Pembayaran gagal.\n";
            cout << "Checkout dibatalkan.\n";
            return;
        }
        kembalian        = jumlahBayar - grandTotal;
        adaKembalian     = 1;
        statusPembayaran = "LUNAS";
    } else if (pilihanMetode == "2") {
        metodePembayaran = "Transfer Bank";
        statusPembayaran = "LUNAS (Transfer Dikonfirmasi)";
    } else if (pilihanMetode == "3") {
        metodePembayaran = "QRIS";
        statusPembayaran = "LUNAS (QRIS Berhasil)";
    } else if (pilihanMetode == "4") {
        metodePembayaran = "Kartu Debit/Kredit";
        statusPembayaran = "LUNAS (Kartu Approved)";
    } else {
        cout << "Metode pembayaran gak valid! Checkout dibatalkan.\n";
        return;
    }

    string waktuTransaksi = getWaktuSekarang();
    string noTransaksi    = generateNoTransaksi();

    for (int i = 0; i < jmlKeranjang; i++)
        kurangiStok(&daftarProduk[keranjang[i].idxProduk].stok.jumlah, keranjang[i].jumlah);

    cout << "\n\n";
    cout << "  ================================================\n";
    cout << "         STRUK PEMBELIAN - TOKO SKINCARE\n";
    cout << "  ================================================\n";
    cout << "  No. Transaksi  : " << noTransaksi    << "\n";
    cout << "  Waktu          : " << waktuTransaksi << "\n";
    cout << "  Nama Pelanggan : " << namaUser       << "\n";
    cout << "  ------------------------------------------------\n";
    for (int i = 0; i < jmlKeranjang; i++) {
        Produk &p  = daftarProduk[keranjang[i].idxProduk];
        double sub = p.harga.satuan * keranjang[i].jumlah;
        cout << "  " << left << setw(22) << p.nama
             << "x" << setw(4) << keranjang[i].jumlah
             << setw(15) << rupiahFormat(sub) << "\n";
    }
    cout << "  ------------------------------------------------\n";
    cout << "  TOTAL          : " << rupiahFormat(grandTotal) << "\n";
    if (adaKembalian == 1) {
        cout << "  Dibayar        : " << rupiahFormat(jumlahBayar) << "\n";
        cout << "  Kembalian      : " << rupiahFormat(kembalian)   << "\n";
    }
    cout << "  ------------------------------------------------\n";
    cout << "  Metode Bayar   : " << metodePembayaran << "\n";
    cout << "  Status         : " << statusPembayaran << "\n";
    cout << "  ================================================\n";
    cout << "  Makasih udah belanja! Semoga cocok ya :)\n";
    cout << "  ================================================\n\n";

    jmlKeranjang = 0;
}

// ===================== MENU MERK + KERANJANG =====================

void menuMerkDanKeranjang(string namaUser) {
    ItemKeranjang keranjang[50];
    int jmlKeranjang = 0;

    int modeSort = menuSortingPelanggan();
    if (modeSort == -1) return;

    string merkAktif = "";
    int peta[100];
    int noProduk = 0;

    string pilihan;

    while (true) {
        cout << "\n==================================================\n";
        cout << "         PILIH MERK SKINCARE & KERANJANG\n";
        cout << "==================================================\n";

        cout << "  Urutan aktif : ";
        if      (modeSort == 1) cout << "Tabel skincare\n";
        else if (modeSort == 2) cout << "Harga Termurah -> Termahal\n";
        else if (modeSort == 3) cout << "Stok Sedikit -> Terbanyak\n";
        else                    cout << "Default\n";

        if (merkAktif != "")
            cout << "  Merk aktif  : " << merkAktif
                 << " (" << noProduk << " produk)\n";

        cout << "  Isi keranjang: " << jmlKeranjang << " item\n\n";

        cout << "  --- Pilih Merk ---\n";
        cout << "  [1] Wardah\n";
        cout << "  [2] Glad2Glow\n";
        cout << "  [3] Emina\n";
        cout << "  [4] Originote\n";
        cout << "  [5] Skintific\n";

        cout << "  --- Keranjang ---\n";
        cout << "  [6] Lihat keranjang\n";
        cout << "  [7] Hapus item dari keranjang\n";
        cout << "  [8] Checkout\n";

        cout << "  --- Pengaturan ---\n";
        cout << "  [9] Ganti Urutan Tampilan\n";

        cout << "  [0] Kembali\n";
        cout << "==================================================\n";
        cout << "Pilih: ";
        getline(cin, pilihan);

        string merkBaru = "";

        if      (pilihan == "1") merkBaru = "Wardah";
        else if (pilihan == "2") merkBaru = "Glad2Glow";
        else if (pilihan == "3") merkBaru = "Emina";
        else if (pilihan == "4") merkBaru = "Originote";
        else if (pilihan == "5") merkBaru = "Skintific";

        if (merkBaru != "") {
            merkAktif = merkBaru;
            noProduk = tampilProdukMerk(merkAktif, peta, modeSort);

            if (noProduk > 0) {
                string tanya;
                while (true) {
                    cout << "\nMau langsung tambah ke keranjang? (y/n): ";
                    getline(cin, tanya);
                    if (tanya == "y" || tanya == "Y" || tanya == "n" || tanya == "N") break;
                    cout << "Input gak valid! Masukkan y atau n aja ya.\n";
                }

                if (tanya == "y" || tanya == "Y") {
                    tambahKeKeranjangDariMerk(keranjang, jmlKeranjang, peta, noProduk);
                }
            }
            continue;
        }

        if (pilihan == "6") {
            if (jmlKeranjang == 0) { cout << "Keranjang masih kosong nih...\n"; continue; }
            tampilKeranjang(keranjang, jmlKeranjang);
            continue;
        }

        if (pilihan == "7") {
            if (jmlKeranjang == 0) { cout << "Keranjang masih kosong!\n"; continue; }

            tampilKeranjang(keranjang, jmlKeranjang);

            string hapusStr;
            cout << "\nHapus item nomor berapa? ";
            getline(cin, hapusStr);

            if (validAngka(hapusStr) == 0 ||
                bacaAngka(hapusStr) < 1 ||
                bacaAngka(hapusStr) > jmlKeranjang) {
                cout << "Nomornya gak valid!\n";
                continue;
            }

            int idxHapus = bacaAngka(hapusStr) - 1;
            string namaHapus = daftarProduk[keranjang[idxHapus].idxProduk].nama;

            // ===== KONFIRMASI HAPUS ITEM KERANJANG =====
            string konfirmHapus;
            while (true) {
                cout << "Yakin mau hapus '" << namaHapus << "' dari keranjang? (y/n): ";
                getline(cin, konfirmHapus);
                if (konfirmHapus == "y" || konfirmHapus == "Y" ||
                    konfirmHapus == "n" || konfirmHapus == "N") break;
                cout << "Input gak valid! Masukkan y atau n aja ya.\n";
            }

            if (konfirmHapus == "n" || konfirmHapus == "N") {
                cout << "Penghapusan dibatalkan.\n";
                continue;
            }
            // ===========================================

            for (int i = idxHapus; i < jmlKeranjang - 1; i++)
                keranjang[i] = keranjang[i + 1];
            jmlKeranjang--;

            cout << "'" << namaHapus << "' berhasil dihapus dari keranjang.\n";
            continue;
        }

        if (pilihan == "8") {
            prosesCheckout(keranjang, jmlKeranjang, namaUser);
            continue;
        }

        if (pilihan == "9") {
            int sortBaru = menuSortingPelanggan();
            if (sortBaru == -1) { continue; }
            modeSort = sortBaru;
            if (merkAktif != "") {
                cout << "\nMemperbarui tampilan " << merkAktif << "...\n";
                noProduk = tampilProdukMerk(merkAktif, peta, modeSort);
            }
            continue;
        }

        if (pilihan == "0") return;

        cout << "Pilihan gak valid!\n";
    }
}

// ===================== LOGIN & REGISTER =====================

void doRegister() {
    if (jumlahPengguna >= MAKS_PENGGUNA) { cout << "Slot pengguna udah penuh nih!\n"; return; }

    cout << "\n=== DAFTAR AKUN BARU ===\n";

    string nama, password;
    cout << "Nama     : "; getline(cin, nama);

    for (int i = 0; i < jumlahPengguna; i++) {
        if (dataPengguna[i].nama == nama) {
            cout << "Nama itu udah dipake, coba nama lain ya!\n";
            return;
        }
    }

    cout << "Password : "; getline(cin, password);

    dataPengguna[jumlahPengguna].nama     = nama;
    dataPengguna[jumlahPengguna].password = password;
    dataPengguna[jumlahPengguna].role     = "pelanggan";
    dataPengguna[jumlahPengguna].aktif    = 1;
    jumlahPengguna++;

    simpanPengguna();
    cout << "\nAkun '" << nama << "' berhasil dibuat sebagai pelanggan!\n";
}

int doLogin(Pengguna* data, int jml) {
    int coba = 0;
    while (coba < 3) {
        cout << "\n=== LOGIN TOKO SKINCARE ===\n";

        string inputNama, inputPassword;
        cout << "Nama     : "; getline(cin, inputNama);
        cout << "Password : "; getline(cin, inputPassword);

        int ketemu = -1;
        for (int i = 0; i < jml; i++) {
            if (data[i].aktif == 1 &&
                data[i].nama     == inputNama &&
                data[i].password == inputPassword) {
                ketemu = i;
                break;
            }
        }

        if (ketemu != -1) {
            cout << "\nLogin berhasil! Hai " << data[ketemu].nama << " :)\n";
            simpanRiwayatLogin(data[ketemu].nama, data[ketemu].role, "BERHASIL");
            if (data[ketemu].role == "admin")
                menuAdmin(data[ketemu].nama);
            else
                menuPelanggan(data[ketemu].nama);
            return 1;
        }

        coba++;
        simpanRiwayatLogin(inputNama, "-", "GAGAL");
        cout << "\nLogin gagal. Nama atau password salah.\n";
        if (coba < 3) cout << "Sisa percobaan: " << (3 - coba) << "\n";
    }

    cout << "\nTerlalu banyak percobaan. Program berhenti.\n";
    return 0;
}

// ===================== MENU UTAMA =====================

void menuAdmin(string namaUser) {
    string pilihan;
    while (true) {
        cout << "\n==================================================\n";
        cout << "          TOKO SKINCARE - ADMIN\n";
        cout << "==================================================\n";
        cout << "  Halo, " << namaUser << "!\n\n";
        cout << "  [1] Tambah Produk Baru\n";
        cout << "  [2] Edit Data Produk\n";
        cout << "  [3] Hapus Produk\n";
        cout << "  [4] Kelola Produk\n";
        cout << "  [5] Riwayat Login\n";
        cout << "  [0] Logout\n";
        cout << "==================================================\n";
        cout << "  Mau ngapain? (0-5): ";
        getline(cin, pilihan);

        if      (pilihan == "1") tambahProduk();
        else if (pilihan == "2") editProduk();
        else if (pilihan == "3") hapusProduk();
        else if (pilihan == "4") menuKelolaLihatProduk();
        else if (pilihan == "5") { cout << "\n--- Riwayat Login ---\n"; tampilRiwayatLogin(); }
        else if (pilihan == "0") { cout << "Sampai jumpa! BYE BYE\n"; return; }
        else cout << "Pilihan gak valid, coba lagi dong...\n";
    }
}

void menuPelanggan(string namaUser) {
    string pilihan;
    while (true) {
        cout << "\n==================================================\n";
        cout << "        TOKO SKINCARE - PELANGGAN\n";
        cout << "==================================================\n";
        cout << "  Halo, " << namaUser << "!\n\n";
        cout << "  [1] Pilih Merk Skincare\n";
        cout << "  [2] Cari by Jenis\n";
        cout << "  [0] Keluar\n";
        cout << "==================================================\n";
        cout << "  Mau apa? (0-3): ";
        getline(cin, pilihan);

        if      (pilihan == "1") menuMerkDanKeranjang(namaUser);
        else if (pilihan == "2") cariNama();
        else if (pilihan == "0") { cout << "Makasih udah mampir! Sampai jumpa lagi ya :)\n"; return; }
        else cout << "Pilihan gak valid...\n";
    }
}

// ===================== MAIN =====================

int main() {
    loadPengguna();

    int adminAda = 0, tamuAda = 0;
    for (int i = 0; i < jumlahPengguna; i++) {
        if (dataPengguna[i].nama == "admin") adminAda = 1;
        if (dataPengguna[i].nama == "Tamu")  tamuAda  = 1;
    }
    if (adminAda == 0) {
        dataPengguna[jumlahPengguna++] = {"admin", "123",  "admin",     1};
    }
    if (tamuAda == 0) {
        dataPengguna[jumlahPengguna++] = {"Tamu",  "000",  "pelanggan", 1};
    }

    daftarProduk[totalProduk++] = {"Lightening Serum",       "Wardah",    {55000,  "IDR"}, {40, "pcs"}, "Serum",       1};
    daftarProduk[totalProduk++] = {"UV Shield Sunscreen",    "Wardah",    {48000,  "IDR"}, {35, "pcs"}, "Sunscreen",   1};
    daftarProduk[totalProduk++] = {"Hydrating Toner",        "Wardah",    {38000,  "IDR"}, {30, "pcs"}, "Toner",       1};
    daftarProduk[totalProduk++] = {"Face Wash Aloe",         "Wardah",    {30000,  "IDR"}, {25, "pcs"}, "Face Wash",   1};
    daftarProduk[totalProduk++] = {"Daily Moisturizer",      "Wardah",    {52000,  "IDR"}, {20, "pcs"}, "Moisturizer", 1};
    daftarProduk[totalProduk++] = {"Brightening Serum",      "Glad2Glow", {62000,  "IDR"}, {25, "pcs"}, "Serum",       1};
    daftarProduk[totalProduk++] = {"Exfo Toner AHA BHA",     "Glad2Glow", {57000,  "IDR"}, {20, "pcs"}, "Toner",       1};
    daftarProduk[totalProduk++] = {"Moisturizer Gel",        "Glad2Glow", {45000,  "IDR"}, {30, "pcs"}, "Moisturizer", 1};
    daftarProduk[totalProduk++] = {"Gentle Face Wash",       "Glad2Glow", {35000,  "IDR"}, {20, "pcs"}, "Face Wash",   1};
    daftarProduk[totalProduk++] = {"Hydra Moisturizer",      "Glad2Glow", {50000,  "IDR"}, {18, "pcs"}, "Moisturizer", 1};
    daftarProduk[totalProduk++] = {"Sun Protection SPF30",   "Emina",     {40000,  "IDR"}, {50, "pcs"}, "Sunscreen",   1};
    daftarProduk[totalProduk++] = {"Bright Stuff Toner",     "Emina",     {35000,  "IDR"}, {45, "pcs"}, "Toner",       1};
    daftarProduk[totalProduk++] = {"Acne Care Serum",        "Emina",     {42000,  "IDR"}, {28, "pcs"}, "Serum",       1};
    daftarProduk[totalProduk++] = {"Bright Stuff Face Wash", "Emina",     {28000,  "IDR"}, {40, "pcs"}, "Face Wash",   1};
    daftarProduk[totalProduk++] = {"Ms Pimple Moisturizer",  "Emina",     {36000,  "IDR"}, {30, "pcs"}, "Moisturizer", 1};
    daftarProduk[totalProduk++] = {"Hyalucera Moisturizer",  "Originote", {75000,  "IDR"}, {22, "pcs"}, "Moisturizer", 1};
    daftarProduk[totalProduk++] = {"Ceramide Cleanser",      "Originote", {68000,  "IDR"}, {18, "pcs"}, "Cleanser",    1};
    daftarProduk[totalProduk++] = {"Niacinamide Serum",      "Originote", {72000,  "IDR"}, {20, "pcs"}, "Serum",       1};
    daftarProduk[totalProduk++] = {"Mild Cleanser Face Wash","Originote", {60000,  "IDR"}, {15, "pcs"}, "Face Wash",   1};
    daftarProduk[totalProduk++] = {"Barrier Moisturizer",    "Originote", {80000,  "IDR"}, {12, "pcs"}, "Moisturizer", 1};
    daftarProduk[totalProduk++] = {"5x Ceramide Barrier",    "Skintific", {115000, "IDR"}, {15, "pcs"}, "Moisturizer", 1};
    daftarProduk[totalProduk++] = {"SymWhite Serum",         "Skintific", {125000, "IDR"}, {12, "pcs"}, "Serum",       1};
    daftarProduk[totalProduk++] = {"Mugwort Clay Mask",      "Skintific", {98000,  "IDR"}, {18, "pcs"}, "Mask",        1};
    daftarProduk[totalProduk++] = {"Acne Facial Wash",       "Skintific", {95000,  "IDR"}, {14, "pcs"}, "Face Wash",   1};
    daftarProduk[totalProduk++] = {"Ceramide Moisturizer",   "Skintific", {120000, "IDR"}, {10, "pcs"}, "Moisturizer", 1};

    cout << "\n=======================================================\n";
    cout << "|        Selamat Datang di Toko Skincare!            |\n";
    cout << "=======================================================\n";

    string pilihan;
    int berjalan = 1;
    while (berjalan) {
        cout << "\n=======================================================\n";
        cout << "|                  MENU UTAMA                        |\n";
        cout << "=======================================================\n";
        cout << "|  [1] Register                                      |\n";
        cout << "|  [2] Login                                         |\n";
        cout << "|  [0] Keluar                                        |\n";
        cout << "=======================================================\n";
        cout << "Pilih menu (0-2): ";
        getline(cin, pilihan);

        if      (pilihan == "1") doRegister();
        else if (pilihan == "2") {
            int hasil = doLogin(dataPengguna, jumlahPengguna);
            if (hasil == 0) berjalan = 0;
        }
        else if (pilihan == "0") {
            cout << "\nTerima kasih udah mampir ke Toko Skincare!\n\n";
            berjalan = 0;
        }
        else cout << "Pilihan tidak valid. Silakan pilih 0-2.\n";
    }

    return 0;
}