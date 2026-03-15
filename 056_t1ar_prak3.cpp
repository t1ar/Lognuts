/**
 * @file 056_t1ar_prak3.cpp
 * @brief Aplikasi manajemen bengkel mobil "Lognuts".
 *
 * Program ini memungkinkan pelanggan untuk melakukan booking servis, melihat antrian,
 * membatalkan servis, dan melihat riwayat servis. Selain itu, terdapat
 * menu admin untuk mengelola data montir dan menandai servis sebagai selesai.
 * Data disimpan secara persisten dalam file teks.
 */

#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <sstream>

// Definisi path database jika belum ada (untuk kompabilitas)
#ifndef DB_PATH
    #define DB_PATH "Lognuts_DB.dat"
#endif

#ifndef MONTIR_DB
    #define MONTIR_DB "Montir_DB.dat"
#endif

using namespace std;

const string border = "|"; // Karakter pemisah untuk data dalam file

// --- Classes ---

class Service; // Forward declaration untuk class Service

/**
 * @class Customer
 * @brief Merepresentasikan data seorang pelanggan.
 *
 * Menggunakan struktur data Doubly Linked List untuk menyimpan semua pelanggan.
 */
class Customer {
public:
    string name;            // Nama lengkap pelanggan
    int age = 0;            // Umur pelanggan
    char gender{};          // Jenis kelamin ('L' atau 'P')
    string phone;           // Nomor telepon
    string address;         // Alamat pelanggan
    Service* serviceHistory = nullptr; // Pointer ke riwayat servis (Linked List)
    Customer* next = nullptr; // Pointer ke pelanggan berikutnya dalam list
    Customer* prev = nullptr; // Pointer ke pelanggan sebelumnya dalam list
};

/**
 * @class Service
 * @brief Merepresentasikan satu unit servis mobil.
 *
 * Digunakan dalam beberapa struktur data:
 * - Linked list global untuk antrian (pending) dan servis selesai (completed).
 * - Linked list per pelanggan untuk riwayat servis (serviceHistory).
 */
class Service {
public:
    string carModel;        // Model mobil
    string carBrand;        // Merek mobil
    string issueDesc;       // Deskripsi masalah/kendala
    string nameMechanic;    // Nama montir yang menangani
    string entryDate;       // Tanggal servis masuk
    bool isCompleted = false; // Status apakah servis sudah selesai
    bool isCancelled = false; // Status apakah servis dibatalkan

    Customer* customerData = nullptr; // Pointer ke data pelanggan yang memiliki servis ini
    Service* nextGlobal = nullptr;   // Pointer untuk antrian global (pending/completed)
    Service* nextInHistory = nullptr;// Pointer untuk riwayat servis per pelanggan
};

// --- Stack Implementation for Undo Cancel ---

/**
 * @class CancelNode
 * @brief Node untuk stack yang menyimpan servis yang baru saja dibatalkan.
 */
class CancelNode {
public:
    Service* service;   // Pointer ke data servis yang dibatalkan
    CancelNode* next;   // Pointer ke node berikutnya di stack
};
CancelNode* cancelTop = nullptr; // Pointer ke puncak stack

/**
 * @brief Menambahkan servis yang dibatalkan ke puncak stack.
 * @param s Pointer ke objek Service yang akan di-push.
 */
void pushCancel(Service* s) {
    CancelNode* newNode = new CancelNode();
    newNode->service = s;
    newNode->next = cancelTop;
    cancelTop = newNode;
}

/**
 * @brief Mengambil dan menghapus servis dari puncak stack (untuk fitur undo).
 * @return Service* Pointer ke objek Service yang di-pop, atau nullptr jika stack kosong.
 */
Service* popCancel() {
    if (cancelTop == nullptr) return nullptr;
    CancelNode* temp = cancelTop;
    Service* s = temp->service;
    cancelTop = cancelTop->next;
    delete temp;
    return s;
}

