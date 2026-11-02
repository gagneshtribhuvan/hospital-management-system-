// Hospital Management System - Prototype (console based, C++17)
// Compile : g++ -std=c++17 -Wall -Wextra hospital_management.cpp -o hospital
// Run     : ./hospital
//
// Features: patients, doctors, appointments, admit/discharge with billing,
//           data saved in simple text files (patients.txt, doctors.txt, appointments.txt)

#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

const double BED_CHARGE_PER_DAY = 1500.0;
const string PATIENT_FILE = "patients.txt";
const string DOCTOR_FILE = "doctors.txt";
const string APPT_FILE = "appointments.txt";

// ---------- Data models ----------
struct Patient {
    int id = 0;
    string name;
    int age = 0;
    string gender;
    string disease;
    bool admitted = false;
};

struct Doctor {
    int id = 0;
    string name;
    string specialization;
    double fee = 0;
};

struct Appointment {
    int id = 0;
    int patientId = 0;
    int doctorId = 0;
    string date;
};

// ---------- Helpers ----------
static string toLower(string s) {
    transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return tolower(c); });
    return s;
}

static string clean(string s) {  // '|' is our file separator
    replace(s.begin(), s.end(), '|', ' ');
    return s;
}

static vector<string> split(const string& line, char delim = '|') {
    vector<string> parts;
    string item;
    stringstream ss(line);
    while (getline(ss, item, delim)) parts.push_back(item);
    return parts;
}

static int readInt(const string& prompt) {
    int v;
    while (true) {
        cout << prompt;
        if (cin >> v) {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return v;
        }
        if (cin.eof()) exit(0);
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Invalid input. Please enter a number.\n";
    }
}

static double readDouble(const string& prompt) {
    double v;
    while (true) {
        cout << prompt;
        if (cin >> v) {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return v;
        }
        if (cin.eof()) exit(0);
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Invalid input. Please enter a number.\n";
    }
}

static string readLine(const string& prompt) {
    string s;
    cout << prompt;
    if (!getline(cin, s)) exit(0);
    return clean(s);
}

// ---------- Hospital ----------
class Hospital {
    vector<Patient> patients;
    vector<Doctor> doctors;
    vector<Appointment> appointments;
    int nextPatientId = 1, nextDoctorId = 1, nextApptId = 1;

    Patient* findPatient(int id) {
        for (auto& p : patients)
            if (p.id == id) return &p;
        return nullptr;
    }
    Doctor* findDoctor(int id) {
        for (auto& d : doctors)
            if (d.id == id) return &d;
        return nullptr;
    }

public:
    // ----- persistence -----
    void load() {
        string line;
        ifstream pf(PATIENT_FILE);
        while (getline(pf, line)) {
            auto f = split(line);
            if (f.size() < 6) continue;
            Patient p;
            p.id = stoi(f[0]);
            p.name = f[1];
            p.age = stoi(f[2]);
            p.gender = f[3];
            p.disease = f[4];
            p.admitted = (f[5] == "1");
            patients.push_back(p);
            nextPatientId = max(nextPatientId, p.id + 1);
        }
        ifstream df(DOCTOR_FILE);
        while (getline(df, line)) {
            auto f = split(line);
            if (f.size() < 4) continue;
            Doctor d;
            d.id = stoi(f[0]);
            d.name = f[1];
            d.specialization = f[2];
            d.fee = stod(f[3]);
            doctors.push_back(d);
            nextDoctorId = max(nextDoctorId, d.id + 1);
        }
        ifstream af(APPT_FILE);
        while (getline(af, line)) {
            auto f = split(line);
            if (f.size() < 4) continue;
            Appointment a;
            a.id = stoi(f[0]);
            a.patientId = stoi(f[1]);
            a.doctorId = stoi(f[2]);
            a.date = f[3];
            appointments.push_back(a);
            nextApptId = max(nextApptId, a.id + 1);
        }
        if (doctors.empty()) seedDemoDoctors();
    }

    void save() {
        ofstream pf(PATIENT_FILE);
        for (auto& p : patients)
            pf << p.id << '|' << p.name << '|' << p.age << '|' << p.gender << '|' << p.disease
               << '|' << (p.admitted ? 1 : 0) << '\n';
        ofstream df(DOCTOR_FILE);
        for (auto& d : doctors)
            df << d.id << '|' << d.name << '|' << d.specialization << '|' << d.fee << '\n';
        ofstream af(APPT_FILE);
        for (auto& a : appointments)
            af << a.id << '|' << a.patientId << '|' << a.doctorId << '|' << a.date << '\n';
        cout << "Data saved.\n";
    }

    void seedDemoDoctors() {
        doctors.push_back({nextDoctorId++, "Dr. Sharma", "Cardiology", 800});
        doctors.push_back({nextDoctorId++, "Dr. Patil", "General Medicine", 400});
        doctors.push_back({nextDoctorId++, "Dr. Deshmukh", "Orthopedics", 600});
    }

    // ----- patients -----
    void addPatient() {
        Patient p;
        p.id = nextPatientId++;
        p.name = readLine("Name: ");
        p.age = readInt("Age: ");
        p.gender = readLine("Gender (M/F/O): ");
        p.disease = readLine("Disease / complaint: ");
        patients.push_back(p);
        cout << "Patient added. Patient ID = " << p.id << "\n";
    }

    void printPatientHeader() {
        cout << left << setw(5) << "ID" << setw(22) << "Name" << setw(6) << "Age" << setw(8)
             << "Gender" << setw(22) << "Disease" << "Status\n";
        cout << string(70, '-') << "\n";
    }

