#include <iostream>
#include <string>
#include <vector>
#include <memory>
//#include <pqxx/pqxx> // PostgreSQL C++ Connection Library

// --- GUI Library Headers (Dear ImGui / GLFW / OpenGL) ---
// #include "imgui.h"
// #include "imgui_impl_glfw.h"
// #include "imgui_impl_opengl3.h"
// #include <GLFW/glfw3.h>

using namespace std;

// Forward declarations
class Product;
class Order;

// ==========================================
// 1. DATABASE MANAGER 
// ==========================================
class DatabaseManager {
public:
    // Saves a new user into the real database table[cite: 2, 10]
    static void insertUser(const string& role, int id, const string& name, const string& email, const string& password, int num) {
        // TO DO : Implement this function
        // Write your libpqxx INSERT code here
    }

    // Checks user credentials against the real database[cite: 2, 10]
    static bool authenticateQuery(const string& role, int id, const string& name, const string& password) {
        // TO DO : Implement this function
        // Write your libpqxx SELECT query here
        return true; 
    }
};

// ==========================================
// 2. USER HIERARCHY (OOP & GUI Polymorphism)
// ==========================================

class Authentication {
public:
    virtual bool login(int id, const string& name, const string& password) = 0;
    virtual void signUp(int id, const string& name, const string& password, const string& email, int num) = 0;
    virtual ~Authentication() {}
};

class User : public Authentication {
protected:
    int id;
    string name;
    string email;
    string password;
    int phoneNumber;
    string role;

public:
    User(int id, string name, string email, string password, int num, string role)
        : id(id), name(name), email(email), password(password), phoneNumber(num), role(role) {
    }

    string getName() const {
        return name;
    }

    string getRole() const {
        return role;
    }

    int getId() const {
        return id;
    }

    // Replaced text menu with graphical window renderer (Polymorphism)
    virtual void renderGUI() = 0;
    virtual ~User() {}
};

class Manager : public User {
public:
    Manager(int id, string name, string email, string password, int num)
        : User(id, name, email, password, num, "Manager") {
        // TO DO : Implement this function
    }

    bool login(int id, const string& name, const string& password) override {
        // TO DO : Implement this function
        return DatabaseManager::authenticateQuery("manager", id, name, password);
    }

    void signUp(int id, const string& name, const string& password, const string& email, int num) override {
        // TO DO : Implement this function
        DatabaseManager::insertUser("Manager", id, name, email, password, num);
    }
    
    void renderGUI() override {
        // TO DO : Implement this function
        // Draw Manager Graphical Dashboard (e.g., ImGui::Begin("Manager Dashboard"), buttons for tables, products, employees)[cite: 2]
    }
};

class Employee : public User {
public:
    Employee(int id, string name, string email, string password, int num)
        : User(id, name, email, password, num, "Employee") {
        // TO DO : Implement this function
    }

    bool login(int id, const string& name, const string& password) override {
        // TO DO : Implement this function
        return DatabaseManager::authenticateQuery("employee", id, name, password);
    }

    void signUp(int id, const string& name, const string& password, const string& email, int num) override {
        // TO DO : Implement this function
        DatabaseManager::insertUser("Employee", id, name, email, password, num);
    }

    void renderGUI() override {
        // TO DO : Implement this function
        // Draw Employee Graphical Dashboard (e.g., buttons to view products/customers, verify orders)[cite: 2]
    }
};

class Customer : public User {
public:
    Customer(int id, string name, string email, string password, int num)
        : User(id, name, email, password, num, "Customer") {
        // TO DO : Implement this function
    }

    bool login(int id, const string& name, const string& password) override {
        // TO DO : Implement this function
        return DatabaseManager::authenticateQuery("customer", id, name, password);
    }

    void signUp(int id, const string& name, const string& password, const string& email, int num) override {
        // TO DO : Implement this function
        DatabaseManager::insertUser("Customer", id, name, email, password, num);
    }

    void renderGUI() override {
        // TO DO : Implement this function
        // Draw Customer Graphical Dashboard (e.g., view product catalog, place orders)[cite: 2]
    }
};

// ==========================================
// 3. USER FACTORY (Creates Users)
// ==========================================
class UserFactory {
public:
    static shared_ptr<User> createUser(const string& role, int id, const string& name, const string& email, const string& password, int num) {
        // TO DO : Implement this function
        if (role == "Manager") {
            return make_shared<Manager>(id, name, email, password, num);
        } 
        else if (role == "Employee") {
            return make_shared<Employee>(id, name, email, password, num);
        } 
        else if (role == "Customer") {
            return make_shared<Customer>(id, name, email, password, num);
        }
        return nullptr;
    }
};

// ==========================================
// 4. GUI APPLICATION RUNNER (Replaces Terminal Loop)
// ==========================================
class RunGUI {
private:
    shared_ptr<User> loggedInUser = nullptr;
    bool isLoggedIn = false;

public:
    void renderMainGUIWindow() {
        // TO DO : Implement this function
        // Main GUI frame loop: manages Login/Sign-up text inputs, dropdowns, and switches to loggedInUser->renderGUI() upon successful authentication[cite: 2]
    }
};

// ==========================================
// 5. MAIN GRAPHICAL PROGRAM
// ==========================================
int main() {
    // TO DO : Implement this function
    // Initialize GLFW window, OpenGL context, and Dear ImGui context here, then run the render loop using RunGUI.
    
    RunGUI guiApp;
    cout << "Electronic Store GUI Application Initialized Skeleton.\n";
    return 0;
}