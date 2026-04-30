#include <iostream>
#include <iomanip>

using namespace std;

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
    int    idxProduk;
    int    jumlah;
};

int MAKS_PENGGUNA = 10;
int MAKS_PRODUK   = 100;
int MAKS_KERANJANG = 50;

Pengguna dataPengguna[10];
Produk   daftarProduk[100];

int jumlahPengguna = 0;
int totalProduk    = 0;

void menuAdmin(string namaUser);
void menuPelanggan(string namaUser);
int  doLogin(Pengguna* data, int jml);

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

// ==================== TAMPILAN ====================

int tampilTabel(int stockOnly) {
    cout << "\n+----+----------------------+------------------+----------------+---------+--------------+\n";
    cout << "| No | Nama Produk          | Merk             | Harga          | Stok    | Jenis        |\n";
    cout << "+----+----------------------+------------------+----------------+---------+--------------+\n";

    int no = 0, adaData = 0;
    for (int i = 0; i < totalProduk; i++) {
        if (daftarProduk[i].aktif == 0) continue;
        if (stockOnly == 1 && daftarProduk[i].stok.jumlah == 0) continue;
        no++;
        adaData = 1;
        cout << "| " << left << setw(3) << no
             << "| " << setw(21) << daftarProduk[i].nama
             << "| " << setw(17) << daftarProduk[i].merk
             << "| " << setw(15) << rupiahFormat(daftarProduk[i].harga.satuan)
             << "| " << setw(8)  << daftarProduk[i].stok.jumlah
             << "| " << setw(13) << daftarProduk[i].jenis << "|\n";
    }
    if (adaData == 0)
        cout << "|                      (belum ada data produk)                                        |\n";
    cout << "+----+----------------------+------------------+----------------+---------+--------------+\n";
    return no;
}

// ==================== SORTING ====================

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

    if (mode == 1) bubbleSortNamaDesc(tmp, jml);
    else if (mode == 2) selectionSortHargaAsc(tmp, jml);
    else if (mode == 3) insertionSortStokAsc(tmp, jml);

    cout << "\n+----+----------------------+------------------+----------------+---------+--------------+\n";
    cout << "| No | Nama Produk          | Merk             | Harga          | Stok    | Jenis        |\n";
    cout << "+----+----------------------+------------------+----------------+---------+--------------+\n";
    for (int i = 0; i < jml; i++) {
        cout << "| " << left << setw(3) << (i + 1)
             << "| " << setw(21) << tmp[i].nama
             << "| " << setw(17) << tmp[i].merk
             << "| " << setw(15) << rupiahFormat(tmp[i].harga.satuan)
             << "| " << setw(8)  << tmp[i].stok.jumlah
             << "| " << setw(13) << tmp[i].jenis << "|\n";
    }
    cout << "+----+----------------------+------------------+----------------+---------+--------------+\n";
}

void menuSorting() {
    string pilihan;
    while (true) {
        cout << "\n--- Urutkan Produk ---\n";
        cout << "  [1] Nama (Z -> A)\n";
        cout << "  [2] Harga (Termurah -> Termahal)\n";
        cout << "  [3] Stok (Sedikit -> Terbanyak)\n";
        cout << "  [0] Kembali\n";
        cout << "Pilih: ";
        getline(cin, pilihan);

        if      (pilihan == "1") { cout << "\n=== Urut Nama Z -> A ==="; tampilDenganSorting(1); }
        else if (pilihan == "2") { cout << "\n=== Urut Harga Termurah -> Termahal ==="; tampilDenganSorting(2); }
        else if (pilihan == "3") { cout << "\n=== Urut Stok Sedikit -> Terbanyak ==="; tampilDenganSorting(3); }
        else if (pilihan == "0") return;
        else cout << "Pilihan gak valid!\n";
    }
}

// ==================== PILIH NOMOR ====================

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

// ==================== FITUR ADMIN ====================

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

void lihatProduk() {
    cout << "\n--- Daftar Produk Skincare ---";
    tampilTabel(0);
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
    cout << "\nYakin mau hapus '" << daftarProduk[idx].nama << "'? (y/n): ";
    getline(cin, konfirm);

    if (konfirm == "y" || konfirm == "Y")
        nonaktifkanProduk(&daftarProduk[idx].aktif, daftarProduk[idx].nama);
    else
        cout << "Penghapusan dibatalkan.\n";
}

