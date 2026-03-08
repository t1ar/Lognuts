#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib> // Untuk fungsi system() clear terminal
#include <sstream> // Untuk stringstream di loadData()

/*
    Daftar montir yang tersedia saat ini:
    1. Suby
    2. Farhan
    3. Dimas
    4. Aldo
    5. Rafi
*/

#ifndef DB_PATH
    #define DB_PATH "Lognuts_DB.dat"
#endif

using namespace std;


//Karakter pemisah data saat proses simpan/load file.
//Digunakan untuk memudahkan parsing isi file database.
const string border = "|";

// --- Classes (Pengganti Struct) ---

class Service;

/*
    Class Customer
    --------------
    Menyimpan informasi pelanggan bengkel.
    Data pelanggan disusun dalam doubly linked list,
    serta memiliki riwayat servis dalam bentuk linked list.
*/
class Customer {
public:
    string name;               // Nama pelanggan
    int age = 0;               // Umur pelanggan
    char gender{};             // Jenis kelamin pelanggan (L/P)
    string phone;              // Nomor telepon pelanggan
    string address;            // Alamat pelanggan
    Service* serviceHistory = nullptr; // Pointer ke riwayat servis pelanggan
    Customer* next = nullptr;          // Pointer ke pelanggan berikutnya
    Customer* prev = nullptr;          // Pointer ke pelanggan sebelumnya
};

/*
    Class Service
    -------------
    Menyimpan data terkait satu layanan servis kendaraan.
    Data servis bisa terhubung ke:
    - pelanggan pemilik servis
    - antrian global pending/completed
    - riwayat servis milik customer
*/
class Service {
public:
    string carModel;           // Model mobil
    string carBrand;           // Merek mobil
    string issueDesc;          // Deskripsi kendala kendaraan
    string nameMechanic;       // Nama montir yang menangani
    string entryDate;          // Tanggal masuk servis
    bool isCompleted = false;  // Status servis: selesai / belum

    Customer* customerData = nullptr;  // Pointer ke data pelanggan
    Service* nextGlobal = nullptr;     // Pointer ke node berikutnya dalam antrian global
    Service* nextInHistory = nullptr;  // Pointer ke node berikutnya dalam riwayat customer
};

// --- Global Variables ---

/*
    Head dan tail untuk linked list pelanggan.
*/
Customer* customerHead = nullptr;
Customer* customerTail = nullptr;

/*
    Head dan tail untuk antrian servis yang belum selesai.
*/
Service* pendingHead = nullptr;
Service* pendingTail = nullptr;

/*
    Head untuk linked list servis yang sudah selesai.
*/
Service* completedHead = nullptr;

// --- Function Declarations ---

/*
    Deklarasi fungsi menu utama dan fungsi utilitas.
*/
void CustomerMenu();
void AdminMenu();
void ServiceMenuAdmin();
void clearScreen();
void cinClean();
bool isValidInt(int &input);
bool isValidChar(char &input);

// --- Helper Functions ---

/*
    Fungsi clearScreen()
    --------------------
    Membersihkan tampilan terminal.
    Menggunakan "cls" untuk Windows dan "clear" untuk Linux/macOS.
*/
void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

/*
    Fungsi cinClean()
    -----------------
    Membersihkan status error pada cin dan membuang sisa input buffer.
    Berguna setelah input gagal atau setelah penggunaan cin >>.
*/
void cinClean() {
    cin.clear();
    cin.ignore(10000,'\n');
}

/*
    Fungsi isValidInt()
    -------------------
    Memvalidasi apakah input integer berhasil dibaca.
    Jika gagal, buffer dibersihkan lalu mengembalikan false.
*/
bool isValidInt(int &input) {
    if (cin.fail()) {
        cout << "Input tidak valid! Input harus berupa angka." << endl;
        cinClean();
        return false;
    }
    return true;
}