// --- Global Variables ---
Customer* customerHead = nullptr; // Head dari Doubly Linked List Customer
Customer* customerTail = nullptr; // Tail dari Doubly Linked List Customer
Service* pendingHead = nullptr;   // Head dari Queue (Linked List) servis yang sedang antri
Service* pendingTail = nullptr;   // Tail dari Queue (Linked List) servis yang sedang antri
Service* completedHead = nullptr; // Head dari Linked List servis yang sudah selesai

string mechanics[50]; // Array untuk menyimpan nama-nama montir
int mechanicCount = 0; // Jumlah montir saat ini

// --- Utilities ---

/**
 * @brief Membersihkan layar konsol.
 * Bekerja untuk Windows (cls) dan sistem UNIX-like (clear).
 */
void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

/**
 * @brief Membersihkan buffer input (cin).
 * Mencegah error input loop tak terbatas saat input tidak valid.
 */
void cinClean() {
    cin.clear();
    cin.ignore(10000,'\n');
}

// --- Data Management ---

/**
 * @brief Memuat daftar nama montir dari file MONTIR_DB.
 * Jika file tidak ada, akan diisi dengan data default.
 */
void loadMechanics() {
    ifstream file(MONTIR_DB);
    mechanicCount = 0;
    if (file.is_open()) {
        string name;
        while (getline(file, name)) {
            if(!name.empty()) mechanics[mechanicCount++] = name;
        }
        file.close();
    }
    // Jika file kosong atau tidak ada, isi dengan data default
    if (mechanicCount == 0) {
        mechanics[0] = "Suby"; mechanics[1] = "Farhan";
        mechanics[2] = "Aldo"; mechanics[3] = "Dimas";
        mechanicCount = 4;
    }
}

/**
 * @brief Menyimpan daftar nama montir ke file MONTIR_DB.
 */
void saveMechanics() {
    ofstream file(MONTIR_DB);
    for (int i = 0; i < mechanicCount; i++) {
        file << mechanics[i] << endl;
    }
    file.close();
}

/**
 * @brief Menambahkan pelanggan baru ke Doubly Linked List.
 * @param newCust Pointer ke objek Customer baru.
 */
void addCustomer(Customer* newCust) {
    if (!customerHead) customerHead = customerTail = newCust;
    else {
        customerTail->next = newCust;
        newCust->prev = customerTail;
        customerTail = newCust;
    }
}

/**
 * @brief Menambahkan servis baru ke antrian (pending queue).
 * @param s Pointer ke objek Service yang akan ditambahkan.
 */
void enqueuePending(Service* s) {
    s->nextGlobal = nullptr;
    if (!pendingTail) pendingHead = pendingTail = s;
    else {
        pendingTail->nextGlobal = s;
        pendingTail = s;
    }
}

/**
 * @brief Menambahkan servis yang telah selesai ke daftar completed.
 * @param s Pointer ke objek Service yang telah selesai.
 */
void addCompleted(Service* s) {
    s->nextGlobal = completedHead;
    completedHead = s;
}

/**
 * @brief Mencari pelanggan berdasarkan nama.
 * @param name Nama pelanggan yang dicari.
 * @return Customer* Pointer ke objek Customer jika ditemukan, selain itu nullptr.
 */
Customer* findCustomer(const string& name) {
    Customer* temp = customerHead;
    while (temp) {
        if (temp->name == name) return temp;
        temp = temp->next;
    }
    return nullptr;
}

/**
 * @brief Menyimpan semua data pelanggan dan riwayat servisnya ke file DB_PATH.
 */
void saveData() {
    ofstream file(DB_PATH);
    Customer* current = customerHead;
    while (current) {
        file << "C" << border << current->name << border << current->age << border
             << current->gender << border << current->phone << border << current->address << endl;
        Service* s = current->serviceHistory;
        while (s) {
            file << "S," << s->carBrand << border << s->carModel << border
                 << s->issueDesc << border << s->nameMechanic << border
                 << s->entryDate << border << (s->isCompleted ? "1" : "0") << border
                 << (s->isCancelled ? "1" : "0") << endl;
            s = s->nextInHistory;
        }
        current = current->next;
    }
    file.close();
}