// ==================== FITUR PELANGGAN ====================

void lihatKatalog() {
    cout << "\n--- Katalog Produk Skincare ---";
    tampilTabel(0);
}

void cariNama() {
    cout << "\n--- Cari Produk by Nama (Linear Search) ---\n";

    int jml = 0;
    for (int i = 0; i < totalProduk; i++)
        if (daftarProduk[i].aktif == 1) jml++;
    if (jml == 0) { cout << "Belum ada produk.\n"; return; }

    string keyword;
    cout << "Masukkan nama produk: ";
    getline(cin, keyword);

    for (int i = 0; i < (int)keyword.size(); i++)
        keyword[i] = tolower(keyword[i]);

    cout << "\n+----+----------------------+------------------+----------------+---------+--------------+\n";
    cout << "| No | Nama Produk          | Merk             | Harga          | Stok    | Jenis        |\n";
    cout << "+----+----------------------+------------------+----------------+---------+--------------+\n";

    int ketemu = 0;
    for (int i = 0; i < totalProduk; i++) {
        if (daftarProduk[i].aktif == 0) continue;
        string sekarang = daftarProduk[i].nama;
        for (int j = 0; j < (int)sekarang.size(); j++)
            sekarang[j] = tolower(sekarang[j]);

        if (sekarang.find(keyword) != string::npos) {
            ketemu++;
            cout << "| " << left << setw(3) << ketemu
                 << "| " << setw(21) << daftarProduk[i].nama
                 << "| " << setw(17) << daftarProduk[i].merk
                 << "| " << setw(15) << rupiahFormat(daftarProduk[i].harga.satuan)
                 << "| " << setw(8)  << daftarProduk[i].stok.jumlah
                 << "| " << setw(13) << daftarProduk[i].jenis << "|\n";
        }
    }
    if (ketemu == 0)
        cout << "|                 Produk yang dicari gak ketemu...                                    |\n";
    cout << "+----+----------------------+------------------+----------------+---------+--------------+\n";
    if (ketemu > 0) cout << "Ditemukan " << ketemu << " produk.\n";
}

void cariHarga() {
    cout << "\n--- Cari Produk by Harga (Selection Sort + Binary Search) ---\n";

    Produk tmp[100];
    int jml = 0;
    for (int i = 0; i < totalProduk; i++)
        if (daftarProduk[i].aktif == 1) tmp[jml++] = daftarProduk[i];

    if (jml == 0) { cout << "Belum ada produk.\n"; return; }

    // selection sort dulu biar bisa binary search
    selectionSortHargaAsc(tmp, jml);

    string inputHarga;
    cout << "Masukkan harga yang dicari (Rp): ";
    getline(cin, inputHarga);

    if (validAngka(inputHarga) == 0) { cout << "Harganya harus angka ya!\n"; return; }

    double target = bacaHarga(inputHarga);

    // binary search
    int low = 0, high = jml - 1, posisi = -1;
    while (low <= high) {
        int mid = low + (high - low) / 2;
        if (tmp[mid].harga.satuan == target) {
            posisi = mid;
            break;
        } else if (tmp[mid].harga.satuan < target) {
            low = mid + 1;
        } else {
            high = mid - 1;
        }
    }

    if (posisi == -1) {
        cout << "Produk dengan harga " << rupiahFormat(target) << " gak ada.\n";
        return;
    }

    cout << "\n+----+----------------------+------------------+----------------+---------+--------------+\n";
    cout << "| No | Nama Produk          | Merk             | Harga          | Stok    | Jenis        |\n";
    cout << "+----+----------------------+------------------+----------------+---------+--------------+\n";

    int no = 0;
    for (int i = 0; i < jml; i++) {
        if (tmp[i].harga.satuan == target) {
            no++;
            cout << "| " << left << setw(3) << no
                 << "| " << setw(21) << tmp[i].nama
                 << "| " << setw(17) << tmp[i].merk
                 << "| " << setw(15) << rupiahFormat(tmp[i].harga.satuan)
                 << "| " << setw(8)  << tmp[i].stok.jumlah
                 << "| " << setw(13) << tmp[i].jenis << "|\n";
        }
    }
    cout << "+----+----------------------+------------------+----------------+---------+--------------+\n";
    cout << "Ditemukan " << no << " produk dengan harga " << rupiahFormat(target) << ".\n";
}

