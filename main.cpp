#include <iostream>
#include <string>
#include <cctype>
#include <vector>
#include <iomanip>
#include <fstream>
#include <regex>

using namespace std;

// ============================================================
// ROLES & PERMISSIONS
// ============================================================

enum Role {
    ROLE_ADMIN = 0,
    ROLE_EMPLOYEE = 1,
    ROLE_CUSTOMER = 2
};

string roleToString(Role r) {
    switch (r) {
    case ROLE_ADMIN:    return "Admin";
    case ROLE_EMPLOYEE: return "Employee";
    case ROLE_CUSTOMER: return "Customer";
    default:            return "Unknown";
    }
}

enum Action {
    ACTION_VIEW_ALL = 0,
    ACTION_CREATE_FOR_OTHERS = 1,
    ACTION_EDIT_OTHERS = 2,
    ACTION_DELETE_OTHERS = 3,
    ACTION_MANAGE_ROOMS = 4,
    ACTION_MANAGE_ALL_BOOKINGS = 5,
    ACTION_MANAGE_ALL_PAYMENTS = 6,
    ACTION_VIEW_REPORTS = 7,
    ACTION_COUNT = 8
};

const int ROLE_COUNT = 3;

bool permissionMatrix[ROLE_COUNT][ACTION_COUNT] = {
    // VIEW_ALL CREATE_OTH EDIT_OTH DEL_OTH MNG_ROOM MNG_BOOK MNG_PAY  VIEW_REPORTS
    { true,    true,      true,    true,   true,   true,     true,    true  },  // Admin
    { true,    false,     false,   false,  true,   true,     true,    false },  // Employee
    { false,   false,     false,   false,  false,  false,    false,   false }   // Customer
};

bool hasPermission(Role role, Action action) {
    return permissionMatrix[role][action];
}

// ============================================================
// USER MODULE
// ============================================================

struct User {
    string id;
    string username;
    string password;
    string email;
    Role   role;
    bool   active;
    double totalPaid = 0.0;
    int    bookingCount = 0;
};

bool isValidEmail(const string& email) {
    if (email.size() < 5 || email.size() > 254) return false;
    static const std::regex pattern(
        R"(^[A-Za-z0-9](?:[A-Za-z0-9._%+-]*[A-Za-z0-9])?@[A-Za-z0-9](?:[A-Za-z0-9-]*[A-Za-z0-9])?(?:\.[A-Za-z0-9](?:[A-Za-z0-9-]*[A-Za-z0-9])?)*\.[A-Za-z]{2,}$)"
    );

    if (email.find("..") != string::npos) return false;

    return std::regex_match(email, pattern);
}

const int MAX_USERS = 100;

struct UserModule {
    vector<User> users;
    int          nextId;

    UserModule() : nextId(1) {}

    int findIndexByUsername(const string& username, bool includeInactive = false) {
        for (size_t i = 0; i < users.size(); i++) {
            if (users[i].username == username) {
                if (users[i].active || includeInactive) return static_cast<int>(i);
            }
        }
        return -1;
    }

    int findIndexById(const string& id) {
        for (size_t i = 0; i < users.size(); i++) {
            if (users[i].id == id) return static_cast<int>(i);
        }
        return -1;
    }

    bool createAccount(const string& username, const string& password, const string& email) {
        return createAccountWithRole(username, password, email, ROLE_CUSTOMER);
    }

    bool createAccountWithRole(const string& username, const string& password,
        const string& email, Role role) {
        if (static_cast<int>(users.size()) >= MAX_USERS) {
            cout << "[Error] User storage is full.\n";
            return false;
        }
        if (username.empty() || password.empty()) {
            cout << "[Error] Username/password cannot be empty.\n";
            return false;
        }
        if (!isValidEmail(email)) {
            cout << "[Error] Invalid email format: '" << email
                << "' (must follow the format, e.g. name@example.com).\n";
            return false;
        }
        if (findIndexByUsername(username) != -1) {
            cout << "[Error] Username '" << username << "' already exists.\n";
            return false;
        }

        User u;
        u.id = "U" + to_string(nextId++);
        u.username = username;
        u.password = password;
        u.email = email;
        u.role = role;
        u.active = true;

        users.push_back(u);
        cout << "[OK] Account created: " << username
            << " (email: " << email
            << ", role: " << roleToString(role) << ", id: " << u.id << ")\n";
        return true;
    }

    string loginGetId(const string& username, const string& password) {
        int idx = findIndexByUsername(username);
        if (idx == -1) {
            cout << "[Login Failed] User not found or inactive.\n";
            return "";
        }
        if (users[idx].password != password) {
            cout << "[Login Failed] Incorrect password.\n";
            return "";
        }
        cout << "[Login OK] Welcome " << users[idx].username
            << " (" << roleToString(users[idx].role) << ")\n";
        return users[idx].id;
    }

    bool editAccount(const string& id, const string& newUsername, const string& newPassword,
    const string& newEmail, Role newRole) {
    int idx = findIndexById(id);
    if (idx == -1 || !users[idx].active) {
        cout << "[Error] Account id " << id << " not found.\n";
        return false;
    }


    if (!newUsername.empty() && newUsername != users[idx].username) {
        if (findIndexByUsername(newUsername) != -1) {
            cout << "[Error] Username '" << newUsername << "' already taken.\n";
            return false;
        }
    }
    if (!newEmail.empty() && !isValidEmail(newEmail)) {
        cout << "[Error] Invalid email format: '" << newEmail << "'.\n";
        return false;
    }


    if (!newUsername.empty()) users[idx].username = newUsername;
    if (!newPassword.empty()) users[idx].password = newPassword;
    if (!newEmail.empty())    users[idx].email = newEmail;
    users[idx].role = newRole;

    cout << "[OK] Account id " << id << " updated.\n";
    return true;
}

    bool editOwnAccount(const string& id, const string& newUsername, const string& newPassword,
        const string& newEmail) {
        int idx = findIndexById(id);
        if (idx == -1 || !users[idx].active) {
            cout << "[Error] Account id " << id << " not found.\n";
            return false;
        }
        Role lockedRole = users[idx].role;
        return editAccount(id, newUsername, newPassword, newEmail, lockedRole);
    }

    bool deleteAccount(const string& id) {
        int idx = findIndexById(id);
        if (idx == -1 || !users[idx].active) {
            cout << "[Error] Account id " << id << " not found.\n";
            return false;
        }
        users[idx].active = false;
        cout << "[OK] Account id " << id << " (" << users[idx].username << ") deleted.\n";
        return true;
    }

    void listAccounts() {
        cout << "\n--- User List ---\n";
        for (size_t i = 0; i < users.size(); i++) {
            if (!users[i].active) continue;
            cout << "ID: " << users[i].id
                << " | Username: " << users[i].username
                << " | Email: " << users[i].email
                << " | Role: " << roleToString(users[i].role)
                << " | Bookings: " << users[i].bookingCount
                << " | Total Paid: RM" << fixed << setprecision(2) << users[i].totalPaid << "\n";
        }
        cout << "-----------------\n";
    }

    void printOwnInfo(const string& id) {
        int idx = findIndexById(id);
        if (idx == -1 || !users[idx].active) {
            cout << "[Error] Account id " << id << " not found.\n";
            return;
        }
        cout << "\n--- My Info ---\n";
        cout << "ID: " << users[idx].id
            << " | Username: " << users[idx].username
            << " | Email: " << users[idx].email
            << " | Role: " << roleToString(users[idx].role)
            << " | Bookings: " << users[idx].bookingCount
            << " | Total Paid: RM" << fixed << setprecision(2) << users[idx].totalPaid << "\n";
        cout << "---------------\n";
    }

    Role getRole(const string& id) {
        int idx = findIndexById(id);
        return (idx != -1) ? users[idx].role : ROLE_CUSTOMER;
    }

    void recordBooking(const string& id) {
        int idx = findIndexById(id);
        if (idx != -1) users[idx].bookingCount++;
    }

    void recordPayment(const string& id, double amount) {
        int idx = findIndexById(id);
        if (idx != -1) users[idx].totalPaid += amount;
    }

    void viewInfo(const string& id) {
        int idx = findIndexById(id);
        if (idx == -1 || !users[idx].active) {
            cout << "[Error] Account id " << id << " not found.\n";
            return;
        }
        if (hasPermission(users[idx].role, ACTION_VIEW_ALL)) {
            listAccounts();
        }
        else {
            printOwnInfo(id);
        }
    }


    int getActiveUserCount() const {
        int count = 0;
        for (const User& u : users) if (u.active) count++;
        return count;
    }


    int getActiveUserCountByRole(Role role) const {
        int count = 0;
        for (const User& u : users) if (u.active && u.role == role) count++;
        return count;
    }
};