/**
 * @brief Memuat semua data dari file DB_PATH ke dalam memori (linked list).
 * Data yang dimuat akan langsung disortir ke antrian yang sesuai (pending, completed, cancel).
 */
void loadData() {
    ifstream file(DB_PATH);
    if (!file.is_open()) return;
    string line;
    Customer* lastCust = nullptr;
    while (getline(file, line)) {
        if (line.empty() || line.length() < 2) continue;
        char type = line[0];
        string data = line.substr(2);
        stringstream ss(data);
        string token;

        if (type == 'C') {
            auto c = new Customer();
            getline(ss, c->name, '|');
            getline(ss, token, '|');
            try { c->age = stoi(token); } catch(...) { c->age = 0; } // Pencegahan crash
            getline(ss, token, '|'); c->gender = token.empty() ? ' ' : token[0];
            getline(ss, c->phone, '|');
            getline(ss, c->address, '|');
            addCustomer(c);
            lastCust = c;
        } else if (type == 'S' && lastCust != nullptr) {
            auto* s = new Service();
            getline(ss, s->carBrand, '|');
            getline(ss, s->carModel, '|');
            getline(ss, s->issueDesc, '|');
            getline(ss, s->nameMechanic, '|');
            getline(ss, s->entryDate, '|');
            if (getline(ss, token, '|')) s->isCompleted = (token == "1");
            if (getline(ss, token, '|')) s->isCancelled = (token == "1");

            s->customerData = lastCust;
            s->nextInHistory = lastCust->serviceHistory;
            lastCust->serviceHistory = s;

            // Sortir servis ke antrian yang sesuai
            if(s->isCancelled) pushCancel(s);
            else if(s->isCompleted) addCompleted(s);
            else enqueuePending(s);
        }
    }
    file.close();
}

// --- Core Features ---

/**
 * @brief Menampilkan daftar montir dan meminta pengguna untuk memilih satu.
 * @return string Nama montir yang dipilih.
 */
string selectMechanic() {
    cout << "Pilih Montir:\n";
    for(int i=0; i<mechanicCount; i++) cout << i+1 << ". " << mechanics[i] << "\n";
    cout << "Pilihan: >";
    int pil; cin >> pil; cinClean();
    if(pil > 0 && pil <= mechanicCount) return mechanics[pil-1];
    return mechanics[0]; // Default jika pilihan tidak valid
}

/**
 * @brief Menampilkan semua servis yang ada di antrian (pending queue).
 * Juga menyorot posisi antrian untuk servis milik pelanggan yang sedang login.
 * @param cust Pointer ke objek Customer yang sedang login.
 */
void CustomerQueueInfo(Customer* cust) {
    clearScreen();
    cout << "====== All Services ======\n\n";
    Service* temp = pendingHead;
    int pos = 1;
    string userPosStr = "";

    while (temp) {
        cout << "Model Mobil: " << temp->carModel << "\n";
        cout << "Merek Mobil: " << temp->carBrand << "\n";
        cout << "Kendala: " << temp->issueDesc << "\n";
        cout << "Montir: " << temp->nameMechanic << "\n";
        cout << "Nama Pelanggan: " << temp->customerData->name << "\n";
        cout << "No Telp Pelanggan: " << temp->customerData->phone << "\n\n";

        if (temp->customerData == cust) {
            userPosStr += "Servis " + temp->carModel + " Anda berada di antrian ke-" + to_string(pos) + "\n";
        }
        pos++;
        temp = temp->nextGlobal;
    }
    if (userPosStr != "") cout << userPosStr;
    else cout << "Anda tidak memiliki antrian aktif.\n";

    cout << "\nPress any key to go back..."; cin.get();
}

/**
 * @brief Memproses booking servis baru untuk pelanggan.
 * Jika mobil yang sama sudah ada di antrian, pengguna bisa memilih untuk menambahkan kendala.
 * @param cust Pointer ke objek Customer yang melakukan booking.
 */
