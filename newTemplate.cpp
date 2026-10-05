#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <pqxx/pqxx> // PostgreSQL C++ Connection Library

// --- GUI Library Headers (Dear ImGui / GLFW / OpenGL) ---
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>

using namespace std;

// Forward declarations
class Product{private:
    int productId;
    string name;
    double price;
    int stockQuantity;

public:
    Product(int id, string productName, double productPrice, int quantity)
        : productId(id), name(productName), price(productPrice), stockQuantity(quantity) {}

    int getProductId() const { return productId; }
    string getName() const { return name; }
    double getPrice() const { return price; }
    int getStockQuantity() const { return stockQuantity; }

    void updateStock(int quantity) { stockQuantity += quantity; }
    bool checkAvailability(int requestedQty) const { return stockQuantity >= requestedQty; }
};
class Order{private:
    int orderId;
    int customerId;
    vector<pair<int, int>> productsList; // تخزين (ProductId, Quantity)
    double totalAmount;
    string status; // Pending, Completed, Cancelled

public:
    Order(int id, int custId) 
        : orderId(id), customerId(custId), totalAmount(0.0), status("Pending") {}

    void addProduct(int productId, int quantity, double price) {
        productsList.push_back({productId, quantity});
        totalAmount += (price * quantity);
    }

    double getTotalAmount() const {
        return totalAmount;
    }

    string getStatus() const {
        return status;
    }

    void setStatus(string newStatus) {
        status = newStatus;
    }

    void processTransaction() {
        if (totalAmount > 0) {
            status = "Completed";
        } else {
            status = "Failed";
        }
    }
};

// ==========================================
// 1. DATABASE MANAGER 
// ==========================================
class DatabaseManager {
public:
    static pqxx::connection connectDB() {
        return pqxx::connection("dbname=store user=postgres password=omaryoussri258 host=127.0.0.1 port=5432");
    }
    
    // Saves a new user into the real database table[cite: 2, 10]
    static void insertUser(const string& role, int id, const string& name, const string& email, const string& password, int num) {
        try{
            pqxx::connection C =connectDB();
            pqxx::work W(C);

            string tableName = "";
            string idColumn  = "";

            if(role == "Manager" || role == "manager"){
                tableName = "managers";
                idColumn  = "m_id";
            }
            else if(role == "Employee" || role == "employee"){
                tableName = "employees";
                idColumn  = "employee_id";
            }
            else if(role == "Customer" || role == "customer"){
                tableName = "customers";
                idColumn  = "customer_id";
            }
            string query = "INSERT INTO " + tableName + " (" + idColumn + ", name, email, password, num) VALUES (" +
                            to_string(id) + ", " +
                            W.quote(name) + ", " +
                            W.quote(email) + ", " +
                            W.quote(password) + ", " +
                            W.quote(to_string(num)) + ");";
            W.exec(query);
            W.commit();
            cout << "\n[Database Success]: User added successfully to " << tableName << "!\n";

        }catch(const exception& e){
          cerr << "\n[Database Error]: " << e.what() << endl;  
        }
    }

