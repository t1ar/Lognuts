#include <iostream>
#include <fstream>
#include <string>

//for cmake,just for the db in root folder
#ifndef DB_PATH
    #define DB_PATH "Lognuts_DB.dat"
#endif

using namespace std;
const string border = "|";

// --- Structs ---

/**
 * Struct untuk menyimpan data pelanggan
 * Menggunakan doubly-linked list untuk navigasi maju dan mundur
 */
struct Service;
/**
 * Struct Customer - Menyimpan data pelanggan
 * Terdiri dari:
 *  - name: Nama pelanggan
 *  - age: Umur pelanggan
 *  - gender: Jenis kelamin (L=Laki-laki, P=Perempuan)
 *  - phone: Nomor telepon pelanggan
 *  - address: Alamat pelanggan
 *  - serviceHistory: Pointer ke servis paling baru (untuk riwayat servis)
 *  - next: Pointer ke pelanggan berikutnya (doubly-linked list)
 *  - prev: Pointer ke pelanggan sebelumnya (doubly-linked list)
 */
struct Customer {
    string name;
    int age = 0;
    char gender{};
    string phone;
    string address;
    Service* serviceHistory = nullptr;
    Customer* next = nullptr;
    Customer* prev = nullptr;
};

/**
 * Struct Service - Menyimpan data servis yang dilakukan
 * Terdiri dari:
 *  - carModel: Model mobil yang diservis
 *  - carBrand: Merek mobil yang diservis
 *  - issueDesc: Deskripsi kendala/masalah mobil
 *  - nameMechanic: Nama montir yang menangani servis
 *  - customerData: Pointer ke data pelanggan yang melakukan servis
 *  - nextGlobal: Pointer ke servis berikutnya (linked list global)
 *  - nextInHistory: Pointer ke servis sebelumnya dalam riwayat (urutan terbalik)
 */
struct Service {
    string carModel;
    string carBrand;
    string issueDesc;
    string nameMechanic;
    Customer* customerData = nullptr;
    Service* nextGlobal = nullptr;
    Service* nextInHistory = nullptr;
};

// --- Global Variables ---

/** Pointer ke pelanggan pertama dalam doubly-linked list */
Customer* customerHead = nullptr;
/** Pointer ke pelanggan terakhir dalam doubly-linked list */
Customer* customerTail = nullptr;
/** Pointer ke servis pertama dalam linked list global */
Service* serviceHead = nullptr;

// --- Function Declarations ---
void MainMenu();
void ServiceMenu();
void AllCustomerData();
void IndividualCustomerData();
void AllShortService();
void NewService();
void MechanicHistory();
void saveData();
void loadData();

// --- Helper Functions ---

/**
 * Membersihkan input buffer setelah pembacaan input
 * Menghapus karakter sisa di dalam buffer input stream
 * Digunakan untuk menangani newline atau karakter lainnya yang tertinggal
 */
void cinClean() {
    cin.clear();
    cin.ignore(10000,'\n');
}

/**
 * Validasi apakah input merupakan integer yang valid
 * @param input - Referensi variabel integer yang akan divalidasi
 * @return true jika input valid, false jika input gagal
 */
bool isValidInt(int &input) {
    if (cin.fail()) {
        cout << "Input tidak valid! Input harus berupa angka." << endl;
        cinClean();
        return false;
    }
    return true;
}

/**
 * Validasi apakah input merupakan karakter yang valid
 * @param input - Referensi variabel karakter yang akan divalidasi
 * @return true jika input valid, false jika input gagal
 */
bool isValidChar(char &input) {
    if (cin.fail()) {
        cout << "Input tidak valid! Input harus berupa huruf." << endl;
        cinClean();
        return false;
    }
    return true;
}

/**
 * Menambahkan pelanggan baru ke dalam linked list pelanggan
 * Pelanggan ditambahkan di akhir list (tail)
 * @param newCust - Pointer ke struct Customer yang akan ditambahkan
 */
void addCustomer(Customer* newCust) {
    if (customerHead == nullptr) {
        customerHead = customerTail = newCust;
    } else {
        customerTail->next = newCust;
        newCust->prev = customerTail;
        customerTail = newCust;
    }
}

/**
 * Menambahkan servis baru ke dalam linked list servis global
 * Servis ditambahkan di akhir list
 * @param newServ - Pointer ke struct Service yang akan ditambahkan
 */
void addService(Service* newServ) {
    if (serviceHead == nullptr) {
        serviceHead = newServ;
    } else {
        Service* temp = serviceHead;
        while (temp->nextGlobal != nullptr) temp = temp->nextGlobal;
        temp->nextGlobal = newServ;
    }
}