void BookingService(Customer* cust) {
    clearScreen();
    cout << "====== New Service ======\n";
    string model, brand, issue, date;
    cout << "Model Mobil: >"; getline(cin, model);
    cout << "Merek Mobil: >"; getline(cin, brand);
    cout << "Kendala: >"; getline(cin, issue);

    Service* temp = pendingHead;
    bool appended = false;
    int pos = 1;
    // Cek jika mobil yang sama sudah ada di antrian
    while(temp) {
        if(temp->customerData == cust && temp->carModel == model && temp->carBrand == brand) {
            cout << "\nMobil ini sudah dibengkel anda ingin menambahkan kendala ini kepada servis tersebut (yes/no): >";
            string ans; getline(cin, ans);
            if(ans == "yes" || ans == "y") {
                temp->issueDesc += " + " + issue; // Tambahkan deskripsi kendala
                cout << "\n*Kendala sudah diupdate, nomor antrian anda adalah: " << pos << "*\n";
                saveData();
                appended = true;
                break;
            }
        }
        pos++;
        temp = temp->nextGlobal;
    }

    // Jika tidak, buat servis baru
    if(!appended) {
        cout << "Tanggal Masuk Bengkel: >"; getline(cin, date);
        string mechanic = selectMechanic();

        Service* s = new Service();
        s->carModel = model; s->carBrand = brand; s->issueDesc = issue;
        s->entryDate = date; s->nameMechanic = mechanic; s->customerData = cust;
        s->isCompleted = false; s->isCancelled = false;

        s->nextInHistory = cust->serviceHistory;
        cust->serviceHistory = s;
        enqueuePending(s);
        saveData();

        int q = 0; Service* t = pendingHead;
        while(t) { q++; t = t->nextGlobal; }
        cout << "\n*Servis sudah tercatat, nomor antrian anda adalah: " << q << "*\n";
    }
    cout << "Press any key to go back..."; cin.get();
}

/**
 * @brief Membatalkan servis yang sedang dalam antrian.
 * Servis yang dibatalkan akan dipindahkan dari pending queue ke cancel stack.
 * @param cust Pointer ke objek Customer yang ingin membatalkan servis.
 */
void CancelService(Customer* cust) {
    clearScreen();
    cout << "====== Cancel Service ======\n";

    Service* arr[50]; // Array sementara untuk menampilkan pilihan servis
    int count = 0;
    Service* temp = pendingHead;
    // Kumpulkan semua servis milik customer yang sedang antri
    while(temp) {
        if(temp->customerData == cust) arr[count++] = temp;
        temp = temp->nextGlobal;
    }

    if(count == 0) {
        cout << "Anda tidak memiliki servis yang sedang mengantri.\nPress any key to go back...";
        cin.get(); return;
    }

    // Tampilkan daftar servis yang bisa dibatalkan
    for(int i=0; i<count; i++) {
        cout << i+1 << ". Servis Ke-" << i+1 << ":\n";
        cout << "Model Mobil: " << arr[i]->carModel << "\n";
        cout << "Merek Mobil: " << arr[i]->carBrand << "\n";
        cout << "Kendala: " << arr[i]->issueDesc << "\n";
        cout << "Montir: " << arr[i]->nameMechanic << "\n\n";
    }

    cout << "Pilih Mobil yang ingin dibatalkan: >";
    int pil; cin >> pil; cinClean();

    if(pil > 0 && pil <= count) {
        Service* target = arr[pil-1];

        // Hapus dari pending queue
        if(pendingHead == target) {
            pendingHead = target->nextGlobal;
            if(!pendingHead) pendingTail = nullptr;
        } else {
            Service* prev = pendingHead;
            while(prev->nextGlobal != target) prev = prev->nextGlobal;
            prev->nextGlobal = target->nextGlobal;
            if(pendingTail == target) pendingTail = prev;
        }
        target->nextGlobal = nullptr;

        target->isCancelled = true; // Tandai sebagai dibatalkan
        pushCancel(target); // Masukkan ke cancel stack
        saveData();
        cout << "\n*Servis " << target->carModel << " telah dibatalkan*\n";
    }
    cout << "Press any key to go back..."; cin.get();
}