// ============================================================
// ROOM MODULE
// ============================================================

const int MAXROOMS = 200;

struct Room {
    string roomID;
    string roomType;
    double roomPrice;
    string roomDescription;
    string roomStatus;
};

void printLine(char symbol = '=', int length = 80) {
    for (int i = 0; i < length; i++) cout << symbol;
    cout << endl;
}

void printTitle(string title) {
    cout << endl;
    printLine('=');
    cout << setw(50) << right << title << endl;
    printLine('=');
}

void printRoomHeader() {
    printLine('-');
    cout << left
        << setw(12) << "Room ID"
        << setw(18) << "Room Type"
        << setw(15) << "Price(RM)"
        << setw(20) << "Status"
        << setw(25) << "Description"
        << endl;
    printLine('-');
}

void printRoomRow(const Room& room) {
    cout << left
        << setw(12) << room.roomID
        << setw(18) << room.roomType
        << setw(15) << fixed << setprecision(2) << room.roomPrice
        << setw(20) << room.roomStatus
        << setw(25) << room.roomDescription
        << endl;
}

bool isValidMenuChoice(int choice, int min, int max) {
    return choice >= min && choice <= max;
}

bool isValidPrice(double price) {
    return price > 0;
}

bool isValidString(string text) {
    return !text.empty();
}

bool isValidRoomStatusName(string status) {
    return status == "Available" ||
        status == "Unavailable" ||
        status == "Occupied" ||
        status == "Under Maintenance";
}

bool isValidRoomIDFormat(string roomID) {
    if (roomID.length() != 4) return false;
    if (roomID[0] != 'R') return false;
    for (int i = 1; i < 4; i++) {
        if (!isdigit(roomID[i])) return false;
    }
    int number = stoi(roomID.substr(1));
    return number >= 1;
}

bool isRoomInputFail() {
    if (cin.fail()) {
        cin.clear();
        cin.ignore(1000, '\n');
        return true;
    }
    cin.ignore(1000, '\n');
    return false;
}

int getValidatedInt(string label, int min, int max) {
    int value;
    bool isValid = false;
    do {
        cout << label;
        cin >> value;
        if (isRoomInputFail()) {
            cout << "Invalid input. Please enter a number.\n";
        }
        else if (!isValidMenuChoice(value, min, max)) {
            cout << "Invalid choice. Please enter a number between "
                << min << " and " << max << ".\n";
        }
        else {
            isValid = true;
        }
    } while (!isValid);
    return value;
}

double getValidatedPrice(string label) {
    double price;
    bool isValid = false;
    do {
        cout << label;
        cin >> price;
        if (isRoomInputFail()) {
            cout << "Invalid input. Please enter a valid number.\n";
        }
        else if (!isValidPrice(price)) {
            cout << "Price must be greater than 0.\n";
        }
        else {
            isValid = true;
        }
    } while (!isValid);
    return price;
}

string getValidatedString(string label) {
    string text;
    bool isValid = false;
    do {
        cout << label;
        getline(cin, text);
        if (!isValidString(text)) {
            cout << "Input cannot be empty.\n";
        }
        else {
            isValid = true;
        }
    } while (!isValid);
    return text;
}

struct BookingModule;

struct RoomModule {
    vector<Room> rooms;

    int searchRoomIndexByID(const string& roomID) const {
        for (size_t i = 0; i < rooms.size(); i++) {
            if (rooms[i].roomID == roomID) return static_cast<int>(i);
        }
        return -1;
    }

    int getRoomIDNumber(const string& roomID) const {
        if (!isValidRoomIDFormat(roomID)) return 0;
        return stoi(roomID.substr(1));
    }

    string formatRoomID(int number) const {
        if (number < 10) return "R00" + to_string(number);
        else if (number < 100) return "R0" + to_string(number);
        else return "R" + to_string(number);
    }

    string generateRoomID() const {
        int largestID = 0;
        for (const Room& room : rooms) {
            int currentID = getRoomIDNumber(room.roomID);
            if (currentID > largestID) largestID = currentID;
        }
        return formatRoomID(largestID + 1);
    }

    void assignDefaultRoomDetails(Room& room, int choice) {
        switch (choice) {
        case 1: room.roomType = "Standard"; room.roomPrice = 120.00; room.roomDescription = "Basic room"; break;
        case 2: room.roomType = "Deluxe";   room.roomPrice = 220.00; room.roomDescription = "Larger room"; break;
        case 3: room.roomType = "Suite";    room.roomPrice = 450.00; room.roomDescription = "Premium room"; break;
        }
    }

    void createRoom() {
        if (rooms.size() >= MAXROOMS) {
            cout << "Maximum number of rooms reached.\n";
            return;
        }

        Room newRoom;
        printTitle("CREATE ROOM");
        cout << "1. Standard Room\n";
        cout << "2. Deluxe Room\n";
        cout << "3. Suite Room\n";
        cout << "4. Other Room Type\n";
        cout << "0. Cancel\n";

        int choice = getValidatedInt("Choice: ", 0, 4);
        if (choice == 0) {
            cout << "Room creation cancelled.\n";
            return;
        }

        newRoom.roomID = generateRoomID();
        newRoom.roomStatus = "Available";

        if (choice >= 1 && choice <= 3) {
            assignDefaultRoomDetails(newRoom, choice);
        }
        else {
            newRoom.roomType = getValidatedString("Enter room type: ");
            newRoom.roomDescription = getValidatedString("Enter room description: ");
            newRoom.roomPrice = getValidatedPrice("Enter room price: RM ");
        }

        rooms.push_back(newRoom);
        cout << "\nRoom created successfully.\n";
        cout << "Generated Room ID: " << newRoom.roomID << endl;
    }

    void listRooms() const {
        printTitle("ALL ROOM LIST");
        if (rooms.empty()) {
            cout << "No room records found.\n";
            return;
        }
        printRoomHeader();
        for (const Room& room : rooms) printRoomRow(room);
        printLine('-');
        cout << "Total rooms: " << rooms.size() << endl;
    }

    void listAvailableRooms() const {
        bool found = false;
        printTitle("AVAILABLE ROOM LIST");
        if (rooms.empty()) {
            cout << "No room records found.\n";
            return;
        }
        printRoomHeader();
        for (const Room& room : rooms) {
            if (room.roomStatus == "Available") {
                printRoomRow(room);
                found = true;
            }
        }
        printLine('-');
        if (!found) cout << "No available rooms found.\n";
    }

    void searchRoomByID() const {
        printTitle("SEARCH ROOM BY ID");
        string roomID = getValidatedString("Enter Room ID: ");
        int index = searchRoomIndexByID(roomID);
        if (index == -1) {
            cout << "Invalid Room ID. Room not found.\n";
            return;
        }
        printRoomHeader();
        printRoomRow(rooms[index]);
        printLine('-');
    }

    void searchRoomByType() const {
        printTitle("SEARCH ROOM BY TYPE");
        string roomType = getValidatedString("Enter room type: ");
        bool found = false;
        printRoomHeader();
        for (const Room& room : rooms) {
            if (room.roomType == roomType) {
                printRoomRow(room);
                found = true;
            }
        }
        printLine('-');
        if (!found) cout << "No room found with this room type.\n";
    }

    string promptStatusChoice() const {
        cout << "1. Available\n";
        cout << "2. Unavailable\n";
        cout << "3. Occupied\n";
        cout << "4. Under Maintenance\n";
        cout << "0. Cancel\n";

        int choice = getValidatedInt("Choice: ", 0, 4);
        switch (choice) {
        case 1: return "Available";
        case 2: return "Unavailable";
        case 3: return "Occupied";
        case 4: return "Under Maintenance";
        default: return "Cancel";
        }
    }

    void searchRoomByStatus() const {
        printTitle("SEARCH ROOM BY STATUS");
        string status = promptStatusChoice();
        if (status == "Cancel") {
            cout << "Search cancelled.\n";
            return;
        }

        bool found = false;
        printRoomHeader();
        for (const Room& room : rooms) {
            if (room.roomStatus == status) {
                printRoomRow(room);
                found = true;
            }
        }
        printLine('-');
        if (!found) cout << "No room found with this status.\n";
    }

    void searchRoomByTypeAndStatus() const {
        printTitle("SEARCH ROOM BY TYPE AND STATUS");
        string roomType = getValidatedString("Enter room type: ");
        string status = promptStatusChoice();
        if (status == "Cancel") {
            cout << "Search cancelled.\n";
            return;
        }

        bool found = false;
        printRoomHeader();
        for (const Room& room : rooms) {
            if (room.roomType == roomType && room.roomStatus == status) {
                printRoomRow(room);
                found = true;
            }
        }
        printLine('-');
        if (!found) cout << "No room found with this room type and status.\n";
    }

