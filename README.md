# 🏨 C++ Console Hotel Management System

A robust, console-based Hotel Management System built in standard C++. This project was developed to simulate real-world enterprise software architecture, featuring role-based access control (RBAC), automated billing with SST (Sales and Service Tax) calculation, receipt export, and robust input validation.

---

## 🚀 Key Features

* **Role-Based Access Control (RBAC):** 
  * Distinct permission matrix for **Admin**, **Employee**, and **Customer**.
  * Dynamic menu rendering based on user roles and permissions.
* **User & Account Management:**
  * Secure authentication, account creation with role assignment, and self-service profile updates.
  * Regex-based strict email validation and duplicate checks.
* **Room Management:**
  * Dynamic room ID generation (`R001`, `R002`, etc.) and multi-criteria search (by ID, type, status).
  * Room status synchronization (Available, Occupied, Under Maintenance) linked with active bookings.
* **Booking & Payment Processing:**
  * Date validation (including leap year checks) and duration tracking.
  * Automated subtotal, tax (6% SST), and total calculation.
  * Support for multiple payment methods (Cash, Card, Online) and dynamic status updates (Pending, Paid, Refunded).
* **Reporting & File I/O:**
  * Comprehensive Admin Dashboard showing real-time room occupancy, user statistics, and net income reports.
  * Automated `.txt` receipt export functionality for successful transactions.

---

## 🛠️ Tech Stack & Concepts
* **Language:** C++ (Standard C++)
* **Core Concepts:** Object-Oriented Programming (OOP concepts & modular design), STL (`std::vector`, `std::string`), File I/O (`std::ofstream`), Regular Expressions (`<regex>`), Error Handling & Input Validation.

---