/*
    Fungsi isValidChar()
    --------------------
    Memvalidasi apakah input karakter berhasil dibaca.
    Jika gagal, buffer dibersihkan lalu mengembalikan false.
*/
bool isValidChar(char &input) {
    if (cin.fail()) {
        cout << "Input tidak valid! Input harus berupa huruf." << endl;
        cinClean();
        return false;
    }
    return true;
}

/*
    Fungsi addCustomer()
    --------------------
    Menambahkan pelanggan baru ke akhir linked list customer.
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

/*
    Fungsi enqueuePending()
    -----------------------
    Menambahkan data servis ke antrian pending (queue).
*/
void enqueuePending(Service* s) {
    s->nextGlobal = nullptr;
    if (pendingTail == nullptr) {
        pendingHead = pendingTail = s;
    } else {
        pendingTail->nextGlobal = s;
        pendingTail = s;
    }
}

/*
    Fungsi addCompleted()
    ---------------------
    Menambahkan data servis ke daftar completed.
    Data disimpan di linked list servis selesai.
*/
void addCompleted(Service* s) {
    s->nextGlobal = nullptr;
    if (completedHead == nullptr) {
        completedHead = s;
    } else {
        Service* temp = completedHead;
        while(temp->nextGlobal != nullptr) temp = temp->nextGlobal;
        temp->nextGlobal = s;
    }
}

/*
    Fungsi findCustomer()
    ---------------------
    Mencari pelanggan berdasarkan nama.
    Mengembalikan pointer ke customer jika ditemukan,
    atau nullptr jika tidak ditemukan.
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

/*
    Fungsi saveData()
    -----------------
    Menyimpan seluruh data customer dan riwayat servis
    ke dalam file database.

    Format:
    - Baris customer diawali dengan "C"
    - Baris service diawali dengan "S,"
*/
void saveData() {
    ofstream file(DB_PATH);
    Customer* current = customerHead;
    while (current != nullptr) {
        file << "C" << border << current->name << border
        << current->age << border << current->gender
        << border << current->phone << border << current->address << endl;

        Service* s = current->serviceHistory;
        while (s != nullptr) {
            file << "S," << s->carBrand << border << s->carModel << border
                 << s->issueDesc << border << s->nameMechanic << border
                 << s->entryDate << border << (s->isCompleted ? "1" : "0") << endl;
            s = s->nextInHistory;
        }
        current = current->next;
    }
    file.close();
}

/*
    Fungsi loadData()
    -----------------
    Membaca data dari file database lalu membangun ulang:
    - linked list customer
    - riwayat servis tiap customer
    - antrian pending
    - daftar completed
*/
void loadData() {
    ifstream file(DB_PATH);
    if (!file.is_open()) return;

    string line;
    Customer* lastCust = nullptr;

    while (getline(file, line)) {
        if (line.empty()) continue;

        char type = line[0];
        string data = line.substr(2);
        stringstream ss(data);
        string token;

        if (type == 'C') {
            auto c = new Customer();

            getline(ss, c->name, '|');

            getline(ss, token, '|');
            c->age = stoi(token);

            getline(ss, token, '|');
            c->gender = token[0];

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

            if (getline(ss, s->entryDate, '|')) {
                getline(ss, token, '|');
                s->isCompleted = (token == "1");
            } else {
                s->entryDate = "N/A";
                s->isCompleted = false;
            }

            s->customerData = lastCust;
            s->nextInHistory = lastCust->serviceHistory;
            lastCust->serviceHistory = s;

            if(s->isCompleted) addCompleted(s);
            else enqueuePending(s);
        }
    }
    file.close();
}

// --- Customer Features ---