    void searchRoomMenu() const {
        int choice;
        do {
            printTitle("SEARCH ROOM MENU");
            cout << "1. Search by Room ID\n";
            cout << "2. Search by Room Type\n";
            cout << "3. Search by Room Status\n";
            cout << "4. Search by Room Type and Status\n";
            cout << "0. Return\n";

            choice = getValidatedInt("Choice: ", 0, 4);
            switch (choice) {
            case 1: searchRoomByID(); break;
            case 2: searchRoomByType(); break;
            case 3: searchRoomByStatus(); break;
            case 4: searchRoomByTypeAndStatus(); break;
            case 0: cout << "Returning to Room Management Menu...\n"; break;
            }
        } while (choice != 0);
    }

    void setRoomPriceByType() {
        printTitle("SET ROOM PRICE BY TYPE");
        string roomType = getValidatedString("Enter room type: ");
        double newPrice = getValidatedPrice("Enter new room price: RM ");
        bool found = false;

        for (Room& room : rooms) {
            if (room.roomType == roomType) {
                room.roomPrice = newPrice;
                found = true;
            }
        }

        if (found) cout << "Room price updated for all " << roomType << " rooms.\n";
        else cout << "No room found with this room type.\n";
    }

    void setRoomPriceMenu() {
        printTitle("SET ROOM PRICE");
        cout << "1. Set price by Room Type\n0. Cancel\n";
        int choice = getValidatedInt("Choice: ", 0, 1);
        switch (choice) {
        case 1: setRoomPriceByType(); break;
        case 0: cout << "Price update cancelled.\n"; break;
        }
    }

    string chooseRoomStatus() const {
        printTitle("SELECT ROOM STATUS");
        return promptStatusChoice();
    }

    void setRoomStatusByID(BookingModule& bookingModule);
    void setRoomStatusByType(BookingModule& bookingModule);
    void setRoomStatusMenu(BookingModule& bookingModule);

    void deleteRoomInteractive() {
        printTitle("DELETE ROOM");
        string roomID = getValidatedString("Enter Room ID to delete: ");
        int index = searchRoomIndexByID(roomID);
        if (index == -1) {
            cout << "Invalid Room ID. Room not found.\n";
            return;
        }

        if (rooms[index].roomStatus == "Occupied") {
            cout << "This room is currently occupied and cannot be deleted.\n";
            return;
        }

        cout << "\nRoom selected for deletion:\n";
        printRoomHeader();
        printRoomRow(rooms[index]);
        printLine('-');

        cout << "Are you sure you want to delete this room? (Y/N): ";
        char confirm;
        cin >> confirm;
        cin.ignore(1000, '\n');

        if (confirm == 'Y' || confirm == 'y') {
            rooms.erase(rooms.begin() + index);
            cout << "Room deleted successfully.\n";
        }
        else {
            cout << "Room deletion cancelled.\n";
        }
    }

    void setRoomDescription() {
        printTitle("SET ROOM DESCRIPTION");
        string roomID = getValidatedString("Enter Room ID: ");
        int index = searchRoomIndexByID(roomID);
        if (index == -1) {
            cout << "Invalid Room ID. Room not found.\n";
            return;
        }

        string newDescription = getValidatedString("Enter new room description: ");
        rooms[index].roomDescription = newDescription;
        cout << "Room description updated successfully.\n";
    }

    void displayRoomStatusSummary() const {
        printTitle("ROOM STATUS SUMMARY");
        int availableCount = 0;
        int unavailableCount = 0;
        int occupiedCount = 0;
        int maintenanceCount = 0;

        for (const Room& room : rooms) {
            if (room.roomStatus == "Available") availableCount++;
            else if (room.roomStatus == "Unavailable") unavailableCount++;
            else if (room.roomStatus == "Occupied") occupiedCount++;
            else if (room.roomStatus == "Under Maintenance") maintenanceCount++;
        }

        cout << left << setw(30) << "Available Rooms:" << availableCount << endl;
        cout << left << setw(30) << "Unavailable Rooms:" << unavailableCount << endl;
        cout << left << setw(30) << "Occupied Rooms:" << occupiedCount << endl;
        cout << left << setw(30) << "Under Maintenance Rooms:" << maintenanceCount << endl;
        cout << left << setw(30) << "Total Rooms:" << rooms.size() << endl;
    }

    bool roomExists(const string& roomID) const {
        return searchRoomIndexByID(roomID) != -1;
    }

    bool isAvailable(const string& roomID) const {
        int index = searchRoomIndexByID(roomID);
        return index != -1 && rooms[index].roomStatus == "Available";
    }

    string getStatus(const string& roomID) const {
        int index = searchRoomIndexByID(roomID);
        return index != -1 ? rooms[index].roomStatus : "";
    }

    double getPrice(const string& roomID) const {
        int index = searchRoomIndexByID(roomID);
        if (index == -1) return -1;
        return rooms[index].roomPrice;
    }

    string getRoomType(const string& roomID) const {
        int index = searchRoomIndexByID(roomID);
        return index != -1 ? rooms[index].roomType : "";
    }

    bool updateRoomStatusByID(const string& roomID, const string& newStatus) {
        if (!isValidRoomStatusName(newStatus)) return false;
        int index = searchRoomIndexByID(roomID);
        if (index == -1) return false;
        rooms[index].roomStatus = newStatus;
        return true;
    }


    int getTotalRoomCount() const {
        return static_cast<int>(rooms.size());
    }

    int getAvailableRoomCount() const {
        int count = 0;
        for (const Room& room : rooms) {
            if (room.roomStatus == "Available") count++;
        }
        return count;
    }

    int getOccupiedRoomCount() const {
        int count = 0;
        for (const Room& room : rooms) {
            if (room.roomStatus == "Occupied") count++;
        }
        return count;
    }

    int getUnavailableRoomCount() const {
        int count = 0;
        for (const Room& room : rooms) {
            if (room.roomStatus == "Unavailable") count++;
        }
        return count;
    }

    int getMaintenanceRoomCount() const {
        int count = 0;
        for (const Room& room : rooms) {
            if (room.roomStatus == "Under Maintenance") count++;
        }
        return count;
    }

    void loadSampleRooms() {
        Room r1 = { "R001", "Standard", 120.00, "Basic room", "Available" };
        Room r2 = { "R002", "Deluxe", 220.00, "Larger room", "Available" };
        Room r3 = { "R003", "Suite", 450.00, "Premium room", "Under Maintenance" };
        rooms.push_back(r1);
        rooms.push_back(r2);
        rooms.push_back(r3);
    }

    void roomManagementMenu(BookingModule& bookingModule);
};

// ============================================================
// BOOKING MODULE
// ============================================================

struct Booking {
    string id;
    string roomId;
    string userId;
    string checkInDate;
    int    daysStaying;
    bool   active;
};

const int MAX_BOOKINGS = 200;