void menuCari() {
    string pilihan;
    while (true) {
        cout << "\n--- Cari Produk ---\n";
        cout << "  [1] Cari by Nama\n";
        cout << "  [2] Cari by Harga\n";
        cout << "  [0] Kembali\n";
        cout << "Pilih: ";
        getline(cin, pilihan);

        if      (pilihan == "1") cariNama();
        else if (pilihan == "2") cariHarga();
        else if (pilihan == "0") return;
        else cout << "Pilihan gak valid!\n";
    }
}

// ==================== KERANJANG BELANJA ====================

void tampilKeranjang(ItemKeranjang keranjang[], int jmlKeranjang) {
    cout << "\n========================================\n";
    cout << "         KERANJANG BELANJA\n";
    cout << "========================================\n";
    cout << "+----+----------------------+----------+----------------+----------------+\n";
    cout << "| No | Nama Produk          | Qty      | Harga Satuan   | Subtotal       |\n";
    cout << "+----+----------------------+----------+----------------+----------------+\n";

    double grandTotal = 0;
    for (int i = 0; i < jmlKeranjang; i++) {
        Produk &p = daftarProduk[keranjang[i].idxProduk];
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

void menuKeranjang(string namaUser) {
    ItemKeranjang keranjang[50];
    int jmlKeranjang = 0;

    string pilihan;
    while (true) {
        cout << "\n--- Keranjang Belanja ---\n";
        cout << "  [1] Tambah Produk ke Keranjang\n";
        cout << "  [2] Lihat Keranjang\n";
        cout << "  [3] Hapus Produk dari Keranjang\n";
        cout << "  [4] Checkout\n";
        cout << "  [0] Kembali\n";
        cout << "Pilih: ";
        getline(cin, pilihan);

        if (pilihan == "1") {
            // ---- tambah ke keranjang ----
            int adaStok = 0;
            for (int i = 0; i < totalProduk; i++)
                if (daftarProduk[i].aktif == 1 && daftarProduk[i].stok.jumlah > 0) { adaStok = 1; break; }

            if (adaStok == 0) { cout << "Semua produk lagi habis nih...\n"; continue; }
            if (jmlKeranjang >= MAKS_KERANJANG) { cout << "Keranjang udah penuh!\n"; continue; }

            // tampil produk yang ada stok
            int peta[100], no = 0;
            cout << "\n+----+----------------------+------------------+----------------+---------+--------------+\n";
            cout << "| No | Nama Produk          | Merk             | Harga          | Stok    | Jenis        |\n";
            cout << "+----+----------------------+------------------+----------------+---------+--------------+\n";
            for (int i = 0; i < totalProduk; i++) {
                if (daftarProduk[i].aktif == 0 || daftarProduk[i].stok.jumlah == 0) continue;
                no++;
                peta[no - 1] = i;
                cout << "| " << left << setw(3) << no
                     << "| " << setw(21) << daftarProduk[i].nama
                     << "| " << setw(17) << daftarProduk[i].merk
                     << "| " << setw(15) << rupiahFormat(daftarProduk[i].harga.satuan)
                     << "| " << setw(8)  << daftarProduk[i].stok.jumlah
                     << "| " << setw(13) << daftarProduk[i].jenis << "|\n";
            }
            cout << "+----+----------------------+------------------+----------------+---------+--------------+\n";

            string pilihanProduk, pilihanJumlah;
            cout << "\nMau tambah produk nomor berapa? ";
            getline(cin, pilihanProduk);

            if (validAngka(pilihanProduk) == 0 || bacaAngka(pilihanProduk) < 1 || bacaAngka(pilihanProduk) > no) {
                cout << "Nomornya gak valid!\n"; continue;
            }

            int idx = peta[bacaAngka(pilihanProduk) - 1];
            Produk &p = daftarProduk[idx];

            // hitung stok yang sudah ada di keranjang
            int stokDiKeranjang = 0;
            for (int i = 0; i < jmlKeranjang; i++)
                if (keranjang[i].idxProduk == idx)
                    stokDiKeranjang += keranjang[i].jumlah;

            int stokTersedia = p.stok.jumlah - stokDiKeranjang;
            if (stokTersedia <= 0) {
                cout << "Stok produk ini udah habis dimasukin ke keranjang!\n"; continue;
            }

            cout << "Mau beli berapa " << p.nama << "? (stok tersedia: " << stokTersedia << "): ";
            getline(cin, pilihanJumlah);

            if (validAngka(pilihanJumlah) == 0 || bacaAngka(pilihanJumlah) <= 0) {
                cout << "Jumlahnya gak valid!\n"; continue;
            }

            int jml = bacaAngka(pilihanJumlah);
            if (jml > stokTersedia) {
                cout << "Jumlahnya melebihi stok yang tersedia (" << stokTersedia << ")!\n"; continue;
            }

            // cek kalau produk sudah ada di keranjang, update jumlahnya aja
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

        } else if (pilihan == "2") {
            // ---- lihat keranjang ----
            if (jmlKeranjang == 0) { cout << "Keranjang masih kosong nih...\n"; continue; }
            tampilKeranjang(keranjang, jmlKeranjang);

        } else if (pilihan == "3") {
            // ---- hapus dari keranjang ----
            if (jmlKeranjang == 0) { cout << "Keranjang masih kosong!\n"; continue; }

            tampilKeranjang(keranjang, jmlKeranjang);

            string hapusStr;
            cout << "\nHapus item nomor berapa? ";
            getline(cin, hapusStr);

            if (validAngka(hapusStr) == 0 || bacaAngka(hapusStr) < 1 || bacaAngka(hapusStr) > jmlKeranjang) {
                cout << "Nomornya gak valid!\n"; continue;
            }

            int idxHapus = bacaAngka(hapusStr) - 1;
            string namaHapus = daftarProduk[keranjang[idxHapus].idxProduk].nama;

            for (int i = idxHapus; i < jmlKeranjang - 1; i++)
                keranjang[i] = keranjang[i + 1];
            jmlKeranjang--;

            cout << "'" << namaHapus << "' berhasil dihapus dari keranjang.\n";

        } else if (pilihan == "4") {
            // ---- checkout ----
            if (jmlKeranjang == 0) { cout << "Keranjang masih kosong, belum bisa checkout!\n"; continue; }

            tampilKeranjang(keranjang, jmlKeranjang);

            string konfirm;
            cout << "\nLanjut bayar? (y/n): ";
            getline(cin, konfirm);

            if (konfirm != "y" && konfirm != "Y") {
                cout << "Checkout dibatalkan.\n"; continue;
            }

            // kurangi stok
            for (int i = 0; i < jmlKeranjang; i++)
                kurangiStok(&daftarProduk[keranjang[i].idxProduk].stok.jumlah, keranjang[i].jumlah);

            // hitung total
            double grandTotal = 0;
            for (int i = 0; i < jmlKeranjang; i++)
                grandTotal += daftarProduk[keranjang[i].idxProduk].harga.satuan * keranjang[i].jumlah;

            // struk
            cout << "\n\n";
            cout << "  ================================================\n";
            cout << "         STRUK PEMBELIAN - TOKO SKINCARE\n";
            cout << "  ================================================\n";
            cout << "  Nama Pelanggan : " << namaUser << "\n";
            cout << "  ------------------------------------------------\n";
            for (int i = 0; i < jmlKeranjang; i++) {
                Produk &p = daftarProduk[keranjang[i].idxProduk];
                double sub = p.harga.satuan * keranjang[i].jumlah;
                cout << "  " << left << setw(22) << p.nama
                     << "x" << setw(4) << keranjang[i].jumlah
                     << setw(15) << rupiahFormat(sub) << "\n";
            }
            cout << "  ------------------------------------------------\n";
            cout << "  TOTAL          : " << rupiahFormat(grandTotal) << "\n";
            cout << "  ================================================\n";
            cout << "  Makasih udah belanja! Semoga cocok ya :\n";
            cout << "  ================================================\n\n";

            // kosongkan keranjang
            jmlKeranjang = 0;

        } else if (pilihan == "0") {
            return;
        } else {
            cout << "Pilihan gak valid!\n";
        }
    }
}

// ==================== LOGIN & REGISTER ====================

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
            if (data[ketemu].role == "admin")
                menuAdmin(data[ketemu].nama);
            else
                menuPelanggan(data[ketemu].nama);
            return 1;
        }

        coba++;
        cout << "\nLogin gagal. Nama atau password salah.\n";
        if (coba < 3) cout << "Sisa percobaan: " << (3 - coba) << "\n";
    }

    cout << "\nTerlalu banyak percobaan. Program berhenti.\n";
    return 0;
}