/*
    Fungsi CustomerQueue()
    ----------------------
    Menampilkan daftar antrian servis yang masih menunggu.
*/
void CustomerQueue() {
    clearScreen();
    cout << "====== Antrian Servis ======\n\n";
    Service* temp = pendingHead;
    if (!temp) cout << "Antrian kosong." << endl;
    while (temp != nullptr) {
        cout << "-----------------------" << endl;
        cout << "Merek Mobil: " << temp->carBrand << endl;
        cout << "Model Mobil: " << temp->carModel << endl;
        cout << "Kendala: " << temp->issueDesc << endl;
        cout << "Montir: " << temp->nameMechanic << endl;
        cout << "Tanggal Masuk: " << temp->entryDate << endl;
        cout << "Status: Menunggu Antrian" << endl;
        cout << "-----------------------\n";
        temp = temp->nextGlobal;
    }
    cout << "\nTekan Enter untuk kembali...";
    cin.get();
}

/*
    Fungsi CustomerHistory()
    ------------------------
    Menampilkan riwayat servis milik pelanggan
    berdasarkan nama yang dimasukkan.
*/
void CustomerHistory() {
    clearScreen();
    cout << "====== Riwayat Servis Anda ======\n\n";
    cout << "Masukkan Nama: >";
    string name; getline(cin, name);

    Customer* c = findCustomer(name);
    if(!c) {
        cout << "\nData pelanggan tidak ditemukan.\n";
    } else {
        cout << "\n====== Services =======\n";
        Service* s = c->serviceHistory;
        if(!s) cout << "Belum ada riwayat.\n";
        while(s) {
            cout << "Model Mobil: " << s->carModel << endl;
            cout << "Merek Mobil: " << s->carBrand << endl;
            cout << "Kendala: " << s->issueDesc << endl;
            cout << "Montir: " << s->nameMechanic << endl;
            cout << "Tanggal Masuk: " << s->entryDate << endl;
            cout << "Nama Pelanggan: " << s->customerData->name << endl;
            cout << "No Telp Pelanggan: " << s->customerData->phone << endl;
            cout << "Status: " << (s->isCompleted ? "Selesai" : "Menunggu") << "\n\n";
            s = s->nextInHistory;
        }
    }
    cout << "Press any key to go back ...";
    cin.get();
}

// --- Admin Features ---

/*
    Fungsi NewCustomerAndService()
    ------------------------------
    Digunakan admin untuk:
    1. Menambahkan pelanggan baru jika belum terdaftar
    2. Menambahkan data servis baru ke antrian pending
*/
void NewCustomerAndService() {
    clearScreen();
    cout << "====== Pendaftaran Servis Baru ======\n\n";
    cout << "Nama Pelanggan: >";
    string name; getline(cin, name);
    Customer* c = findCustomer(name);

    if (c == nullptr) {
        c = new Customer();
        c->name = name;
        cout << "No Telp Pelanggan: >"; getline(cin, c->phone);
        cout << "Alamat: >"; getline(cin, c->address);
        while (c->age <= 0) {
            cout << "Umur: >"; cin >> c->age;
            if (!isValidInt(c->age)) c->age = -1;
        }
        while (c->gender != 'L' && c->gender != 'P') {
            cout << "Gender (L/P): >"; cin >> c->gender;
            c->gender = char(toupper(c->gender));
            if (!isValidChar(c->gender)) continue;
        }
        cinClean();
        addCustomer(c);
        cout << "\n*Pelanggan baru terdaftar*\n";
    } else {
        cout << "\n*Data pelanggan ditemukan*\n";
    }

    auto* s = new Service();
    cout << "\nMerek Mobil: "; getline(cin, s->carBrand);
    cout << "Model Mobil: "; getline(cin, s->carModel);
    cout << "Kendala: "; getline(cin, s->issueDesc);
    cout << "Montir (Suby,Farhan,Dimas,Aldo,Rafi): "; getline(cin, s->nameMechanic);
    cout << "Tanggal Masuk (ex. 1-Mei-2026): "; getline(cin, s->entryDate);

    s->customerData = c;
    s->nextInHistory = c->serviceHistory;
    c->serviceHistory = s;
    s->isCompleted = false;

    enqueuePending(s);
    saveData();
    cout << "\n*Servis berhasil dicatat ke dalam antrian*\n";
    cout << "\nPress any key to go back...";
    cin.get();
}