bool isLeapYear(int year) {
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

int daysInMonth(int year, int month) {
    static const int days[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    if (month == 2 && isLeapYear(year)) return 29;
    return days[month - 1];
}

bool isValidDate(const string& date) {
    if (date.size() != 10) return false;
    if (date[4] != '-' || date[7] != '-') return false;
    for (size_t i = 0; i < date.size(); i++) {
        if (i == 4 || i == 7) continue;
        if (!isdigit(static_cast<unsigned char>(date[i]))) return false;
    }

    int year = stoi(date.substr(0, 4));
    int month = stoi(date.substr(5, 2));
    int day = stoi(date.substr(8, 2));

    if (month < 1 || month > 12) return false;
    if (day < 1 || day > daysInMonth(year, month)) return false;

    const int MIN_YEAR = 2000;
    const int MAX_YEAR = 2100;
    if (year < MIN_YEAR || year > MAX_YEAR) return false;

    return true;
}

struct BookingModule {
    vector<Booking> bookings;
    int             nextId;

    BookingModule() : nextId(1) {}

    int findIndexById(const string& id) {
        for (size_t i = 0; i < bookings.size(); i++) {
            if (bookings[i].id == id && bookings[i].active) return static_cast<int>(i);
        }
        return -1;
    }

    bool hasActiveBookingForRoom(const string& roomId) {
        for (size_t i = 0; i < bookings.size(); i++) {
            if (bookings[i].active && bookings[i].roomId == roomId) return true;
        }
        return false;
    }

    bool createBooking(const string& roomId, const string& userId, const string& checkInDate,
        int daysStaying, RoomModule& roomModule, UserModule& userModule) {
        if (static_cast<int>(bookings.size()) >= MAX_BOOKINGS) {
            cout << "[Error] Booking storage is full.\n";
            return false;
        }
        if (!roomModule.roomExists(roomId)) {
            cout << "[Error] Room id " << roomId << " does not exist.\n";
            return false;
        }
        if (!roomModule.isAvailable(roomId)) {
            cout << "[Error] Room id " << roomId << " is not available for booking.\n";
            return false;
        }
        if (!isValidDate(checkInDate)) {
            cout << "[Error] Invalid date format (expected YYYY-MM-DD): " << checkInDate << "\n";
            return false;
        }
        if (daysStaying <= 0) {
            cout << "[Error] Days staying must be greater than 0.\n";
            return false;
        }

        Booking b;
        b.id = "BK" + to_string(nextId++);
        b.roomId = roomId;
        b.userId = userId;
        b.checkInDate = checkInDate;
        b.daysStaying = daysStaying;
        b.active = true;

        bookings.push_back(b);
        roomModule.updateRoomStatusByID(roomId, "Occupied");
        userModule.recordBooking(userId);
        cout << "[OK] Booking created: " << b.id << " (Room " << roomId
            << ", " << daysStaying << " day(s) from " << checkInDate << ")\n";
        return true;
    }

    bool editBooking(const string& id, const string& newDate, int newDays,
        const string& callerId, Role callerRole) {
        int idx = findIndexById(id);
        if (idx == -1) {
            cout << "[Error] Booking id " << id << " not found.\n";
            return false;
        }
        bool isOwner = (bookings[idx].userId == callerId);
        bool isStaff = hasPermission(callerRole, ACTION_MANAGE_ALL_BOOKINGS);
        if (!isOwner && !isStaff) {
            cout << "[Error] You do not have permission to edit this booking.\n";
            return false;
        }
        if (!newDate.empty()) {
            if (!isValidDate(newDate)) {
                cout << "[Error] Invalid date format (expected YYYY-MM-DD): " << newDate << "\n";
                return false;
            }
            bookings[idx].checkInDate = newDate;
        }
        if (newDays > 0) bookings[idx].daysStaying = newDays;

        cout << "[OK] Booking id " << id << " updated.\n";
        return true;
    }

    bool cancelBooking(const string& id, const string& callerId, Role callerRole, RoomModule& roomModule) {
        int idx = findIndexById(id);
        if (idx == -1) {
            cout << "[Error] Booking id " << id << " not found.\n";
            return false;
        }
        bool isOwner = (bookings[idx].userId == callerId);
        bool isStaff = hasPermission(callerRole, ACTION_MANAGE_ALL_BOOKINGS);
        if (!isOwner && !isStaff) {
            cout << "[Error] You do not have permission to cancel this booking.\n";
            return false;
        }
        bookings[idx].active = false;
        if (roomModule.getStatus(bookings[idx].roomId) == "Occupied") {
            roomModule.updateRoomStatusByID(bookings[idx].roomId, "Available");
        }
        cout << "[OK] Booking id " << id << " cancelled.\n";
        return true;
    }

    bool bookingExists(const string& id) {
        return findIndexById(id) != -1;
    }

    string getOwner(const string& id) {
        int idx = findIndexById(id);
        return (idx != -1) ? bookings[idx].userId : "";
    }

    int getDays(const string& id) {
        int idx = findIndexById(id);
        return (idx != -1) ? bookings[idx].daysStaying : 0;
    }

    string getRoomId(const string& id) {
        int idx = findIndexById(id);
        return (idx != -1) ? bookings[idx].roomId : "";
    }

    string getCheckInDate(const string& id) {
        int idx = findIndexById(id);
        return (idx != -1) ? bookings[idx].checkInDate : "";
    }

    void printBookingRow(const Booking& b) {
        cout << "ID: " << b.id
            << " | Room: " << b.roomId
            << " | User: " << b.userId
            << " | Check-in: " << b.checkInDate
            << " | Days: " << b.daysStaying << "\n";
    }

    void listAllBookings() {
        cout << "\n--- All Bookings ---\n";
        bool any = false;
        for (size_t i = 0; i < bookings.size(); i++) {
            if (!bookings[i].active) continue;
            any = true;
            printBookingRow(bookings[i]);
        }
        if (!any) cout << "(No bookings yet)\n";
        cout << "--------------------\n";
    }

    void listMyBookings(const string& userId) {
        cout << "\n--- My Bookings ---\n";
        bool any = false;
        for (size_t i = 0; i < bookings.size(); i++) {
            if (!bookings[i].active) continue;
            if (bookings[i].userId != userId) continue;
            any = true;
            printBookingRow(bookings[i]);
        }
        if (!any) cout << "(You have no bookings)\n";
        cout << "-------------------\n";
    }

    int getTotalBookingsEver() const {
        return nextId - 1;
    }

 
    int getActiveBookingCount() const {
        int count = 0;
        for (const Booking& b : bookings) if (b.active) count++;
        return count;
    }

    int getCancelledBookingCount() const {
        int count = 0;
        for (const Booking& b : bookings) if (!b.active) count++;
        return count;
    }
};

void RoomModule::setRoomStatusByID(BookingModule& bookingModule) {
    printTitle("SET ROOM STATUS BY ID");
    string roomID = getValidatedString("Enter Room ID: ");
    int index = searchRoomIndexByID(roomID);
    if (index == -1) {
        cout << "Invalid Room ID. Room not found.\n";
        return;
    }

    if (bookingModule.hasActiveBookingForRoom(roomID)) {
        cout << "This room has an active booking and its status cannot be changed manually.\n";
        cout << "Cancel the booking first if the room needs to be marked Unavailable/Under Maintenance.\n";
        return;
    }

    string newStatus = chooseRoomStatus();
    if (newStatus == "Cancel") {
        cout << "Status update cancelled.\n";
        return;
    }

    rooms[index].roomStatus = newStatus;
    cout << "Room status updated successfully.\n";
}

void RoomModule::setRoomStatusByType(BookingModule& bookingModule) {
    printTitle("SET ROOM STATUS BY TYPE");
    string roomType = getValidatedString("Enter room type: ");
    string newStatus = chooseRoomStatus();
    if (newStatus == "Cancel") {
        cout << "Status update cancelled.\n";
        return;
    }

    bool found = false;
    bool skippedAny = false;
    for (Room& room : rooms) {
        if (room.roomType == roomType) {
            found = true;
            if (bookingModule.hasActiveBookingForRoom(room.roomID)) {
                skippedAny = true;
                continue;
            }
            room.roomStatus = newStatus;
        }
    }

    if (found) {
        cout << "Room status updated for all " << roomType << " rooms.\n";
        if (skippedAny) {
            cout << "Rooms with an active booking were skipped.\n";
        }
    }
    else {
        cout << "No room found with this room type.\n";
    }
}

void RoomModule::setRoomStatusMenu(BookingModule& bookingModule) {
    printTitle("SET ROOM STATUS");
    cout << "1. Set status by Room ID\n";
    cout << "2. Set status by Room Type\n";
    cout << "0. Cancel\n";

    int choice = getValidatedInt("Choice: ", 0, 2);
    switch (choice) {
    case 1: setRoomStatusByID(bookingModule); break;
    case 2: setRoomStatusByType(bookingModule); break;
    case 0: cout << "Status update cancelled.\n"; break;
    }
}

void RoomModule::roomManagementMenu(BookingModule& bookingModule) {
    int choice;
    do {
        printTitle("ROOM MANAGEMENT MODULE");
        cout << "1. Create Room\n";
        cout << "2. Delete Room\n";
        cout << "3. Search Room\n";
        cout << "4. List All Rooms\n";
        cout << "5. List Available Rooms\n";
        cout << "6. Set Room Price\n";
        cout << "7. Set Room Status\n";
        cout << "8. Set Room Description\n";
        cout << "9. Display Room Status Summary\n";
        cout << "0. Exit\n";

        choice = getValidatedInt("Choice: ", 0, 9);
        switch (choice) {
        case 1: createRoom(); break;
        case 2: deleteRoomInteractive(); break;
        case 3: searchRoomMenu(); break;
        case 4: listRooms(); break;
        case 5: listAvailableRooms(); break;
        case 6: setRoomPriceMenu(); break;
        case 7: setRoomStatusMenu(bookingModule); break;
        case 8: setRoomDescription(); break;
        case 9: displayRoomStatusSummary(); break;
        case 0: cout << "Exiting Room Management Module...\n"; break;
        }
    } while (choice != 0);
}

// ============================================================
// PAYMENT MODULE
// ============================================================

enum PaymentMethod {
    PAY_CASH = 0,
    PAY_CARD = 1,
    PAY_ONLINE = 2
};

string paymentMethodToString(PaymentMethod m) {
    switch (m) {
    case PAY_CASH:   return "Cash";
    case PAY_CARD:   return "Card";
    case PAY_ONLINE: return "Online";
    default:          return "Unknown";
    }
}

enum PaymentStatus {
    PAYMENT_PENDING = 0,
    PAYMENT_PAID = 1,
    PAYMENT_REFUNDED = 2
};

string paymentStatusToString(PaymentStatus s) {
    switch (s) {
    case PAYMENT_PENDING:  return "Pending";
    case PAYMENT_PAID:     return "Paid";
    case PAYMENT_REFUNDED: return "Refunded";
    default:                return "Unknown";
    }
}

struct Payment {
    string        id;
    string        bookingId;
    string        userId;
    double        subtotal;
    double        taxAmount;
    double        amount;
    PaymentMethod method;
    PaymentStatus status;
    bool          active;
};

const int MAX_PAYMENTS = 200;
const double SST_RATE = 0.06;

struct PaymentModule {
    vector<Payment> payments;
    int             nextId;

    PaymentModule() : nextId(1) {}

    int findIndexById(const string& id) {
        for (size_t i = 0; i < payments.size(); i++) {
            if (payments[i].id == id && payments[i].active) return static_cast<int>(i);
        }
        return -1;
    }

    int findIndexByBookingId(const string& bookingId) {
        for (size_t i = 0; i < payments.size(); i++) {
            if (payments[i].bookingId == bookingId && payments[i].active) return static_cast<int>(i);
        }
        return -1;
    }


    int findActivePaidIndexByBookingId(const string& bookingId) {
        for (size_t i = 0; i < payments.size(); i++) {
            if (payments[i].bookingId == bookingId && payments[i].active
                && payments[i].status == PAYMENT_PAID) {
                return static_cast<int>(i);
            }
        }
        return -1;
    }

    bool makePayment(const string& bookingId, const string& callerId, Role callerRole,
        PaymentMethod method, BookingModule& bookingModule, RoomModule& roomModule,
        UserModule& userModule) {
        if (static_cast<int>(payments.size()) >= MAX_PAYMENTS) {
            cout << "[Error] Payment storage is full.\n";
            return false;
        }
        if (!bookingModule.bookingExists(bookingId)) {
            cout << "[Error] Booking id " << bookingId << " does not exist.\n";
            return false;
        }
        bool isOwner = (bookingModule.getOwner(bookingId) == callerId);
        bool isStaff = hasPermission(callerRole, ACTION_MANAGE_ALL_PAYMENTS);
        if (!isOwner && !isStaff) {
            cout << "[Error] You can only pay for your own bookings.\n";
            return false;
        }
        if (findActivePaidIndexByBookingId(bookingId) != -1) {
            cout << "[Error] Booking " << bookingId << " has already been paid for.\n";
            return false;
        }

        string roomId = bookingModule.getRoomId(bookingId);
        int    days = bookingModule.getDays(bookingId);
        double subtotal = roomModule.getPrice(roomId) * days;
        double taxAmount = subtotal * SST_RATE;
        double total = subtotal + taxAmount;

        Payment p;
        p.id = "PMT" + to_string(nextId++);
        p.bookingId = bookingId;
        p.userId = bookingModule.getOwner(bookingId);
        p.subtotal = subtotal;
        p.taxAmount = taxAmount;
        p.amount = total;
        p.method = method;
        p.status = PAYMENT_PAID;
        p.active = true;

        payments.push_back(p);
        userModule.recordPayment(p.userId, total);
        cout << "[OK] Payment recorded: " << p.id << " for booking " << bookingId
            << " | Subtotal: RM" << fixed << setprecision(2) << subtotal
            << " | SST (6%): RM" << taxAmount
            << " | Total: RM" << total
            << " | Method: " << paymentMethodToString(method) << "\n";
        return true;
    }

    bool updatePaymentStatus(const string& id, PaymentStatus newStatus, Role callerRole,
        UserModule& userModule) {
        if (!hasPermission(callerRole, ACTION_MANAGE_ALL_PAYMENTS)) {
            cout << "[Error] You do not have permission to update payment status.\n";
            return false;
        }
        int idx = findIndexById(id);
        if (idx == -1) {
            cout << "[Error] Payment id " << id << " not found.\n";
            return false;
        }

        PaymentStatus oldStatus = payments[idx].status;
        if (oldStatus == newStatus) {
            cout << "[OK] Payment id " << id << " is already "
                << paymentStatusToString(newStatus) << ".\n";
            return true;
        }


        if (oldStatus == PAYMENT_PAID && newStatus != PAYMENT_PAID) {
            userModule.recordPayment(payments[idx].userId, -payments[idx].amount);
        }
        else if (oldStatus != PAYMENT_PAID && newStatus == PAYMENT_PAID) {
            userModule.recordPayment(payments[idx].userId, payments[idx].amount);
        }

        payments[idx].status = newStatus;
        cout << "[OK] Payment id " << id << " status updated to "
            << paymentStatusToString(newStatus) << ".\n";
        return true;
    }

    bool deletePayment(const string& id, Role callerRole, UserModule& userModule) {
        if (!hasPermission(callerRole, ACTION_MANAGE_ALL_PAYMENTS)) {
            cout << "[Error] You do not have permission to delete payment records.\n";
            return false;
        }
        int idx = findIndexById(id);
        if (idx == -1) {
            cout << "[Error] Payment id " << id << " not found.\n";
            return false;
        }


        if (payments[idx].status == PAYMENT_PAID) {
            userModule.recordPayment(payments[idx].userId, -payments[idx].amount);
        }

        payments[idx].active = false;
        cout << "[OK] Payment id " << id << " deleted.\n";
        return true;
    }

    void printPaymentRow(const Payment& p) {
        cout << "ID: " << p.id
            << " | Booking: " << p.bookingId
            << " | User: " << p.userId
            << " | Subtotal: RM" << fixed << setprecision(2) << p.subtotal
            << " | SST(6%): RM" << p.taxAmount
            << " | Total: RM" << p.amount
            << " | Method: " << paymentMethodToString(p.method)
            << " | Status: " << paymentStatusToString(p.status) << "\n";
    }

    void listAllPayments() {
        cout << "\n--- All Payments ---\n";
        bool any = false;
        for (size_t i = 0; i < payments.size(); i++) {
            if (!payments[i].active) continue;
            any = true;
            printPaymentRow(payments[i]);
        }
        if (!any) cout << "(No payments yet)\n";
        cout << "--------------------\n";
    }

    void listMyPayments(const string& userId) {
        cout << "\n--- My Payments ---\n";
        bool any = false;
        for (size_t i = 0; i < payments.size(); i++) {
            if (!payments[i].active) continue;
            if (payments[i].userId != userId) continue;
            any = true;
            printPaymentRow(payments[i]);
        }
        if (!any) cout << "(You have no payment records)\n";
        cout << "-------------------\n";
    }

    double getTotalIncome() const {
        double total = 0.0;
        for (const Payment& p : payments) {
            if (p.active && p.status == PAYMENT_PAID) total += p.amount;
        }
        return total;
    }


    double getTotalRefunded() const {
        double total = 0.0;
        for (const Payment& p : payments) {
            if (p.active && p.status == PAYMENT_REFUNDED) total += p.amount;
        }
        return total;
    }


    double getTotalPending() const {
        double total = 0.0;
        for (const Payment& p : payments) {
            if (p.active && p.status == PAYMENT_PENDING) total += p.amount;
        }
        return total;
    }

    int getTotalPaymentCount() const {
        int count = 0;
        for (const Payment& p : payments) if (p.active) count++;
        return count;
    }

    int getPendingPaymentCount() const {
        int count = 0;
        for (const Payment& p : payments) if (p.active && p.status == PAYMENT_PENDING) count++;
        return count;
    }

    int getRefundedPaymentCount() const {
        int count = 0;
        for (const Payment& p : payments) if (p.active && p.status == PAYMENT_REFUNDED) count++;
        return count;
    }
};




bool exportPaymentReceipt(const string& paymentId, PaymentModule& paymentModule,
    BookingModule& bookingModule, RoomModule& roomModule,
    UserModule& userModule) {
    int payIdx = paymentModule.findIndexById(paymentId);
    if (payIdx == -1) {
        cout << "[Error] Payment id " << paymentId << " not found. Cannot export receipt.\n";
        return false;
    }
    Payment& p = paymentModule.payments[payIdx];

    string roomId = bookingModule.getRoomId(p.bookingId);
    string roomType = roomModule.getRoomType(roomId);
    string checkInDate = bookingModule.getCheckInDate(p.bookingId);
    int    nights = bookingModule.getDays(p.bookingId);
    double ratePerNight = roomModule.getPrice(roomId);

    int userIdx = userModule.findIndexById(p.userId);
    string guestName = (userIdx != -1) ? userModule.users[userIdx].username : p.userId;
    string guestEmail = (userIdx != -1) ? userModule.users[userIdx].email : "";

    string filename = paymentId + "_receipt.txt";
    ofstream outFile(filename);
    if (!outFile.is_open()) {
        cout << "[Error] Could not create receipt file: " << filename << "\n";
        return false;
    }

    outFile << "========================================\n";
    outFile << "           HOTEL BOOKING RECEIPT\n";
    outFile << "========================================\n";
    outFile << "Receipt No     : " << p.id << "\n";
    outFile << "----------------------------------------\n";
    outFile << "Guest Name     : " << guestName << "\n";
    outFile << "Email          : " << guestEmail << "\n";
    outFile << "----------------------------------------\n";
    outFile << "Booking ID     : " << p.bookingId << "\n";
    outFile << "Room ID        : " << roomId << "\n";
    outFile << "Room Type      : " << roomType << "\n";
    outFile << "Check-in Date  : " << checkInDate << "\n";
    outFile << "Nights Staying : " << nights << "\n";
    outFile << "Rate / Night   : RM" << fixed << setprecision(2) << ratePerNight << "\n";
    outFile << "----------------------------------------\n";
    outFile << "Subtotal       : RM" << fixed << setprecision(2) << p.subtotal << "\n";
    outFile << "SST (6%)       : RM" << fixed << setprecision(2) << p.taxAmount << "\n";
    outFile << "TOTAL PAID     : RM" << fixed << setprecision(2) << p.amount << "\n";
    outFile << "----------------------------------------\n";
    outFile << "Payment Method : " << paymentMethodToString(p.method) << "\n";
    outFile << "Payment Status : " << paymentStatusToString(p.status) << "\n";
    outFile << "========================================\n";
    outFile << "      Thank you for staying with us!\n";
    outFile << "========================================\n";

    outFile.close();
    cout << "[OK] Receipt exported: " << filename << "\n";
    return true;
}

// ============================================================
// GENERAL UI HELPERS
// ============================================================

void clearScreen() {
    cout << "\033[2J\033[1;1H";
}

void printBanner() {
    cout << R"(
888   |      e      ,d88~~\ 888~~\  888 888b    |       888   |            d8             888 
888___|     d8b     8888    888   | 888 |Y88b   |       888___|  e88~-_  _d88__  e88~~8e  888 
888   |    /Y88b    `Y88b   888 _/  888 | Y88b  |       888   | d888   i  888   d888  88b 888 
888   |   /  Y88b    `Y88b, 888  \  888 |  Y88b |       888   | 8888   |  888   8888__888 888 
888   |  /____Y88b     8888 888   | 888 |   Y88b|       888   | Y888   '  888   Y888    , 888 
888   | /      Y88b \__88P' 888__/  888 |    Y888       888   |  "88_-~   "88_/  "88___/  888 
)" << endl;
}

string prompt(const string& label) {
    cout << label;
    string value;
    getline(cin, value);
    if (!cin) {
        cout << "\n[Input closed] Exiting.\n";
        exit(0);
    }
    return value;
}

void pause() {
    cout << "\nPress Enter to continue...";
    string dummy;
    getline(cin, dummy);
}

bool confirmYN(const string& label) {
    string answer = prompt(label);
    return answer == "y" || answer == "Y";
}

int promptInt(const string& label) {
    while (true) {
        string s = prompt(label);
        try {
            size_t pos;
            int value = stoi(s, &pos);
            if (pos == s.size()) return value;
        }
        catch (...) {}
        cout << "[Error] Please enter a valid whole number.\n";
    }
}

int promptIntOptional(const string& label) {
    string s = prompt(label);
    if (s.empty()) return 0;
    try {
        size_t pos;
        int value = stoi(s, &pos);
        if (pos == s.size()) return value;
    }
    catch (...) {}
    cout << "[Error] Invalid number, keeping unchanged.\n";
    return 0;
}

Role promptRole(Role currentRole) {
    cout << "1. Admin\n";
    cout << "2. Employee\n";
    cout << "3. Customer\n";
    string choice = prompt("Choose a role (leave blank to keep current): ");
    if (choice.empty()) return currentRole;
    if (choice == "1") return ROLE_ADMIN;
    if (choice == "2") return ROLE_EMPLOYEE;
    if (choice == "3") return ROLE_CUSTOMER;
    cout << "[Error] Invalid choice, keeping current role (" << roleToString(currentRole) << ").\n";
    return currentRole;
}

Role promptNewAccountRole() {
    cout << "1. Admin\n";
    cout << "2. Employee\n";
    cout << "3. Customer\n";
    string choice = prompt("Choose a role: ");
    if (choice == "1") return ROLE_ADMIN;
    if (choice == "2") return ROLE_EMPLOYEE;
    return ROLE_CUSTOMER;
}

PaymentMethod promptPaymentMethod() {
    cout << "1. Cash\n";
    cout << "2. Card\n";
    cout << "3. Online\n";
    string choice = prompt("Choose payment method: ");
    if (choice == "2") return PAY_CARD;
    if (choice == "3") return PAY_ONLINE;
    return PAY_CASH;
}

PaymentStatus promptPaymentStatus() {
    cout << "1. Pending\n";
    cout << "2. Paid\n";
    cout << "3. Refunded\n";
    string choice = prompt("Choose new status: ");
    if (choice == "1") return PAYMENT_PENDING;
    if (choice == "3") return PAYMENT_REFUNDED;
    return PAYMENT_PAID;
}

// ============================================================
// USER HANDLERS
// ============================================================

void handleRegister(UserModule& module) {
    cout << "\n=== Register New Account ===\n";
    string username = prompt("Username: ");
    string password = prompt("Password: ");
    string email = prompt("Email: ");
    module.createAccount(username, password, email);
    pause();
}

void handleAdminCreateAccount(UserModule& module) {
    cout << "\n=== Create Account (Admin) ===\n";
    string username = prompt("Username: ");
    string password = prompt("Password: ");
    string email = prompt("Email: ");
    Role role = promptNewAccountRole();
    module.createAccountWithRole(username, password, email, role);
    pause();
}

string handleEditOtherAccount(UserModule& module) {
    cout << "\n=== Edit Another Account ===\n";
    module.listAccounts();
    string targetId = prompt("Enter the ID of the account to edit: ");
    int idx = module.findIndexById(targetId);
    if (idx == -1 || !module.users[idx].active) {
        cout << "[Error] Account id " << targetId << " not found.\n";
        pause();
        return"";
    }
    Role currentRole = module.users[idx].role;

    cout << "(Leave a field blank to keep it unchanged)\n";
    string newUsername = prompt("New username: ");
    string newPassword = prompt("New password: ");
    string newEmail = prompt("New email: ");
    cout << "New role:\n";
    Role newRole = promptRole(currentRole);
    bool success = module.editAccount(targetId, newUsername, newPassword, newEmail, newRole);
    pause();
    if (success && newRole != currentRole) {
        return targetId;
    }
    return "";
}

string handleDeleteOtherAccount(UserModule& module) {
    cout << "\n=== Delete Another Account ===\n";
    module.listAccounts();
    string result = "";
    string targetId = prompt("Enter the ID of the account to delete: ");
    if (confirmYN("Are you sure? (y/n): ")) {
        module.deleteAccount(targetId);
        result = targetId;
    }
    else {
        cout << "Canceled decision";
    }
    pause();
    return result;
}

// ============================================================
// BOOKING HANDLERS
// ============================================================

void handleCreateBooking(BookingModule& bookingModule, RoomModule& roomModule,
    UserModule& userModule, const string& userId) {
    cout << "\n=== Book a Room ===\n";
    roomModule.listAvailableRooms();
    string roomId = prompt("Enter Room ID to book: ");
    string date = prompt("Check-in date (YYYY-MM-DD): ");
    int days = promptInt("Number of days staying: ");
    bookingModule.createBooking(roomId, userId, date, days, roomModule, userModule);
    pause();
}

void handleEditOwnBooking(BookingModule& bookingModule, const string& userId, Role role) {
    cout << "\n=== Edit My Booking ===\n";
    bookingModule.listMyBookings(userId);
    string id = prompt("Enter Booking ID to edit: ");
    cout << "(Leave a field blank/0 to keep it unchanged)\n";
    string newDate = prompt("New check-in date (YYYY-MM-DD): ");
    int newDays = promptIntOptional("New number of days: ");
    bookingModule.editBooking(id, newDate, newDays, userId, role);
    pause();
}

void handleCancelOwnBooking(BookingModule& bookingModule, RoomModule& roomModule,
    const string& userId, Role role) {
    cout << "\n=== Cancel My Booking ===\n";
    bookingModule.listMyBookings(userId);
    string id = prompt("Enter Booking ID to cancel: ");
    if (confirmYN("Are you sure? (y/n): ")) {
        bookingModule.cancelBooking(id, userId, role, roomModule);
    }
    else {
        cout << "Canceled decision";
    }
    pause();
}

void handleManageAllBookings(BookingModule& bookingModule, RoomModule& roomModule,
    const string& callerId, Role callerRole) {
    bool inMenu = true;
    while (inMenu) {
        cout << "\n=== Manage All Bookings ===\n";
        cout << "1. List all bookings\n";
        cout << "2. Edit a booking\n";
        cout << "3. Cancel a booking\n";
        cout << "4. Back\n";
        string choice = prompt("Choose an option: ");
        if (choice == "1") {
            bookingModule.listAllBookings();
            pause();
        }
        else if (choice == "2") {
            bookingModule.listAllBookings();
            string id = prompt("Enter Booking ID to edit: ");
            cout << "(Leave a field blank/0 to keep it unchanged)\n";
            string newDate = prompt("New check-in date (YYYY-MM-DD): ");
            int newDays = promptIntOptional("New number of days: ");
            bookingModule.editBooking(id, newDate, newDays, callerId, callerRole);
            pause();
        }
        else if (choice == "3") {
            bookingModule.listAllBookings();
            string id = prompt("Enter Booking ID to cancel: ");
            if (confirmYN("Are you sure? (y/n): ")) {
                bookingModule.cancelBooking(id, callerId, callerRole, roomModule);
            }
            else {
                cout << "Canceled decision";
            }
            pause();
        }
        else if (choice == "4") {
            inMenu = false;
        }
        else {
            cout << "Invalid option.\n";
        }
    }
}

// ============================================================
// PAYMENT HANDLERS
// ============================================================

void handleMakePayment(PaymentModule& paymentModule, BookingModule& bookingModule,
    RoomModule& roomModule, UserModule& userModule,
    const string& userId, Role role) {
    cout << "\n=== Make Payment ===\n";
    bookingModule.listMyBookings(userId);
    string bookingId = prompt("Enter Booking ID to pay for: ");
    cout << "Payment method:\n";
    PaymentMethod method = promptPaymentMethod();
    bool success = paymentModule.makePayment(bookingId, userId, role, method, bookingModule, roomModule, userModule);
    if (success) {
        int payIdx = paymentModule.findActivePaidIndexByBookingId(bookingId);
        if (payIdx != -1) {
            string paymentId = paymentModule.payments[payIdx].id;
            exportPaymentReceipt(paymentId, paymentModule, bookingModule, roomModule, userModule);
        }
    }
    pause();
}

void handleViewOwnPayments(PaymentModule& paymentModule, const string& userId) {
    paymentModule.listMyPayments(userId);
    pause();
}

void handleManagePayments(PaymentModule& paymentModule, UserModule& userModule, Role callerRole) {
    bool inMenu = true;
    while (inMenu) {
        cout << "\n=== Manage Payments ===\n";
        cout << "1. List all payments\n";
        cout << "2. Update payment status\n";
        cout << "3. Delete payment record\n";
        cout << "4. Back\n";
        string choice = prompt("Choose an option: ");
        if (choice == "1") {
            paymentModule.listAllPayments();
            pause();
        }
        else if (choice == "2") {
            paymentModule.listAllPayments();
            string id = prompt("Enter Payment ID to update: ");
            cout << "New status:\n";
            PaymentStatus newStatus = promptPaymentStatus();
            paymentModule.updatePaymentStatus(id, newStatus, callerRole, userModule);
            pause();
        }
        else if (choice == "3") {
            paymentModule.listAllPayments();
            string id = prompt("Enter Payment ID to delete: ");
            if (confirmYN("Are you sure? (y/n): ")) { paymentModule.deletePayment(id, callerRole, userModule); }
            else {
                cout << "Canceled decision";
            }
            pause();
        }
        else if (choice == "4") {
            inMenu = false;
        }
        else {
            cout << "Invalid option.\n";
        }
    }
}

// ============================================================
// ADMIN REPORTS / DASHBOARD
// ============================================================

void handleAdminDashboard(RoomModule& roomModule, BookingModule& bookingModule,
    PaymentModule& paymentModule, UserModule& userModule) {
    printTitle("ADMIN DASHBOARD - SYSTEM REPORTS");

    cout << "\n--- Room Statistics ---\n";
    cout << left << setw(30) << "Total Rooms:" << roomModule.getTotalRoomCount() << endl;
    cout << left << setw(30) << "Available Rooms:" << roomModule.getAvailableRoomCount() << endl;
    cout << left << setw(30) << "Occupied Rooms:" << roomModule.getOccupiedRoomCount() << endl;
    cout << left << setw(30) << "Unavailable Rooms:" << roomModule.getUnavailableRoomCount() << endl;
    cout << left << setw(30) << "Under Maintenance:" << roomModule.getMaintenanceRoomCount() << endl;

    cout << "\n--- Booking Statistics ---\n";
    cout << left << setw(30) << "Total Bookings (all-time):" << bookingModule.getTotalBookingsEver() << endl;
    cout << left << setw(30) << "Active Bookings:" << bookingModule.getActiveBookingCount() << endl;
    cout << left << setw(30) << "Cancelled Bookings:" << bookingModule.getCancelledBookingCount() << endl;

    cout << "\n--- User Statistics ---\n";
    cout << left << setw(30) << "Active User Accounts:" << userModule.getActiveUserCount() << endl;
    cout << left << setw(30) << "  - Admins:" << userModule.getActiveUserCountByRole(ROLE_ADMIN) << endl;
    cout << left << setw(30) << "  - Employees:" << userModule.getActiveUserCountByRole(ROLE_EMPLOYEE) << endl;
    cout << left << setw(30) << "  - Customers:" << userModule.getActiveUserCountByRole(ROLE_CUSTOMER) << endl;

    cout << "\n--- Payment / Income Statistics ---\n";
    cout << left << setw(30) << "Total Payment Records:" << paymentModule.getTotalPaymentCount() << endl;
    cout << left << setw(30) << "Pending Payments:" << paymentModule.getPendingPaymentCount() << endl;
    cout << left << setw(30) << "Refunded Payments:" << paymentModule.getRefundedPaymentCount() << endl;
    cout << left << setw(30) << "Total Income (RM):"
        << fixed << setprecision(2) << paymentModule.getTotalIncome() << endl;
    cout << left << setw(30) << "Total Refunded (RM):"
        << fixed << setprecision(2) << paymentModule.getTotalRefunded() << endl;
    cout << left << setw(30) << "Pending Amount (RM):"
        << fixed << setprecision(2) << paymentModule.getTotalPending() << endl;
    cout << left << setw(30) << "Net Income (RM):"
        << fixed << setprecision(2)
        << (paymentModule.getTotalIncome() - paymentModule.getTotalRefunded()) << endl;

    printLine('=');
    pause();
}

// ============================================================
// CATEGORY SUB-MENUS
// ============================================================

bool handleAccountManagementMenu(UserModule& userModule, const string& userId, Role callerRole) {
    bool canViewAll = hasPermission(callerRole, ACTION_VIEW_ALL);
    bool canCreateOthers = hasPermission(callerRole, ACTION_CREATE_FOR_OTHERS);
    bool canEditOthers = hasPermission(callerRole, ACTION_EDIT_OTHERS);
    bool canDeleteOthers = hasPermission(callerRole, ACTION_DELETE_OTHERS);

    bool accountStillActive = true;
    bool inMenu = true;
    while (inMenu) {
        cout << "\n=== Account Management ===\n";
        cout << (canViewAll ? "1. View all User Info\n" : "1. View my info\n");
        cout << "2. Edit my account\n";
        cout << "3. Delete my account\n";
        if (canCreateOthers) cout << "4. Create account (choose role)\n";
        if (canEditOthers)   cout << "5. Edit another account\n";
        if (canDeleteOthers) cout << "6. Delete another account\n";
        cout << "7. Back\n";

        string choice = prompt("Choose an option: ");

        if (choice == "1") {
            userModule.viewInfo(userId);
            pause();
        }
        else if (choice == "2") {
            cout << "\n(Leave a field blank to keep it unchanged)\n";
            string newUsername = prompt("New username: ");
            string newPassword = prompt("New password: ");
            string newEmail = prompt("New email: ");
            userModule.editOwnAccount(userId, newUsername, newPassword, newEmail);
            pause();
        }
        else if (choice == "3") {
            if (confirmYN("Are you sure? (y/n): ")) {
                userModule.deleteAccount(userId);
                accountStillActive = false;
                inMenu = false;
                pause();
            }
            else {
                cout << "Canceled decision";
                pause();
            }
        }
        else if (canCreateOthers && choice == "4") {
            handleAdminCreateAccount(userModule);
        }
        else if (canEditOthers && choice == "5") {
            
            string changedId = handleEditOtherAccount(userModule);
    if (!changedId.empty() && changedId == userId) {
        cout << "\n[Notice] Your role has changed. Logging you out so your menu can refresh...\n";
        pause();
        accountStillActive = false;
        inMenu = false;
    }
            
        }
        else if (canDeleteOthers && choice == "6") {
            string theid = handleDeleteOtherAccount(userModule);
            if ( theid == userId){
                accountStillActive = false;
                inMenu = false;
            }

        }
        else if (choice == "7") {
            inMenu = false;
        }
        else {
            cout << "Invalid option.\n";
        }
    }
    return accountStillActive;
}

void handleBookingManagementMenu(BookingModule& bookingModule, RoomModule& roomModule,
    UserModule& userModule, const string& userId, Role callerRole) {
    bool canManageAllBookings = hasPermission(callerRole, ACTION_MANAGE_ALL_BOOKINGS);

    bool inMenu = true;
    while (inMenu) {
        cout << "\n=== Booking Management ===\n";
        cout << "1. Book a Room\n";
        cout << "2. My Bookings (view/edit/cancel)\n";
        if (canManageAllBookings) cout << "3. Manage All Bookings\n";
        cout << "4. Back\n";

        string choice = prompt("Choose an option: ");

        if (choice == "1") {
            handleCreateBooking(bookingModule, roomModule, userModule, userId);
        }
        else if (choice == "2") {
            bool inSub = true;
            while (inSub) {
                cout << "\n=== My Bookings ===\n";
                cout << "1. View my bookings\n";
                cout << "2. Edit a booking\n";
                cout << "3. Cancel a booking\n";
                cout << "4. Back\n";
                string sub = prompt("Choose an option: ");
                if (sub == "1") { bookingModule.listMyBookings(userId); pause(); }
                else if (sub == "2") handleEditOwnBooking(bookingModule, userId, callerRole);
                else if (sub == "3") handleCancelOwnBooking(bookingModule, roomModule, userId, callerRole);
                else if (sub == "4") inSub = false;
                else cout << "Invalid option.\n";
            }
        }
        else if (canManageAllBookings && choice == "3") {
            handleManageAllBookings(bookingModule, roomModule, userId, callerRole);
        }
        else if (choice == "4") {
            inMenu = false;
        }
        else {
            cout << "Invalid option.\n";
        }
    }
}

void handlePaymentManagementMenu(PaymentModule& paymentModule, BookingModule& bookingModule,
    RoomModule& roomModule, UserModule& userModule,
    const string& userId, Role callerRole) {
    bool canManageAllPayments = hasPermission(callerRole, ACTION_MANAGE_ALL_PAYMENTS);

    bool inMenu = true;
    while (inMenu) {
        cout << "\n=== Payment Management ===\n";
        cout << "1. Make Payment\n";
        cout << "2. My Payments\n";
        cout << "3. Export Receipt (.txt)\n";
        if (canManageAllPayments) cout << "4. Manage Payments\n";
        cout << "5. Back\n";

        string choice = prompt("Choose an option: ");

        if (choice == "1") {
            handleMakePayment(paymentModule, bookingModule, roomModule, userModule, userId, callerRole);
        }
        else if (choice == "2") {
            handleViewOwnPayments(paymentModule, userId);
        }
        else if (choice == "3") {
            if (!canManageAllPayments) paymentModule.listMyPayments(userId);
            else paymentModule.listAllPayments();
            string paymentId = prompt("Enter Payment ID to export: ");
            int idx = paymentModule.findIndexById(paymentId);
            if (idx == -1) {
                cout << "[Error] Payment id " << paymentId << " not found.\n";
            }
            else if (!canManageAllPayments && paymentModule.payments[idx].userId != userId) {
                cout << "[Error] You can only export receipts for your own payments.\n";
            }
            else {
                exportPaymentReceipt(paymentId, paymentModule, bookingModule, roomModule, userModule);
            }
            pause();
        }
        else if (canManageAllPayments && choice == "4") {
            handleManagePayments(paymentModule, userModule, callerRole);
        }
        else if (choice == "5") {
            inMenu = false;
        }
        else {
            cout << "Invalid option.\n";
        }
    }
}

// ============================================================
// LOGGED-IN MENU
// ============================================================

void handleLoggedInMenu(UserModule& userModule, RoomModule& roomModule,
    BookingModule& bookingModule, PaymentModule& paymentModule,
    const string& userId) {
    Role callerRole = userModule.getRole(userId);
    bool canManageRooms = hasPermission(callerRole, ACTION_MANAGE_ROOMS);
    bool canViewReports = hasPermission(callerRole, ACTION_VIEW_REPORTS);

    bool loggedIn = true;
    while (loggedIn) {
        int n = 0;
        int accountOpt = ++n;
        int roomsOpt = canManageRooms ? ++n : -1;
        int bookingOpt = ++n;
        int paymentOpt = ++n;
        int reportsOpt = canViewReports ? ++n : -1;
        int logoutOpt = ++n;

        cout << "\n=== Main Menu ===\n";
        cout << accountOpt << ". Account Management\n";
        if (canManageRooms) cout << roomsOpt << ". Room Management\n";
        cout << bookingOpt << ". Booking Management\n";
        cout << paymentOpt << ". Payment Management\n";
        if (canViewReports) cout << reportsOpt << ". Admin Dashboard (Reports)\n";
        cout << logoutOpt << ". Logout\n";

        int choice = promptInt("Choose an option: ");

        if (choice == accountOpt) {
            bool stillActive = handleAccountManagementMenu(userModule, userId, callerRole);
            if (!stillActive) {
                loggedIn = false;
            }
        }
        else if (canManageRooms && choice == roomsOpt) {
            roomModule.roomManagementMenu(bookingModule);
        }
        else if (choice == bookingOpt) {
            handleBookingManagementMenu(bookingModule, roomModule, userModule, userId, callerRole);
        }
        else if (choice == paymentOpt) {
            handlePaymentManagementMenu(paymentModule, bookingModule, roomModule, userModule, userId, callerRole);
        }
        else if (canViewReports && choice == reportsOpt) {
            handleAdminDashboard(roomModule, bookingModule, paymentModule, userModule);
        }
        else if (choice == logoutOpt) {
            cout << "Logged out.\n";
            loggedIn = false;
        }
        else {
            cout << "Invalid option.\n";
        }
    }
}

void handleLogin(UserModule& userModule, RoomModule& roomModule,
    BookingModule& bookingModule, PaymentModule& paymentModule) {
    const int MAX_ATTEMPTS = 3;
    for (int attempt = 1; attempt <= MAX_ATTEMPTS; attempt++) {
        cout << "\n=== Login (attempt " << attempt << "/" << MAX_ATTEMPTS << ") ===\n";
        string username = prompt("Username: ");
        string password = prompt("Password: ");

        string userId = userModule.loginGetId(username, password);
        if (!userId.empty()) {
            handleLoggedInMenu(userModule, roomModule, bookingModule, paymentModule, userId);
            return;
        }

        if (attempt < MAX_ATTEMPTS) {
            cout << "Try again.\n";
        }
    }
    cout << "\nToo many failed attempts. Returning to main menu.\n";
    pause();
}

int main() {
    UserModule    userModule;
    RoomModule    roomModule;
    BookingModule bookingModule;
    PaymentModule paymentModule;

    userModule.createAccountWithRole("admin", "adminPass", "admin@example.com", ROLE_ADMIN);
    userModule.createAccountWithRole("staff1", "staffPass", "staff1@example.com", ROLE_EMPLOYEE);
    roomModule.loadSampleRooms();

    clearScreen();
    printBanner();

    bool running = true;
    while (running) {
        cout << "\n===== Welcome =====\n";
        cout << "1. Register\n";
        cout << "2. Login\n";
        cout << "3. Exit\n";
        string choice = prompt("Choose an option: ");

        if (choice == "1") {
            handleRegister(userModule);
        }
        else if (choice == "2") {
            handleLogin(userModule, roomModule, bookingModule, paymentModule);
        }
        else if (choice == "3") {
            cout << "Goodbye!\n";
            running = false;
        }
        else {
            cout << "Invalid option, please try again.\n";
        }
    }

    return 0;
}