    void printPatientRow(const Patient& p) {
        cout << left << setw(5) << p.id << setw(22) << p.name << setw(6) << p.age << setw(8)
             << p.gender << setw(22) << p.disease << (p.admitted ? "Admitted" : "Outpatient")
             << "\n";
    }

    void listPatients() {
        if (patients.empty()) {
            cout << "No patients yet.\n";
            return;
        }
        printPatientHeader();
        for (auto& p : patients) printPatientRow(p);
    }

    void searchPatient() {
        string key = toLower(readLine("Enter patient ID or name: "));
        bool found = false;
        for (auto& p : patients) {
            if (to_string(p.id) == key || toLower(p.name).find(key) != string::npos) {
                if (!found) printPatientHeader();
                printPatientRow(p);
                found = true;
            }
        }
        if (!found) cout << "No matching patient.\n";
    }

    void admitPatient() {
        Patient* p = findPatient(readInt("Patient ID: "));
        if (!p) {
            cout << "Patient not found.\n";
            return;
        }
        if (p->admitted) {
            cout << "Patient is already admitted.\n";
            return;
        }
        p->admitted = true;
        cout << p->name << " admitted.\n";
    }

    void dischargePatient() {
        int id = readInt("Patient ID: ");
        Patient* p = findPatient(id);
        if (!p) {
            cout << "Patient not found.\n";
            return;
        }
        if (!p->admitted) {
            cout << "Patient is not admitted.\n";
            return;
        }
        int days = readInt("Number of days stayed: ");
        if (days < 1) days = 1;

        double bed = days * BED_CHARGE_PER_DAY;
        double consult = 0;
        for (auto& a : appointments) {
            if (a.patientId != id) continue;
            Doctor* d = findDoctor(a.doctorId);
            if (d) consult += d->fee;
        }

        cout << fixed << setprecision(2);
        cout << "\n======== BILL ========\n";
        cout << "Patient : " << p->name << " (ID " << p->id << ")\n";
        cout << "Bed charges (" << days << " days x " << BED_CHARGE_PER_DAY << ") : " << bed << "\n";
        cout << "Doctor consultation fees            : " << consult << "\n";
        cout << "TOTAL                               : " << bed + consult << "\n";
        cout << "======================\n";
        p->admitted = false;
        cout << "Patient discharged.\n";
    }

    // ----- doctors -----
    void addDoctor() {
        Doctor d;
        d.id = nextDoctorId++;
        d.name = readLine("Doctor name: ");
        d.specialization = readLine("Specialization: ");
        d.fee = readDouble("Consultation fee: ");
        doctors.push_back(d);
        cout << "Doctor added. Doctor ID = " << d.id << "\n";
    }

    void listDoctors() {
        if (doctors.empty()) {
            cout << "No doctors yet.\n";
            return;
        }
        cout << left << setw(5) << "ID" << setw(22) << "Name" << setw(22) << "Specialization"
             << "Fee\n";
        cout << string(55, '-') << "\n";
        for (auto& d : doctors)
            cout << left << setw(5) << d.id << setw(22) << d.name << setw(22) << d.specialization
                 << d.fee << "\n";
    }

    // ----- appointments -----
    void bookAppointment() {
        Patient* p = findPatient(readInt("Patient ID: "));
        if (!p) {
            cout << "Patient not found.\n";
            return;
        }
        listDoctors();
        Doctor* d = findDoctor(readInt("Doctor ID: "));
        if (!d) {
            cout << "Doctor not found.\n";
            return;
        }
        Appointment a;
        a.id = nextApptId++;
        a.patientId = p->id;
        a.doctorId = d->id;
        a.date = readLine("Date (DD-MM-YYYY): ");
        appointments.push_back(a);
        cout << "Appointment booked. ID = " << a.id << " (" << p->name << " with " << d->name
             << " on " << a.date << ")\n";
    }

    void listAppointments() {
        if (appointments.empty()) {
            cout << "No appointments yet.\n";
            return;
        }
        cout << left << setw(5) << "ID" << setw(22) << "Patient" << setw(22) << "Doctor" << "Date\n";
        cout << string(60, '-') << "\n";
        for (auto& a : appointments) {
            Patient* p = findPatient(a.patientId);
            Doctor* d = findDoctor(a.doctorId);
            cout << left << setw(5) << a.id << setw(22) << (p ? p->name : "?") << setw(22)
                 << (d ? d->name : "?") << a.date << "\n";
        }
    }
};

// ---------- Main menu ----------
static void showMenu() {
    cout << "\n===== HOSPITAL MANAGEMENT SYSTEM =====\n"
         << " 1. Add patient\n"
         << " 2. View all patients\n"
         << " 3. Search patient\n"
         << " 4. Add doctor\n"
         << " 5. View doctors\n"
         << " 6. Book appointment\n"
         << " 7. View appointments\n"
         << " 8. Admit patient\n"
         << " 9. Discharge patient & generate bill\n"
         << " 0. Save & exit\n";
}

int main() {
    Hospital h;
    h.load();

    while (true) {
        showMenu();
        int choice = readInt("Choose an option: ");
        cout << "\n";
        switch (choice) {
            case 1: h.addPatient(); break;
            case 2: h.listPatients(); break;
            case 3: h.searchPatient(); break;
            case 4: h.addDoctor(); break;
            case 5: h.listDoctors(); break;
            case 6: h.bookAppointment(); break;
            case 7: h.listAppointments(); break;
            case 8: h.admitPatient(); break;
            case 9: h.dischargePatient(); break;
            case 0:
                h.save();
                cout << "Goodbye!\n";
                return 0;
            default: cout << "Invalid option. Try again.\n";
        }
    }
}