/*
    Fungsi CompleteJob()
    --------------------
    Digunakan admin untuk menandai satu pekerjaan montir
    sebagai selesai, lalu memindahkannya dari pending ke completed.
*/
void CompleteJob() {
    clearScreen();
    string active[10];
    int count = 0;
    Service* temp = pendingHead;
    while(temp) {
        bool exists = false;
        for(int i=0; i<count; i++) {
            if(active[i] == temp->nameMechanic) exists = true;
        }
        if(!exists && count < 10) active[count++] = temp->nameMechanic;
        temp = temp->nextGlobal;
    }

    if(count == 0) {
        cout << "====== Jobs Done ======\n\n";
        cout << "Tidak ada servis yang sedang mengantri.\n\nTekan enter untuk kembali...";
        cin.get(); return;
    }

    cout << "====== Jobs Done ======\n\n";
    cout << "Pilih Montir!\n";
    for(int i=0; i<count; i++) cout << i+1 << ". " << active[i] << endl;
    cout << "Pilihan >";

    int pil; cin >> pil; cinClean();
    if(pil < 1 || pil > count) return;

    string target = active[pil-1];

    Service* prev = nullptr;
    Service* curr = pendingHead;
    while(curr != nullptr) {
        if(curr->nameMechanic == target) break;
        prev = curr;
        curr = curr->nextGlobal;
    }

    if(curr != nullptr) {
        clearScreen();
        cout << "====== Service Detail ======\n\n";
        cout << "Model Mobil: " << curr->carModel << endl;
        cout << "Merek Mobil: " << curr->carBrand << endl;
        cout << "Kendala: " << curr->issueDesc << endl;
        cout << "Montir: " << curr->nameMechanic << endl;
        cout << "Tanggal Masuk: " << curr->entryDate << endl;
        cout << "Nama Pelanggan: " << curr->customerData->name << endl;
        cout << "No Telp Pelanggan: " << curr->customerData->phone << endl;

        cout << "\nApakah servis ini sudah selesai? (yes/no): ";
        string ans; getline(cin, ans);
        if(ans == "yes" || ans == "y") {
            if(prev == nullptr) {
                pendingHead = curr->nextGlobal;
                if(!pendingHead) pendingTail = nullptr;
            } else {
                prev->nextGlobal = curr->nextGlobal;
                if(!curr->nextGlobal) pendingTail = prev;
            }

            curr->isCompleted = true;
            addCompleted(curr);
            saveData();
            cout << "\n*Servis berhasil diselesaikan dan dipindahkan ke riwayat!*\n";
        }
    }
    cout << "\nPress any key to go back...";
    cin.get();
}

/*
    Fungsi MechanicHistoryAdmin()
    -----------------------------
    Menampilkan daftar pekerjaan seorang montir,
    baik yang masih pending maupun yang sudah selesai.
*/
void MechanicHistoryAdmin() {
    clearScreen();
    string active[20];
    int count = 0;

    Service* temp = pendingHead;
    while(temp) {
        bool exists = false;
        for(int i=0; i<count; i++) if(active[i] == temp->nameMechanic) exists = true;
        if(!exists && count < 20) active[count++] = temp->nameMechanic;
        temp = temp->nextGlobal;
    }
    temp = completedHead;
    while(temp) {
        bool exists = false;
        for(int i=0; i<count; i++) if(active[i] == temp->nameMechanic) exists = true;
        if(!exists && count < 20) active[count++] = temp->nameMechanic;
        temp = temp->nextGlobal;
    }

    if(count == 0) {
        cout << "====== Riwayat Kerja Montir ======\n\n";
        cout << "Belum ada riwayat kerja sama sekali.\n\nTekan Enter untuk kembali...";
        cin.get(); return;
    }

    cout << "====== Riwayat Kerja Montir ======\n\n";
    cout << "Pilih Montir!\n";
    for(int i=0; i<count; i++) cout << i+1 << ". " << active[i] << endl;
    cout << "Pilihan: ";

    int pil; cin >> pil; cinClean();
    if(pil < 1 || pil > count) return;

    string target = active[pil-1];
    clearScreen();
    cout << "====== " << target << "'s Jobs ======\n\n";

    temp = pendingHead;
    while(temp) {
        if(temp->nameMechanic == target) {
            cout << "[-] (PENDING) " << temp->carBrand << " " << temp->carModel << " - " << temp->issueDesc << endl;
        }
        temp = temp->nextGlobal;
    }
    temp = completedHead;
    while(temp) {
        if(temp->nameMechanic == target) {
            cout << "[v] (SELESAI) " << temp->carBrand << " " << temp->carModel << " - " << temp->issueDesc << endl;
        }
        temp = temp->nextGlobal;
    }
    cout << "\nTekan Enter untuk kembali..."; cin.get();
}