/**
 * Mencari pelanggan berdasarkan nama dalam linked list
 * @param name - Nama pelanggan yang dicari (const reference ke string)
 * @return Pointer ke struct Customer jika ditemukan, nullptr jika tidak ditemukan
 */
Customer* findCustomer(const string& name) {
    Customer* temp = customerHead;
    while (temp != nullptr) {
        if (temp->name == name) return temp;
        temp = temp->next;
    }
    return nullptr;
}

// --- IO Data ---

/**
 * Menyimpan semua data pelanggan dan riwayat servis ke file
 * Format file: C|nama|umur|gender|telp|alamat untuk pelanggan
 *              S,brand|model|kendala|montir untuk servis
 * File disimpan dengan nama 'Lognuts_DB.dat'
 */
void saveData() {
    ofstream file("Lognuts_DB.dat");
    Customer* current = customerHead;
    while (current != nullptr) {
        // Format C: name,age,gender,phone,address
        file << "C" << border << current->name << border
        << current->age << border << current->gender
        << border << current->phone << border << current->address << endl;

        Service* s = current->serviceHistory;
        while (s != nullptr) {
            // Format S: brand,model,issue,mechanic
            file << "S," << s->carBrand << border << s->carModel << border
                 << s->issueDesc << border << s->nameMechanic << endl;
            s = s->nextInHistory;
        }
        current = current->next;
    }
    file.close();
}

/**
 * Memuat semua data pelanggan dan riwayat servis dari file
 * Membaca file 'Lognuts_DB.dat' dan rekonstruksi struktur data linked list
 * Jika file tidak ditemukan, fungsi akan return tanpa error
 */
void loadData() {
    ifstream file("Lognuts_DB.dat");
    if (!file.is_open()) return;
    string line;
    Customer* lastCust = nullptr;
    while (getline(file, line)) {
        if (line.empty()) continue;
        char type = line[0];
        string data = line.substr(2);

        if (type == 'C') {
            auto c = new Customer();
            size_t s1 = data.find(border);
            size_t s2 = data.find(border, s1 + 1);
            size_t s3 = data.find(border, s2 + 1);
            size_t s4 = data.find(border, s3 + 1);

            c->name = data.substr(0, s1);
            c->age = stoi(data.substr(s1 + 1, s2 - s1 - 1));
            c->gender = data[s2 + 1];
            c->phone = data.substr(s3 + 1, s4 - s3 - 1);
            c->address = data.substr(s4 + 1);
            addCustomer(c);
            lastCust = c;
        } else if (type == 'S' && lastCust != nullptr) {
            auto* s = new Service();
            size_t s1 = data.find(border);
            size_t s2 = data.find(border, s1 + 1);
            size_t s3 = data.find(border, s2 + 1);

            s->carBrand = data.substr(0, s1);
            s->carModel = data.substr(s1 + 1, s2 - s1 - 1);
            s->issueDesc = data.substr(s2 + 1, s3 - s2 - 1);
            s->nameMechanic = data.substr(s3 + 1);

            s->customerData = lastCust;
            s->nextInHistory = lastCust->serviceHistory;
            lastCust->serviceHistory = s;
            addService(s);
        }
    }
    file.close();
}

// --- Menus ---

/**
 * Menampilkan daftar semua servis yang telah dilakukan
 * Menampilkan informasi: merek mobil, model, kendala, montir, dan data pelanggan
 * Jika tidak ada data, menampilkan pesan bahwa belum ada data servis
 */
void AllShortService() {
    cout << "\n====== All Services ======\n" << endl;
    Service* temp = serviceHead;
    if (!temp) cout << "Belum ada data servis." << endl;
    while (temp != nullptr) {
        cout << "-----------------------" << endl;
        cout << "Merek Mobil: " << temp->carBrand << endl;
        cout << "Model Mobil: " << temp->carModel << endl;
        cout << "Kendala: " << temp->issueDesc << endl;
        cout << "Montir: " << temp->nameMechanic << endl;
        cout << "Pelanggan: " << temp->customerData->name << " (" << temp->customerData->phone << ")" << endl;
        cout << "-----------------------\n" << endl;
        temp = temp->nextGlobal;
    }
    cout << "Tekan Enter untuk kembali...";
    cin.get(); cinClean(); return;
}

/**
 * Menu untuk menambahkan servis baru ke dalam sistem
 * Meminta data pelanggan (jika pelanggan baru), kemudian data servis
 * Jika pelanggan sudah ada, langsung menambahkan data servis
 * Data disimpan otomatis ke file setelah berhasil ditambahkan
 */