/**
 * @brief Mengembalikan servis yang baru saja dibatalkan (Undo).
 * Mengambil servis dari cancel stack dan memasukkannya kembali ke pending queue.
 */
void UndoCancel() {
    clearScreen();
    cout << "====== Booking Kembali Service ======\n";
    Service* s = popCancel(); // Ambil dari stack

    if(!s) {
        cout << "Tidak ada servis yang baru dibatalkan.\nPress any key to go back...";
        cin.get(); return;
    }

    cout << "Model Mobil: " << s->carModel << "\n";
    cout << "Merek Mobil: " << s->carBrand << "\n";
    cout << "Kendala: " << s->issueDesc << "\n";
    cout << "Montir: " << s->nameMechanic << "\n\n";

    cout << "Apakah anda ingin membooking kembali servis ini? (yes/no): >";
    string ans; getline(cin, ans);

    if(ans == "yes" || ans == "y") {
        cout << "Apakah ingin di reschedule? input (-) jika tidak\n";
        cout << "Tanggal Lama: " << s->entryDate << "\n";
        cout << "Tanggal Baru: >";
        string newDate; getline(cin, newDate);
        if(newDate != "-") s->entryDate = newDate;

        s->isCancelled = false; // Kembalikan statusnya
        enqueuePending(s); // Masukkan kembali ke antrian
        saveData();
        cout << "\n*Servis " << s->carModel << " telah dibooking kembali*\n";
    } else {
        pushCancel(s); // Jika tidak, kembalikan ke stack
    }
    cout << "Press any key to go back..."; cin.get();
}

/**
 * @brief Menampilkan riwayat servis pelanggan yang sudah selesai.
 * @param cust Pointer ke objek Customer yang riwayatnya akan ditampilkan.
 */
void CustomerHistoryOnlyCompleted(Customer* cust) {
    clearScreen();
    cout << "====== Riwayat Servis ======\n\n";
    Service* s = cust->serviceHistory;
    bool found = false;
    while(s) {
        if(s->isCompleted) { // Hanya tampilkan yang sudah selesai
            found = true;
            cout << "Model Mobil: " << s->carModel << "\n";
            cout << "Merek Mobil: " << s->carBrand << "\n";
            cout << "Kendala: " << s->issueDesc << "\n";
            cout << "Montir: " << s->nameMechanic << "\n";
            cout << "Tanggal Masuk: " << s->entryDate << "\n--------------------\n";
        }
        s = s->nextInHistory;
    }
    if(!found) cout << "Belum ada riwayat servis yang selesai.\n";
    cout << "\nPress any key to go back..."; cin.get();
}

// --- Admin Features ---

/**
 * @brief Fitur admin untuk menandai servis sebagai selesai.
 * Memindahkan servis dari pending queue ke completed list.
 */
void AdminCompleteJob() {
    clearScreen();
    cout << "====== Jobs Done (Selesaikan Servis) ======\n";
    if (!pendingHead) {
        cout << "Tidak ada servis yang sedang mengantri.\nTekan enter untuk kembali...";
        cin.get(); return;
    }

    // Tampilkan antrian untuk memudahkan admin
    Service* temp = pendingHead;
    int idx = 1;
    while(temp) {
        cout << idx++ << ". " << temp->customerData->name << " - " << temp->carModel << " (" << temp->nameMechanic << ")\n";
        temp = temp->nextGlobal;
    }

    cout << "\nMasukkan Nama Pelanggan yang servisnya sudah selesai: >";
    string custName; getline(cin, custName);

    Service* prev = nullptr;
    Service* curr = pendingHead;
    // Cari servis berdasarkan nama pelanggan
    while(curr) {
        if(curr->customerData->name == custName) break;
        prev = curr;
        curr = curr->nextGlobal;
    }

    if(curr) {
        // Hapus dari pending queue
        if(prev) prev->nextGlobal = curr->nextGlobal;
        else pendingHead = curr->nextGlobal;
        if(curr == pendingTail) pendingTail = prev;

        curr->isCompleted = true; // Tandai selesai
        addCompleted(curr); // Pindahkan ke completed list
        saveData();
        cout << "\n*Servis untuk " << custName << " (" << curr->carModel << ") berhasil diselesaikan!*\n";
    } else {
        cout << "\nData servis pelanggan tidak ditemukan di antrian.\n";
    }
    cout << "Press any key to go back..."; cin.get();
}