// ==================== MENU UTAMA ====================

void menuAdmin(string namaUser) {
    string pilihan;
    while (true) {
        cout << "\n==================================================\n";
        cout << "          TOKO SKINCARE - ADMIN\n";
        cout << "==================================================\n";
        cout << "  Halo, " << namaUser << "!\n\n";
        cout << "  [1] Tambah Produk Baru\n";
        cout << "  [2] Lihat Semua Produk\n";
        cout << "  [3] Edit Data Produk\n";
        cout << "  [4] Hapus Produk\n";
        cout << "  [5] Urutkan Data Produk\n";
        cout << "  [6] Cari Produk\n";
        cout << "  [0] Logout\n";
        cout << "==================================================\n";
        cout << "  Mau ngapain? (0-6): ";
        getline(cin, pilihan);

        if      (pilihan == "1") tambahProduk();
        else if (pilihan == "2") lihatProduk();
        else if (pilihan == "3") editProduk();
        else if (pilihan == "4") hapusProduk();
        else if (pilihan == "5") menuSorting();
        else if (pilihan == "6") menuCari();
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
        cout << "  [1] Lihat Katalog Produk\n";
        cout << "  [2] Cari Produk\n";
        cout << "  [3] Keranjang Belanja\n";
        cout << "  [4] Urutkan Katalog\n";
        cout << "  [0] Keluar\n";
        cout << "==================================================\n";
        cout << "  Mau apa? (0-4): ";
        getline(cin, pilihan);

        if      (pilihan == "1") lihatKatalog();
        else if (pilihan == "2") menuCari();
        else if (pilihan == "3") menuKeranjang(namaUser);
        else if (pilihan == "4") menuSorting();
        else if (pilihan == "0") { cout << "Makasih udah mampir! Sampai jumpa lagi ya :)\n"; return; }
        else cout << "Pilihan gak valid...\n";
    }
}