/*
    Fungsi ServiceMenuAdmin()
    -------------------------
    Menu khusus admin untuk pengelolaan servis.
*/
void ServiceMenuAdmin() {
    int pil = -1;
    do {
        clearScreen();
        cout << "====== Welcome to Lognuts (ADMIN - Service Menu) ======\n\n";
        cout << "Pilih menu!\n";
        cout << "1. Semua Servis Singkat\n2. Servis Baru\n3. Selesaikan Pekerjaan\n4. Riwayat Kerja Montir\n0. Kembali\nPilihan: ";
        cin >> pil; cinClean();

        switch (pil) {
            case 1: CustomerQueue(); break;
            case 2: NewCustomerAndService(); break;
            case 3: CompleteJob(); break;
            case 4: MechanicHistoryAdmin(); break;
            case 0: break;
            default:
                cout << "Pilihan Salah!\nTekan enter...";
                cin.get();
        }
    } while (pil != 0);
}

/*
    Fungsi AdminMenu()
    ------------------
    Menu utama admin untuk mengakses fitur internal sistem.
*/
void AdminMenu() {
    int pil = -1;
    do {
        clearScreen();
        cout << "====== Welcome to Lognuts (ADMIN) ======\n\n";
        cout << "Pilih menu!\n";
        cout << "1. Servis\n";
        cout << "2. Pelanggan Baru\n";
        cout << "3. Keluar\n";
        cout << "Pilihan: ";
        cin >> pil; cinClean();

        switch (pil) {
            case 1: ServiceMenuAdmin(); break;
            case 2: NewCustomerAndService(); break;
            case 3: return;
            default:
                cout << "Pilihan salah!\nTekan enter...";
                cin.get();
        }
    } while (pil != 3);
}

/*
    Fungsi CustomerMenu()
    ---------------------
    Menu utama yang ditampilkan saat program dijalankan.
    Pengguna biasa dapat melihat antrian dan riwayat servis.
    Jika memasukkan kode khusus admin, maka masuk ke AdminMenu().
*/
void CustomerMenu() {
    string input;
    do {
        clearScreen();
        cout << "====== Welcome to Lognuts ======\n\n";
        cout << "Pilih menu!\n";
        cout << "1. Antrian Servis\n";
        cout << "2. Riwayat Servis Anda\n";
        cout << "3. Keluar\n";
        cout << "Pilihan: ";
        cin >> input;

        if (input == "adminacces8008") {
            cinClean();
            AdminMenu();
            continue;
        }

        int pil = -1;
        try { pil = stoi(input); } catch(...) { pil = -1; }
        cinClean();

        switch (pil) {
            case 1: CustomerQueue(); break;
            case 2: CustomerHistory(); break;
            case 3: cout << "\nBye!\n"; return;
            default:
                cout << "Pilihan salah!\nTekan enter...";
                cin.get();
        }
    } while (true);
}
int main() {
    loadData();
    CustomerMenu();
    return 0;
}