/**
 * @brief Fitur admin untuk menambahkan montir baru.
 */
void AddNewMechanic() {
    clearScreen();
    cout << "====== New Mechanic ======\n";
    cout << "Masukkan Nama montir baru\nNama: >";
    string name; getline(cin, name);

    // Cek duplikasi
    for(int i=0; i<mechanicCount; i++) {
        if(mechanics[i] == name) {
            cout << "\n*" << name << " sudah terdaftar sebagai montir*\n";
            cout << "Press any key to go back..."; cin.get();
            return;
        }
    }

    if(mechanicCount < 50) {
        mechanics[mechanicCount++] = name;
        saveMechanics();
        cout << "\n*" << name << " telah ditambahkan ke daftar montir*\n";
    } else {
        cout << "\nKapasitas montir penuh!\n";
    }
    cout << "Press any key to go back..."; cin.get();
}

/**
 * @brief Menampilkan menu khusus untuk admin.
 */
void AdminMenu() {
    int pil = -1;
    do {
        clearScreen();
        cout << "====== Welcome to Lognuts (Admin) ======\nPilih menu!\n";
        cout << "1. Selesaikan Servis\n2. Montir Baru\n3. Keluar\nPilihan: >";
        cin >> pil; cinClean();

        if (pil == 1) { AdminCompleteJob(); }
        else if (pil == 2) { AddNewMechanic(); }

    } while (pil != 3);
}

/**
 * @brief Menampilkan menu untuk pelanggan.
 * @param cust Pointer ke objek Customer yang sedang login.
 */
void CustomerMenu(Customer* cust) {
    int pil = -1;
    do {
        clearScreen();
        cout << "====== Welcome " << cust->name << " ======\nPilih menu:\n";
        cout << "1. Antrian Anda\n2. Booking Servis\n3. Batalkan Servis\n4. Undo Pembatalan\n5. Riwayat Servis\n6. Keluar\nPilihan: >";
        cin >> pil; cinClean();

        switch (pil) {
            case 1: CustomerQueueInfo(cust); break;
            case 2: BookingService(cust); break;
            case 3: CancelService(cust); break;
            case 4: UndoCancel(); break;
            case 5: CustomerHistoryOnlyCompleted(cust); break;
            case 6: return;
        }
    } while (true);
}

/**
 * @brief Fungsi utama yang menjalankan aplikasi.
 * Mengatur alur login/register pelanggan dan admin.
 */
void StartApp() {
    while(true) {
        clearScreen();
        cout << "====== Welcome To Garasi Suby ======\n";
        cout << "Masukkan nama & Nomor Telepon\n";
        string name, phone;
        cout << "Nama: "; getline(cin, name);

        // Cek akses admin
        if (name == "adminacces8008") {
            AdminMenu();
            continue;
        }

        cout << "No Telp: >"; getline(cin, phone);

        Customer* cust = findCustomer(name);
        // Jika pelanggan baru, buat data baru
        if (!cust) {
            cust = new Customer();
            cust->name = name;
            cust->phone = phone;
            addCustomer(cust);
            saveData();
        }
        CustomerMenu(cust);
    }
}

/**
 * @brief Titik masuk utama program.
 * Memuat data awal dan memulai aplikasi.
 */
int main() {
    loadMechanics(); // Muat data montir
    loadData();      // Muat data pelanggan dan servis
    StartApp();      // Jalankan aplikasi
    return 0;
}
