#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <pqxx/pqxx>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>

using namespace std;

// ==========================================
// 1. Product Class
// ==========================================
class Product {
private:
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

    void updateStock(int quantity) { stockQuantity -= quantity; }
    bool checkAvailability(int requestedQty) const { return stockQuantity >= requestedQty; }
};

// ==========================================
// 2. ORDER CLASS
// ==========================================
class Order {
private:
    int orderId;
    int customerId;
    vector<pair<int, int>> productsList;
    double totalAmount;
    string status;

public:
    Order(int id, int custId) 
        : orderId(id), customerId(custId), totalAmount(0.0), status("Pending") {}

    void addProduct(int productId, int quantity, double price) {
        productsList.push_back({productId, quantity});
        totalAmount += (price * quantity);
    }

    double getTotalAmount() const { return totalAmount; }
    string getStatus() const { return status; }
    void setStatus(string newStatus) { status = newStatus; }

    void processTransaction() {
        if (totalAmount > 0) {
            status = "Completed";
        } else {
            status = "Failed";
        }
    }
};

// ==========================================
// 3. DATABASE MANAGER 
// ==========================================
class DatabaseManager {
public:
    static pqxx::connection connectDB() {
        return pqxx::connection("dbname=store user=postgres password=omaryoussri258 host=127.0.0.1 port=5432");
    }
    
    static void insertUserWithoutId(const string& role, const string& name, const string& email, const string& password, int num) {
        try {
            pqxx::connection C = connectDB();
            pqxx::work W(C);

            string tableName = "";
            if(role == "Manager" || role == "manager"){
                tableName = "managers";
            }
            else if(role == "Employee" || role == "employee"){
                tableName = "employees";
            }
            else if(role == "Customer" || role == "customer"){
                tableName = "customers";
            }
            
            string query = "INSERT INTO " + tableName + " (name, email, password, num) VALUES (" +
                            W.quote(name) + ", " +
                            W.quote(email) + ", " +
                            W.quote(password) + ", " +
                            W.quote(to_string(num)) + ");";
            W.exec(query);
            W.commit();
            cout << "\n[Database Success]: User added successfully to " << tableName << "!\n";
        } catch(const exception& e){
            cerr << "\n[Database Error]: " << e.what() << endl;  
        }
    }

    static bool authenticateQuery(const string& role, const string& name, const string& password) {
        try {
            pqxx::connection C = connectDB();
            pqxx::nontransaction N(C);

            string tableName = "";
            if (role == "Manager" || role == "manager") { 
                tableName = "managers"; 
            } else if (role == "Employee" || role == "employee"){ 
                tableName = "employees"; 
            } else if (role == "Customer" || role == "customer"){ 
                tableName = "customers"; 
            }

            string query = "SELECT * FROM " + tableName + " WHERE TRIM(name) ILIKE TRIM(" + N.quote(name) + 
                           ") AND password = " + N.quote(password) + ";";

            pqxx::result R = N.exec(query);
            return !R.empty();
        } catch (const exception& e) {
            cerr << "\n[Database Error]: " << e.what() << endl;
            return false;
        }
    }
};

// ==========================================
// 4. USER HIERARCHY
// ==========================================
class Authentication {
public:
    virtual bool login(const string& name, const string& password) = 0;
    virtual void signUp(const string& name, const string& password, const string& email, int num) = 0;
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
        : id(id), name(name), email(email), password(password), phoneNumber(num), role(role) {}

    string getName() const { return name; }
    string getRole() const { return role; }
    int getId() const { return id; }

    virtual bool renderGUI() = 0;
    virtual ~User() {}
};

// ==========================================
// 5. Manager Class
// ==========================================
class Manager : public User {
private:
    int managerSubView = 0; 
    int empSubMenu = 0;     
    int prodSubMenu = 0;    

