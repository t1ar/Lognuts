#include <iostream>
#include <fstream>
#include <string>

using namespace std;
const string border = "|";

// --- Structs ---
struct Service;
struct Customer {
    string name;
    string age;
    char gender{};
    string phone;
    string address;
    Service* serviceHistory = nullptr;
    Customer* next = nullptr;
    Customer* prev = nullptr;
};

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
Customer* customerHead = nullptr;
Customer* customerTail = nullptr;
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
void addCustomer(Customer* newCust) {
    if (customerHead == nullptr) {
        customerHead = customerTail = newCust;
    } else {
        customerTail->next = newCust;
        newCust->prev = customerTail;
        customerTail = newCust;
    }
}

void addService(Service* newServ) {
    if (serviceHead == nullptr) {
        serviceHead = newServ;
    } else {
        Service* temp = serviceHead;
        while (temp->nextGlobal != nullptr) temp = temp->nextGlobal;
        temp->nextGlobal = newServ;
    }
}

Customer* findCustomer(const string& name) {
    Customer* temp = customerHead;
    while (temp != nullptr) {
        if (temp->name == name) return temp;
        temp = temp->next;
    }
    return nullptr;
}

// --- IO Data ---
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
            c->age = data.substr(s1 + 1, s2 - s1 - 1);
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
    cin.get();
}

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
        cout << "Umur: "; cin >> c->age;
        cout << "Gender (L/P): "; cin >> c->gender;
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

void MechanicHistory() {
    int index;
    cout << "\nPilih Montir:\n(1)Suby (2)Farhan (3)Dimas (4)Aldo\nInput: ";
    cin >> index;
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
    cin.ignore(); cin.get();
}

void ServiceMenu() {
    int pil;
    do {
        cout << "\n====== Services ======\n1. Semua Servis\n2. Servis Baru\n3. Riwayat Montir\n0. Kembali\nPilihan: ";
        cin >> pil; cin.ignore();
        switch (pil) {
            case 1: AllShortService(); break;
            case 2: NewService(); break;
            case 3: MechanicHistory(); break;
            default: cout << "Pilihan Salah!";
        }
    } while (pil != 0);
}

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
    cout << "\nTekan Enter..."; cin.get();
}

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
        if (curr->gender == 'L' || curr->gender == 'l') cout << "Laki-laki" << endl;
        else if (curr->gender == 'P' || curr->gender == 'p') cout << "Perempuan" << endl;
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
        cin.ignore(); // Bersihkan buffer agar tidak skip input berikutnya

        if ((nav == 'N' || nav == 'n') && curr->next) curr = curr->next;
        else if ((nav == 'P' || nav == 'p') && curr->prev) curr = curr->prev;
        else if (nav != 'E' && nav != 'e') {
            cout << "[!] Pilihan tidak tersedia atau sudah di ujung data." << endl;
            cout << "\nTekan Enter untuk kembali..."; cin.get();
        }
    } while (nav != 'E' && nav != 'e');
}

void MainMenu() {
    int pil;
    do {
        cout << "====== Lognuts ======\n" << endl;
        cout << "Pilih opsi: " << endl;
        cout << "1. Servis" << endl;
        cout << "2. Semua Pelanggan" << endl;
        cout << "3. Navigasi Pelanggan" << endl;
        cout << "0. Keluar\n" << endl;
        cout << "Pilihan: ";
        cin >> pil;
        switch (pil) {
            case 1: cin.ignore(); ServiceMenu(); break;
            case 2: cin.ignore(); AllCustomerData(); break;
            case 3: cin.ignore(); IndividualCustomerData(); break;
            case 0: cout << "Bye!"; break;
            default: cout << "Pilihan salah!";
        }
    } while (pil != 0);
}

int main() {
    loadData();
    MainMenu();
    return 0;
}