int main() {
    // data awal pengguna
    dataPengguna[jumlahPengguna].nama     = "Admin";
    dataPengguna[jumlahPengguna].password = "admin123";
    dataPengguna[jumlahPengguna].role     = "admin";
    dataPengguna[jumlahPengguna].aktif    = 1;
    jumlahPengguna++;

    dataPengguna[jumlahPengguna].nama     = "Tamu";
    dataPengguna[jumlahPengguna].password = "000";
    dataPengguna[jumlahPengguna].role     = "pelanggan";
    dataPengguna[jumlahPengguna].aktif    = 1;
    jumlahPengguna++;

    // data awal produk
    daftarProduk[totalProduk] = {"Serum Vitamin C", "Some By Mi", {85000, "IDR"}, {30, "pcs"}, "Serum", 1}; totalProduk++;
    daftarProduk[totalProduk] = {"Toner Hydrating", "Somethinc", {72000, "IDR"}, {25, "pcs"}, "Toner", 1}; totalProduk++;
    daftarProduk[totalProduk] = {"Moisturizer SPF", "Wardah", {55000, "IDR"}, {40, "pcs"}, "Moisturizer", 1}; totalProduk++;
    daftarProduk[totalProduk] = {"Sunscreen SPF50", "Skin Aqua", {65000, "IDR"}, {50, "pcs"}, "Sunscreen", 1}; totalProduk++;
    daftarProduk[totalProduk] = {"Cleanser Gentle", "CeraVe", {120000, "IDR"}, {20, "pcs"}, "Cleanser", 1}; totalProduk++;

    cout << "\n=======================================================\n";
    cout << "|        Selamat Datang di Toko Skincare!            |\n";
    cout << "=======================================================\n";

    string pilihan;
    int berjalan = 1;
    while (berjalan) {
        cout << "\n=======================================================\n";
        cout << "|                  MENU UTAMA                        |\n";
        cout << "=======================================================\n";
        cout << "|  [1] Register  (Daftar Akun Baru)                  |\n";
        cout << "|  [2] Login     (Masuk ke Akun)                     |\n";
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