    // Checks user credentials against the real database[cite: 2, 10]
    static bool authenticateQuery(const string& role, int id, const string& name, const string& password) {
        try{
            pqxx::connection C = connectDB();
            pqxx::nontransaction N(C);

            string tableName = "";
            string idColumn  = "";
            if (role == "Manager" || role == "manager") { 
                tableName = "managers"; 
                idColumn = "m_id"; 
            }else if (role == "Employee" || role == "employee"){ 
                tableName = "employees"; 
                idColumn = "employee_id"; 
            }
            else if (role == "Customer" || role == "customer"){ 
                tableName = "customers"; 
                idColumn = "customer_id"; 
            }

            string query = "SELECT * FROM " + tableName + " WHERE " + idColumn + " = " + to_string(id) + 
            " AND name = " + N.quote(name) + 
            " AND password = " + N.quote(password) + ";";

            pqxx::result R = N.exec(query);
            return !R.empty();

        }catch (const exception& e) {
        cerr << "\n[Database Error]: " << e.what() << endl;
        return false;
        }
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
        ImGui::Begin("Manager Dashboard");
        ImGui::Text("Welcome Manager: %s", name.c_str());
        if(ImGui::Button("Manage Employees & Tables")) { /* Logic */ }
        ImGui::End();
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
        ImGui::Begin("Employee Dashboard");
        ImGui::Text("Welcome Employee: %s", name.c_str());
        if (ImGui::Button("Verify Orders & Customers")) { /* Logic */ }
        ImGui::End();
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
        ImGui::Begin("Customer Storefront");
        ImGui::Text("Welcome Customer: %s", name.c_str());
        if (ImGui::Button("Browse Catalog & Place Order")) { /* Logic */ }
        ImGui::End();
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

    int inputId = 0;
    char inputName[128] = "";
    char inputPassword[128] = "";
    int selectedRole = 0;
    const char* roles[3] = {"Manager", "Employee", "Customer"};

public:
    void renderMainGUIWindow() {
        ImGui::Begin("Electronic Store Management System");

        if (!isLoggedIn) {
            if (ImGui::BeginTabBar("AuthTabs")) {
                if (ImGui::BeginTabItem("Login")) {
                    ImGui::InputInt("ID", &inputId);
                    ImGui::InputText("Name", inputName, IM_ARRAYSIZE(inputName));
                    
                    
                    ImGui::InputText("Password", inputPassword, IM_ARRAYSIZE(inputPassword), ImGuiInputTextFlags_Password);
                    
                    ImGui::Combo("Role", &selectedRole, roles, 3);

                    if (ImGui::Button("Login Submit")) {
                        string roleStr = roles[selectedRole];
                        auto tempUser = UserFactory::createUser(roleStr, inputId, inputName, "", inputPassword, 0);
                        if (tempUser && tempUser->login(inputId, inputName, inputPassword)) {
                            loggedInUser = tempUser;
                            isLoggedIn = true;
                        }
                    }
                    ImGui::EndTabItem();
                }
                ImGui::EndTabBar();
            }
        } else {
            if (loggedInUser) {
                loggedInUser->renderGUI();
                if (ImGui::Button("Log Out")) { 
                    isLoggedIn = false; 
                    loggedInUser = nullptr; 
                }
            }
        }

        ImGui::End();
    }   
};

// ==========================================
// 5. MAIN GRAPHICAL PROGRAM
// ==========================================
int main() {
    
    //TESTING GUI , DO NOT TOUCH
    /*
    if (!glfwInit())
        return -1;

    GLFWwindow* window = glfwCreateWindow(1280, 720, "Electronic Store Management System", NULL, NULL);
    if (window == NULL) {
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    ImGui::GetStyle().ScaleAllSizes(1.5f);
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    int current_screen = 0;
    char username[64] = "";
    char password[64] = "";
    std::string message = "";

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(io.DisplaySize);

        ImGui::Begin("Electronic Store Management System", NULL, 
            ImGuiWindowFlags_NoResize | 
            ImGuiWindowFlags_NoMove | 
            ImGuiWindowFlags_NoCollapse | 
            ImGuiWindowFlags_NoBringToFrontOnFocus);

        if (current_screen == 0) {
            ImGui::SetCursorPos(ImVec2(io.DisplaySize.x * 0.35f, io.DisplaySize.y * 0.35f));
            ImGui::Text("Welcome to Electronic Store Management System");
            
            ImGui::SetCursorPos(ImVec2(io.DisplaySize.x * 0.4f, io.DisplaySize.y * 0.45f));
            if (ImGui::Button("Login", ImVec2(150, 45))) {
                current_screen = 1;
                message = "";
            }

            ImGui::SetCursorPos(ImVec2(io.DisplaySize.x * 0.4f, io.DisplaySize.y * 0.55f));
            if (ImGui::Button("Sign Up", ImVec2(150, 45))) {
                current_screen = 2;
                message = "";
            }
        }
        else if (current_screen == 1) {
            ImGui::Text("=== Login Screen ===");
            ImGui::InputText("Username", username, IM_ARRAYSIZE(username));
            ImGui::InputText("Password", password, IM_ARRAYSIZE(password), ImGuiInputTextFlags_Password);

            if (ImGui::Button("Login Submit")) {
                message = "Login feature executed!";
            }

            if (!message.empty()) {
                ImGui::TextColored(ImVec4(0, 1, 0, 1), "%s", message.c_str());
            }

            if (ImGui::Button("Back to Home")) {
                current_screen = 0;
            }
        }
        else if (current_screen == 2) {
            ImGui::Text("=== Sign Up Screen ===");
            ImGui::InputText("New Username", username, IM_ARRAYSIZE(username));
            ImGui::InputText("New Password", password, IM_ARRAYSIZE(password), ImGuiInputTextFlags_Password);

            if (ImGui::Button("Create Account")) {
                try {
                    message = "Account created successfully and added to Database!";
                } catch (const std::exception &e) {
                    message = "Error: " + std::string(e.what());
                }
            }

            if (!message.empty()) {
                ImGui::TextColored(ImVec4(1, 1, 0, 1), "%s", message.c_str());
            }

            if (ImGui::Button("Back to Home")) {
                current_screen = 0;
            }
        }

        ImGui::End();

        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
    */
    return 0;
}