void NewService() {
    cout << "\n====== New Service ======" << endl;
    string name;
    cout << "Nama Pelanggan: "; getline(cin, name);
    Customer* c = findCustomer(name);

    if (c == nullptr) {
        c = new Customer();
        c->name = name;
        cout << "No Telp: "; getline(cin, c->phone);
        cout << "Alamat: "; getline(cin, c->address);
        while (c->age <= 0) {
            cout << "Umur: "; cin >> c->age;
            if (!isValidInt(c->age)) {
                c->age = -1;
            } else if (c->age <= 0) {
                cout << "Input tidak valid! Umur harus lebih dari 0." << endl;
            }
        }
        while (c->gender != 'L' && c->gender != 'P') {
            cout << "Gender (L/P): "; cin >> c->gender;
            c->gender = char(toupper(c->gender));
            if (!isValidChar(c->gender)) {
                continue;
            } else if (c->gender != 'L' && c->gender != 'P') {
                cout << "Input tidak valid! Masukkan 'L' untuk Laki-laki atau 'P' untuk Perempuan." << endl;
            }
        }
        cin.ignore();
        addCustomer(c);
        cout << "*Pelanggan baru terdaftar*" << endl;
    } else {
        cout << "*Data pelanggan ditemukan*" << endl;
    }

    auto* s = new Service();
    cout << "Merek Mobil: "; getline(cin, s->carBrand);
    cout << "Model Mobil: "; getline(cin, s->carModel);
    cout << "Kendala: "; getline(cin, s->issueDesc);
    cout << "Montir (Suby/Farhan/Dimas/Aldo): "; getline(cin, s->nameMechanic);

    s->customerData = c;
    s->nextInHistory = c->serviceHistory;
    c->serviceHistory = s;
    addService(s);
    saveData();
    cout << "\nServis berhasil disimpan!" << endl;
}

/**
 * Menampilkan riwayat pekerjaan servis untuk montir tertentu
 * Meminta user memilih montir (Suby, Farhan, Dimas, Aldo)
 * Menampilkan semua servis yang telah dikerjakan oleh montir yang dipilih
 */
void MechanicHistory() {
    int index;
    cout << "\nPilih Montir:\n(1)Suby (2)Farhan (3)Dimas (4)Aldo\nInput: ";
    cin >> index; if (!isValidInt(index)) return; // Validasi input
    if (index >= 1 && index <= 4) {
        string mechanics[] = {"Suby", "Farhan", "Dimas", "Aldo"};
        string target = mechanics[index - 1];
        Service* temp = serviceHead;
        bool found = false;
        cout << "======" << temp->nameMechanic << "'s Jobs" << "====== " << endl;
        while (temp != nullptr) {
            if (temp->nameMechanic == target) {
                cout << "-----------------------" << endl;
                cout << "Merek Mobil: " << temp->carBrand << endl;
                cout << "Model Mobil: " << temp->carModel << endl;
                cout << "Kendala: " << temp->issueDesc << endl;
                cout << "Pelanggan: " << temp->customerData->name << endl;
                cout << "No Telepon Pelanggan: " << temp->customerData->phone << endl;
                cout << "-----------------------" << endl;
                found = true;
            }
            temp = temp->nextGlobal;
        }
        if (!found) cout << "Montir ini belum memiliki riwayat kerja." << endl;
    } else cout << "Pilihan tidak valid." << endl;
    cout << "\nTekan Enter untuk kembali...";
    cin.get(); cinClean();
}

/**
 * Menu utama untuk bagian Servis
 * Menyediakan opsi: Lihat semua servis, Tambah servis baru, Riwayat montir
 * Loop sampai user memilih untuk kembali (pilihan 0)
 */
void ServiceMenu() {
    int pil = -1;
    do {
        cout << "\n====== Services ======\n1. Semua Servis\n2. Servis Baru\n3. Riwayat Montir\n0. Kembali\nPilihan: ";
        cin >> pil;
        if (!isValidInt(pil)) pil=-1; // Validasi input
        switch (pil) {
            case 1: AllShortService(); break;
            case 2: NewService(); break;
            case 3: MechanicHistory(); break;
            default: cout << "Pilihan Salah!";
        }
    } while (pil != 0);
}

/**
 * Menampilkan data lengkap untuk semua pelanggan dalam sistem
 * Untuk setiap pelanggan, menampilkan: nama, telp, alamat, dan servis terakhir
 * Jika tidak ada data pelanggan, menampilkan pesan data kosong
 */