    char empName[128] = "";
    char empEmail[128] = "";
    char empPass[128] = "";
    int empNum = 0;
    double newSalary = 0.0;
    int targetEmpId = 0;

    char prodName[128] = "";
    int prodQty = 0;
    int targetProdId = 0;
    string msg = "";
    string activeTableDataName = "";

public:
    Manager(int id, string name, string email, string password, int num)
        : User(id, name, email, password, num, "Manager") {}

    bool login(const string& name, const string& password) override {
        return DatabaseManager::authenticateQuery("manager", name, password);
    }

    void signUp(const string& name, const string& password, const string& email, int num) override {
        DatabaseManager::insertUserWithoutId("Manager", name, email, password, num);
    }

    bool renderGUI() override {
        bool loggedOut = false;

        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
        ImGui::Begin("Manager Dashboard", NULL, 
            ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

        ImGui::Text("Hello \"Manager\": %s", name.c_str());
        ImGui::Separator();

        if (managerSubView == 0) {
            if (ImGui::Button("Manage Employees", ImVec2(220, 45))) { managerSubView = 1; empSubMenu = 0; msg = ""; }
            if (ImGui::Button("View Tables", ImVec2(220, 45))) { managerSubView = 2; activeTableDataName = ""; }
            if (ImGui::Button("Manage Products", ImVec2(220, 45))) { managerSubView = 3; prodSubMenu = 0; msg = ""; }
        } 
        else if (managerSubView == 1) {
            ImGui::Text("--- Manage Employees ---");
            if (empSubMenu == 0) {
                if (ImGui::Button("Add Employee", ImVec2(200, 35))) empSubMenu = 1;
                if (ImGui::Button("Remove Employee", ImVec2(200, 35))) empSubMenu = 2;
                if (ImGui::Button("Change Employee Salary", ImVec2(200, 35))) empSubMenu = 3;

                if (ImGui::Button("Back to Manager Menu", ImVec2(180, 35))) managerSubView = 0;
            } 
            else if (empSubMenu == 1) {
                ImGui::Text("Add New Employee");
                ImGui::InputText("Name", empName, IM_ARRAYSIZE(empName));
                ImGui::InputText("Email", empEmail, IM_ARRAYSIZE(empEmail));
                ImGui::InputText("Password", empPass, IM_ARRAYSIZE(empPass), ImGuiInputTextFlags_Password);
                ImGui::InputInt("Number", &empNum);

                if (ImGui::Button("Confirm Add", ImVec2(150, 35))) {
                    DatabaseManager::insertUserWithoutId("Employee", empName, empEmail, empPass, empNum);
                    msg = "Employee Added Successfully!";
                }
                if (!msg.empty()) ImGui::TextColored(ImVec4(0,1,0,1), "%s", msg.c_str());

                if (ImGui::Button("Back", ImVec2(100, 30))) empSubMenu = 0;
            }
            else if (empSubMenu == 2) {
                ImGui::Text("Remove Employee");
                ImGui::InputInt("Employee ID to Remove", &targetEmpId);
                if (ImGui::Button("Confirm Remove", ImVec2(150, 35))) {
                    try {
                        pqxx::connection C = DatabaseManager::connectDB();
                        pqxx::work W(C);
                        W.exec("DELETE FROM employees WHERE employee_id = " + to_string(targetEmpId) + ";");
                        W.commit();
                        msg = "Employee Removed Successfully!";
                    } catch(const exception& e) { msg = "Error removing employee"; }
                }
                if (!msg.empty()) ImGui::TextColored(ImVec4(1,0,0,1), "%s", msg.c_str());
                if (ImGui::Button("Back", ImVec2(100, 30))) empSubMenu = 0;
            }
            else if (empSubMenu == 3) {
                ImGui::Text("Change Employee Salary");
                ImGui::InputInt("Employee ID", &targetEmpId);
                ImGui::InputDouble("New Salary", &newSalary);
                if (ImGui::Button("Update Salary", ImVec2(150, 35))) {
                    try {
                        pqxx::connection C = DatabaseManager::connectDB();
                        pqxx::work W(C);
                        W.exec("UPDATE employees SET salary = " + to_string(newSalary) + " WHERE employee_id = " + to_string(targetEmpId) + ";");
                        W.commit();
                        msg = "Salary Updated Successfully!";
                    } catch(const exception& e) { msg = "Error updating salary"; }
                }
                if (!msg.empty()) ImGui::TextColored(ImVec4(0,1,0,1), "%s", msg.c_str());
                if (ImGui::Button("Back", ImVec2(100, 30))) empSubMenu = 0;
            }
        } 
        else if (managerSubView == 2) {
            ImGui::Text("--- Choose Table to View ---");
            if (ImGui::Button("Managers Table", ImVec2(180, 30))) activeTableDataName = "managers";
            if (ImGui::Button("Employees Table", ImVec2(180, 30))) activeTableDataName = "employees";
            if (ImGui::Button("Customers Table", ImVec2(180, 30))) activeTableDataName = "customers";
            if (ImGui::Button("Products Table", ImVec2(180, 30))) activeTableDataName = "products";
            if (ImGui::Button("Orders Table", ImVec2(180, 30))) activeTableDataName = "orders";

            if (!activeTableDataName.empty()) {
                ImGui::Separator();
                ImGui::Text("Data from table: %s", activeTableDataName.c_str());
                try {
                    pqxx::connection C = DatabaseManager::connectDB();
                    pqxx::nontransaction N(C);
                    pqxx::result R = N.exec("SELECT * FROM " + activeTableDataName + ";");

                    int cols = R.columns();
                    if (ImGui::BeginTable("ManagerTableViewerDynamic", cols, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
                        for (int i = 0; i < cols; ++i) {
                            ImGui::TableSetupColumn(R.column_name(i));
                        }
                        ImGui::TableHeadersRow();

                        for (auto row : R) {
                            ImGui::TableNextRow();
                            for (int i = 0; i < cols; ++i) {
                                ImGui::TableSetColumnIndex(i);
                                ImGui::Text("%s", row[i].c_str());
                            }
                        }
                        ImGui::EndTable();
                    }
                } catch(const exception& e) {
                    ImGui::TextColored(ImVec4(1,0,0,1), "Could not load table data.");
                }
            }

            ImGui::Separator();
            if (ImGui::Button("Back to Manager Menu", ImVec2(180, 35))) { managerSubView = 0; activeTableDataName = ""; }
        }
        else if (managerSubView == 3) {
            ImGui::Text("--- Manage Products ---");
            if (prodSubMenu == 0) {
                if (ImGui::Button("Add Product", ImVec2(180, 35))) prodSubMenu = 1;
                if (ImGui::Button("Remove Product", ImVec2(180, 35))) prodSubMenu = 2;
                if (ImGui::Button("Manage a Product", ImVec2(180, 35))) prodSubMenu = 3;

                if (ImGui::Button("Back to Manager Menu", ImVec2(180, 35))) managerSubView = 0;
            }
            else if (prodSubMenu == 1) {
                ImGui::Text("Add Product");
                ImGui::InputText("Name", prodName, IM_ARRAYSIZE(prodName));
                ImGui::InputInt("Quantity", &prodQty);

                if (ImGui::Button("Confirm Add Product", ImVec2(180, 35))) {
                    try {
                        pqxx::connection C = DatabaseManager::connectDB();
                        pqxx::work W(C);
                        W.exec("INSERT INTO products (p_name, quantity) VALUES (" + W.quote(prodName) + ", " + to_string(prodQty) + ");");
                        W.commit();
                        msg = "Product Added Successfully!";
                    } catch(const exception& e) { msg = "Error adding product"; }
                }
                if (!msg.empty()) ImGui::TextColored(ImVec4(0,1,0,1), "%s", msg.c_str());
                if (ImGui::Button("Back", ImVec2(100, 30))) prodSubMenu = 0;
            }
            else if (prodSubMenu == 2) {
                ImGui::Text("Remove Product");
                ImGui::InputInt("Product ID to Remove", &targetProdId);
                if (ImGui::Button("Confirm Remove", ImVec2(150, 35))) {
                    try {
                        pqxx::connection C = DatabaseManager::connectDB();
                        pqxx::work W(C);
                        W.exec("DELETE FROM products WHERE p_id = " + to_string(targetProdId) + ";");
                        W.commit();
                        msg = "Product Removed Successfully!";
                    } catch(const exception& e) { msg = "Couldn't Find Product"; }
                }
                if (!msg.empty()) ImGui::TextColored(ImVec4(1,0,0,1), "%s", msg.c_str());
                if (ImGui::Button("Back", ImVec2(100, 30))) prodSubMenu = 0;
            }
            else if (prodSubMenu == 3) {
                ImGui::Text("Manage/Modify Product Quantity");
                ImGui::InputInt("Product ID", &targetProdId);
                ImGui::InputInt("New Quantity", &prodQty);
                if (ImGui::Button("Update Product", ImVec2(150, 35))) {
                    try {
                        pqxx::connection C = DatabaseManager::connectDB();
                        pqxx::work W(C);
                        W.exec("UPDATE products SET quantity = " + to_string(prodQty) + " WHERE p_id = " + to_string(targetProdId) + ";");
                        W.commit();
                        msg = "Product Modified Successfully!";
                    } catch(const exception& e) { msg = "Couldn't Find Product"; }
                }
                if (!msg.empty()) ImGui::TextColored(ImVec4(0,1,0,1), "%s", msg.c_str());
                if (ImGui::Button("Back", ImVec2(100, 30))) prodSubMenu = 0;
            }
        }

        ImGui::SetCursorPos(ImVec2(ImGui::GetIO().DisplaySize.x - 150, ImGui::GetIO().DisplaySize.y - 70));
        if (ImGui::Button("Log Out", ImVec2(130, 40))) { loggedOut = true; }

        ImGui::End();
        return loggedOut;
    }
};

// ==========================================
// 6. Employee Class
// ==========================================
class Employee : public User {
private:
    int empSubView = 0; 
    string activeEmployeeTableName = "";
    string verifyMsg = "";
    int orderToVerify = -1; 

public:
    Employee(int id, string name, string email, string password, int num)
        : User(id, name, email, password, num, "Employee") {}

    bool login(const string& name, const string& password) override {
        return DatabaseManager::authenticateQuery("employee", name, password);
    }

    void signUp(const string& name, const string& password, const string& email, int num) override {
        DatabaseManager::insertUserWithoutId("Employee", name, email, password, num);
    }

    bool renderGUI() override {
        bool loggedOut = false;
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
        ImGui::Begin("Employee Dashboard", NULL, 
            ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

        ImGui::Text("Welcome \"Employee\": %s", name.c_str());
        ImGui::Separator();

        if (empSubView == 0) {
            if (ImGui::Button("View Tables", ImVec2(180, 40))) { empSubView = 2; activeEmployeeTableName = ""; }
            if (ImGui::Button("Verfiy Order", ImVec2(180, 40))) { empSubView = 1; verifyMsg = ""; }
        } 
        else if (empSubView == 1) {
            ImGui::Text("--- Pending Orders Verification ---");
            
            orderToVerify = -1; 

            try {
                pqxx::connection C = DatabaseManager::connectDB();
                pqxx::nontransaction N(C);
                pqxx::result R = N.exec("SELECT order_id, customer_id, p_id, p_quantity, state FROM orders WHERE state = 'Pending';");

                if (R.empty()) {
                    ImGui::Text("No pending orders found.");
                } else {
                    int cols = R.columns();
                    if (ImGui::BeginTable("PendingOrdersTableDynamic", cols + 1, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
                        for (int i = 0; i < cols; ++i) {
                            ImGui::TableSetupColumn(R.column_name(i));
                        }
                        ImGui::TableSetupColumn("Action");
                        ImGui::TableHeadersRow();

                        for (auto row : R) {
                            ImGui::TableNextRow();
                            int ordId = row[0].as<int>();
                            int prodId = row[2].as<int>();
                            int orderedQty = row[3].as<int>();

                            for (int i = 0; i < cols; ++i) {
                                ImGui::TableSetColumnIndex(i);
                                ImGui::Text("%s", row[i].c_str());
                            }
                            
                            ImGui::TableSetColumnIndex(cols);
                            string btnLabel = "Verify##" + to_string(ordId);
                            if (ImGui::Button(btnLabel.c_str())) {
                                orderToVerify = ordId; 
                            }
                        }
                        ImGui::EndTable(); 
                    }
                }
            } catch(const exception& e) {
                ImGui::TextColored(ImVec4(1,0,0,1), "Error loading pending orders.");
            }

            if (orderToVerify != -1) {
                try {
                    pqxx::connection C = DatabaseManager::connectDB();
                    pqxx::work W_check(C);
                    
                    pqxx::result ordRes = W_check.exec("SELECT p_id, p_quantity FROM orders WHERE order_id = " + to_string(orderToVerify) + ";");
                    if (!ordRes.empty()) {
                        int prodId = ordRes[0][0].as<int>();
                        int orderedQty = ordRes[0][1].as<int>();

                        pqxx::result stockRes = W_check.exec("SELECT quantity FROM products WHERE p_id = " + to_string(prodId) + ";");
                        if (!stockRes.empty()) {
                            int storedStock = stockRes[0][0].as<int>();

                            if (orderedQty <= storedStock) {
                                W_check.exec("UPDATE products SET quantity = quantity - " + to_string(orderedQty) + " WHERE p_id = " + to_string(prodId) + ";");
                                W_check.exec("UPDATE orders SET state = 'Accepted' WHERE order_id = " + to_string(orderToVerify) + ";");
                                W_check.commit();
                                verifyMsg = "Order #" + to_string(orderToVerify) + " Accepted! Stock decreased successfully.";
                            } else {
                                verifyMsg = "Order #" + to_string(orderToVerify) + " CANNOT be made! Ordered quantity > Stored stock.";
                            }
                        } else {
                            verifyMsg = "Product not found in stock table!";
                        }
                    }
                } catch(const exception& e) {
                    verifyMsg = "Database error during verification.";
                }
            }

            if (!verifyMsg.empty()) {
                ImGui::Separator();
                ImGui::TextWrapped("%s", verifyMsg.c_str());
            }

            if (ImGui::Button("Back", ImVec2(100, 30))) empSubView = 0;
        } 
        else if (empSubView == 2) {
            ImGui::Text("--- Choose Table ---");
            if (ImGui::Button("Customers", ImVec2(150, 30))) activeEmployeeTableName = "customers";
            if (ImGui::Button("Orders", ImVec2(150, 30))) activeEmployeeTableName = "orders";
            if (ImGui::Button("Products", ImVec2(150, 30))) activeEmployeeTableName = "products";

            if (!activeEmployeeTableName.empty()) {
                ImGui::Separator();
                ImGui::Text("Table Data: %s", activeEmployeeTableName.c_str());
                try {
                    pqxx::connection C = DatabaseManager::connectDB();
                    pqxx::nontransaction N(C);
                    pqxx::result R = N.exec("SELECT * FROM " + activeEmployeeTableName + ";");

                    int cols = R.columns();
                    if (ImGui::BeginTable("EmpTableViewDynamic", cols, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
                        for (int i = 0; i < cols; ++i) {
                            ImGui::TableSetupColumn(R.column_name(i));
                        }
                        ImGui::TableHeadersRow();

                        for (auto row : R) {
                            ImGui::TableNextRow();
                            for (int i = 0; i < cols; ++i) {
                                ImGui::TableSetColumnIndex(i);
                                ImGui::Text("%s", row[i].c_str());
                            }
                        }
                        ImGui::EndTable(); 
                    }
                } catch(const exception& e) {
                    ImGui::TextColored(ImVec4(1,0,0,1), "Error loading table data.");
                }
            }

            ImGui::Separator();
            if (ImGui::Button("Back", ImVec2(100, 30))) { empSubView = 0; activeEmployeeTableName = ""; }
        }

        ImGui::SetCursorPos(ImVec2(ImGui::GetIO().DisplaySize.x - 150, ImGui::GetIO().DisplaySize.y - 70));
        if (ImGui::Button("Log Out", ImVec2(130, 40))) { loggedOut = true; }

        ImGui::End();
        return loggedOut;
    }
};

// ==========================================
// 7. Customer Class
// ==========================================
class Customer : public User {
private:
    int custSubView = 0; 
    int orderProdId = 0;
    int orderQty = 1;
    string customerMsg = "";

public:
    Customer(int id, string name, string email, string password, int num)
        : User(id, name, email, password, num, "Customer") {}

    bool login(const string& name, const string& password) override {
        return DatabaseManager::authenticateQuery("customer", name, password);
    }

    void signUp(const string& name, const string& password, const string& email, int num) override {
        DatabaseManager::insertUserWithoutId("Customer", name, email, password, num);
    }

    bool renderGUI() override {
        bool loggedOut = false;
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
        ImGui::Begin("Customer Storefront", NULL, 
            ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

        ImGui::Text("Welcome \"Customer\": %s", name.c_str());
        ImGui::Separator();

        if (custSubView == 0) {
            ImGui::Text("Product Table (Catalog)");
            try {
                pqxx::connection C = DatabaseManager::connectDB();
                pqxx::nontransaction N(C);
                pqxx::result R = N.exec("SELECT p_id, p_name, quantity FROM products;");

                int cols = R.columns();
                if (ImGui::BeginTable("CustomerProductCatalogDynamic", cols, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
                    for (int i = 0; i < cols; ++i) {
                        ImGui::TableSetupColumn(R.column_name(i));
                    }
                    ImGui::TableHeadersRow();

                    for (auto row : R) {
                        ImGui::TableNextRow();
                        for (int i = 0; i < cols; ++i) {
                            ImGui::TableSetColumnIndex(i);
                            ImGui::Text("%s", row[i].c_str());
                        }
                    }
                    ImGui::EndTable();
                }
            } catch(const exception& e) {
                ImGui::TextColored(ImVec4(1,0,0,1), "Error loading products catalog.");
            }

            if (ImGui::Button("Make an Order", ImVec2(180, 45))) {
                custSubView = 1;
                customerMsg = "";
            }
        } 
        else if (custSubView == 1) {
            ImGui::Text("--- Product Catalog Reference ---");
            try {
                pqxx::connection C = DatabaseManager::connectDB();
                pqxx::nontransaction N(C);
                pqxx::result R = N.exec("SELECT p_id, p_name, quantity FROM products;");

                int cols = R.columns();
                if (ImGui::BeginTable("CheckoutCatalogRefDynamic", cols, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
                    for (int i = 0; i < cols; ++i) {
                        ImGui::TableSetupColumn(R.column_name(i));
                    }
                    ImGui::TableHeadersRow();

                    for (auto row : R) {
                        ImGui::TableNextRow();
                        for (int i = 0; i < cols; ++i) {
                            ImGui::TableSetColumnIndex(i);
                            ImGui::Text("%s", row[i].c_str());
                        }
                    }
                    ImGui::EndTable();
                }
            } catch(...) {}

            ImGui::Separator();
            ImGui::Text("--- Check Out Form ---");
            ImGui::InputInt("Product ID :", &orderProdId);
            ImGui::InputInt("Quantity :", &orderQty);

            if (ImGui::Button("Confirm Order", ImVec2(150, 40))) {
                try {
                    pqxx::connection C = DatabaseManager::connectDB();
                    pqxx::work W(C);
                    
                    pqxx::result custRes = W.exec("SELECT customer_id FROM customers WHERE TRIM(name) ILIKE TRIM(" + W.quote(name) + ");");
                    int actualCustId = 1; 
                    if (!custRes.empty()) {
                        actualCustId = custRes[0][0].as<int>();
                    }

                    W.exec("INSERT INTO orders (customer_id, p_id, p_quantity, state) VALUES (" + 
                           to_string(actualCustId) + ", " + to_string(orderProdId) + ", " + to_string(orderQty) + ", 'Pending');");
                    W.commit();
                    
                    customerMsg = "Order placed successfully! Sent to employee for verification.";
                    custSubView = 0;
                } catch(const exception& e) {
                    customerMsg = "Error placing order. Check product ID validity.";
                }
            }

            if (!customerMsg.empty()) {
                ImGui::TextColored(ImVec4(0,1,0,1), "%s", customerMsg.c_str());
            }

            if (ImGui::Button("Back to Catalog", ImVec2(130, 30))) {
                custSubView = 0;
            }
        }

        ImGui::SetCursorPos(ImVec2(ImGui::GetIO().DisplaySize.x - 150, ImGui::GetIO().DisplaySize.y - 70));
        if (ImGui::Button("Log Out", ImVec2(130, 40))) { loggedOut = true; }

        ImGui::End();
        return loggedOut;
    }
};

// ==========================================
// 8. USER FACTORY
// ==========================================
class UserFactory {
public:
    static shared_ptr<User> createUser(const string& role, int id, const string& name, const string& email, const string& password, int num) {
        if (role == "Manager") {
            return make_shared<Manager>(id, name, email, password, num);
        } else if (role == "Employee") {
            return make_shared<Employee>(id, name, email, password, num);
        } else if (role == "Customer") {
            return make_shared<Customer>(id, name, email, password, num);
        }
        return nullptr;
    }
};

// ==========================================
// 9. GUI APPLICATION RUNNER
// ==========================================
class RunGUI {
private:
    int current_screen = 0; 
    
    shared_ptr<User> loggedInUser = nullptr;
    bool isLoggedIn = false;

    char inputName[128] = "";
    char inputEmail[128] = "";
    char inputPassword[128] = "";
    int inputNumber = 0;
    int selectedRole = 0;
    const char* roles[3] = {"Manager", "Employee", "Customer"};
    string statusMessage = "";

public:
    void renderMainGUIWindow() {
        if (!isLoggedIn) {
            ImGui::SetNextWindowPos(ImVec2(0, 0));
            ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
            ImGui::Begin("TechNova Store Management System", NULL, 
                ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

            if (current_screen == 0) {
                ImGui::SetCursorPos(ImVec2(ImGui::GetIO().DisplaySize.x * 0.32f, ImGui::GetIO().DisplaySize.y * 0.30f));
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.2f, 0.7f, 1.0f, 1.0f)); 
                ImGui::Text("WELCOME TO TECHNOVA STORE");
                ImGui::PopStyleColor();

                ImGui::SetCursorPos(ImVec2(ImGui::GetIO().DisplaySize.x * 0.38f, ImGui::GetIO().DisplaySize.y * 0.37f));
                ImGui::Text("Powering Your Digital World");

                ImGui::SetCursorPos(ImVec2(ImGui::GetIO().DisplaySize.x * 0.38f, ImGui::GetIO().DisplaySize.y * 0.46f));
                if (ImGui::Button("Sign Up", ImVec2(180, 45))) {
                    current_screen = 1;
                    statusMessage = "";
                }

                ImGui::SetCursorPos(ImVec2(ImGui::GetIO().DisplaySize.x * 0.38f, ImGui::GetIO().DisplaySize.y * 0.55f));
                if (ImGui::Button("Login", ImVec2(180, 45))) {
                    current_screen = 2;
                    statusMessage = "";
                }
            } 
            else if (current_screen == 1) {
                ImGui::Text("Please Fill Out The Following Information");
                ImGui::Separator();

                ImGui::InputText("Name", inputName, IM_ARRAYSIZE(inputName));
                ImGui::InputText("E-mail", inputEmail, IM_ARRAYSIZE(inputEmail));
                ImGui::InputText("Password", inputPassword, IM_ARRAYSIZE(inputPassword), ImGuiInputTextFlags_Password);
                ImGui::InputInt("Number", &inputNumber);
                ImGui::Combo("Role", &selectedRole, roles, 3);

                if (ImGui::Button("Submit Sign Up", ImVec2(150, 40))) {
                    string roleStr = roles[selectedRole];
                    auto tempUser = UserFactory::createUser(roleStr, 0, inputName, inputEmail, inputPassword, inputNumber);
                    if (tempUser) {
                        tempUser->signUp(inputName, inputPassword, inputEmail, inputNumber);
                        statusMessage = "Account Created Successfully!";
                        current_screen = 2; 
                    }
                }

                if (!statusMessage.empty()) {
                    ImGui::TextColored(ImVec4(0, 1, 0, 1), "%s", statusMessage.c_str());
                }

                if (ImGui::Button("Back", ImVec2(100, 30))) {
                    current_screen = 0;
                }
            } 
            else if (current_screen == 2) {
                ImGui::Text("Welcome Back !");
                ImGui::Separator();

                ImGui::InputText("Name", inputName, IM_ARRAYSIZE(inputName));
                ImGui::InputText("Password", inputPassword, IM_ARRAYSIZE(inputPassword), ImGuiInputTextFlags_Password);
                ImGui::Combo("Role", &selectedRole, roles, 3);

                if (ImGui::Button("Login Submit", ImVec2(150, 40))) {
                    string roleStr = roles[selectedRole];
                    int realCustId = 1;
                    try {
                        pqxx::connection C = DatabaseManager::connectDB();
                        pqxx::nontransaction N(C);
                        pqxx::result R = N.exec("SELECT customer_id FROM customers WHERE TRIM(name) ILIKE TRIM(" + N.quote(inputName) + ");");
                        if (!R.empty()) realCustId = R[0][0].as<int>();
                    } catch(...) {}

                    auto tempUser = UserFactory::createUser(roleStr, realCustId, inputName, "", inputPassword, 0);
                    if (tempUser && tempUser->login(inputName, inputPassword)) {
                        loggedInUser = tempUser;
                        isLoggedIn = true;
                    } else {
                        statusMessage = "Invalid Credentials!";
                    }
                }

                if (!statusMessage.empty()) {
                    ImGui::TextColored(ImVec4(1, 0, 0, 1), "%s", statusMessage.c_str());
                }

                if (ImGui::Button("Back", ImVec2(100, 30))) {
                    current_screen = 0;
                }
            }
            ImGui::End();
        } else {
            if (loggedInUser) {
                if (loggedInUser->renderGUI()) {
                    isLoggedIn = false; 
                    loggedInUser = nullptr; 
                    current_screen = 0;
                }
            }
        }
    }
};

// ==========================================
// 10. MAIN FUNCTION
// ==========================================
int main() {
    if (!glfwInit())
        return -1;

    GLFWwindow* window = glfwCreateWindow(1280, 720, "TechNova Store Management System", NULL, NULL);
    if (window == NULL) {
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    ImGui::GetStyle().ScaleAllSizes(1.8f); 
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    RunGUI guiApp;

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        guiApp.renderMainGUIWindow();

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

    return 0;
}