void AllCustomerData() {
    Customer* temp = customerHead;
    cout << "====== All Customers ======" << endl;
    if (!temp) cout << "Data kosong." << endl;
    while (temp != nullptr) {
        cout << "-----------------------" << endl;
        cout << "Nama: " << temp->name << endl;
        cout << "Telp: " << temp->phone << endl;
        cout << "Alamat: " << temp->address << endl;
        cout << "___Servis Terakhir___" << endl;
        if (temp->serviceHistory) {
            cout << "Mobil: " << temp->serviceHistory->carBrand
            << " " << temp->serviceHistory->carModel << endl;
            cout << "Kendala: " << temp->serviceHistory->issueDesc << endl;
        }
        cout << "-----------------------" << endl;
        temp = temp->next;
    }
    cout << "\nTekan Enter..."; cin.get(); cinClean(); return;
}

/**
 * Menu untuk navigasi dan melihat data pelanggan secara individual
 * Menampilkan data detail pelanggan: nama, telp, umur, gender, alamat, dan 3 servis terakhir
 * Memungkinkan navigasi dengan tombol N (Next), P (Previous), E (Exit)
 * Jika tidak ada pelanggan, menampilkan pesan data kosong
 */
void IndividualCustomerData() {
    if (!customerHead) { cout << "Data kosong."; return; }
    Customer* curr = customerHead;
    char nav;
    do {
        cout << "====== Customer Data ======" << endl;
        cout << "Nama: " << curr->name << endl;
        cout << "Nomor Telepon: " << curr->phone << endl;
        cout << "Umur: " << curr->age << endl;
        cout << "Gender: ";
        if (curr->gender == 'L') cout << "Laki-laki" << endl;
        else if (curr->gender == 'P') cout << "Perempuan" << endl;
        else cout << "Tidak diketahui" << endl;
        cout << "Alamat: " << curr->address << endl;
        Service* s = curr->serviceHistory;
        cout << "\n3 Servis Terakhir" << endl;
        cout << "-----------------------" << endl;
        if (s == nullptr) {
            cout << "(Belum ada riwayat servis)" << endl;
        } else {
            int count = 0;
            while (s != nullptr && count < 3) {
                cout << "Mobil: " << s->carBrand << " " << s->carModel << endl;
                cout << "Kendala: " << s->issueDesc << endl;
                cout << "Montir: " << s->nameMechanic << endl;
                cout << "-----------------------" << endl;
                s = s->nextInHistory;
                count++;
            }
        }
        // 3. Navigasi
        cout << "\n[N]ext, [P]revious, [E]xit" << endl;
        cout << "Pilihan: ";
        cin >> nav;
        nav = char(toupper(nav));
        if (!isValidChar(nav)) continue; // Validasi input
        if (nav == 'N' && curr->next) curr = curr->next;
        else if ((nav == 'P') && curr->prev) curr = curr->prev;
        else if (nav != 'E') {
            cout << "[!] Pilihan tidak tersedia atau sudah di ujung data." << endl;
            cout << "\nTekan Enter untuk kembali...";cinClean(); cin.get(); 
        }
    } while (nav != 'E' && nav != 'e');
}

/**
 * Menu utama aplikasi Lognuts
 * Menyediakan opsi: Servis, Semua Pelanggan, Navigasi Pelanggan, Keluar
 * Loop sampai user memilih untuk keluar (pilihan 0)
 * Menampilkan pesan "Bye!" saat keluar
 */
void MainMenu() {
    int pil = -1;
    do {
        cout << "\n====== Lognuts ======\n" << endl;
        cout << "Pilih opsi: " << endl;
        cout << "1. Servis" << endl;
        cout << "2. Semua Pelanggan" << endl;
        cout << "3. Navigasi Pelanggan" << endl;
        cout << "0. Keluar\n" << endl;
        cout << "Pilihan: ";
        cin >> pil;
        if (!isValidInt(pil)) pil=-1; // Validasi input
        switch (pil) {
            case 1: ServiceMenu(); break;
            case 2: AllCustomerData(); break;
            case 3: IndividualCustomerData(); break;
            case 0: cout << "Bye!"; break;
            default: cout << "Pilihan salah!";
        }
    } while (pil != 0);
}

/**
 * Fungsi utama program
 * Memuat data dari file saat program dimulai
 * Menampilkan menu utama untuk interaksi dengan pengguna
 * @return 0 jika program berakhir normal
 */
int main() {
    loadData();
    MainMenu();
    return 0;
}