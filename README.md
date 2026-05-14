# Blockchain-Based Attendance Tracking System

An immutable, cryptographically secure attendance tracking application implemented purely in C. This project demonstrates the core principles of blockchain technology by replacing easily manipulated traditional attendance sheets with a decentralized ledger structure.

## System Overview

The system loads a pre-defined registry of students and allows administrators to mark attendance. Each attendance record is encapsulated in a dynamically allocated `Block`, hashed using `SHA-256`, and cryptographically linked to the previous block. Any attempt to alter historical attendance records will break the chain linkage, triggering the system's tamper detection mechanisms.

### Key Features
* **Immutable Ledger:** Records cannot be altered once appended to the chain.
* **Cryptographic Hashing:** Uses OpenSSL's SHA-256 for deterministic block hashing.
* **Simulated Digital Signatures:** Generates a deterministic 72-byte buffer mimicking an ECDSA footprint to authenticate entries.
* **File-Based Persistence:** Loads valid student credentials from a CSV-formatted `students.txt` file at startup.
* **Tamper Detection Engine:** Iterates through the linked list in memory to validate `hash` and `previous_hash` integrity, catching unauthorized modifications.

## Prerequisites

To compile and run this application, your system must have the following installed:
* **GCC (GNU Compiler Collection):** To compile the C source code.
* **OpenSSL Development Library:** Required for the cryptographic hashing functions.
  * Ubuntu/Debian: `sudo apt-get install libssl-dev`
  * CentOS/RHEL: `sudo yum install openssl-devel`
  * macOS (Homebrew): `brew install openssl`

## Compilation and Setup

1. Clone or download this repository to your local machine.
2. Ensure that your registry file, `students.txt`, is located in the same directory as the source code.
3. Compile the source code by linking the OpenSSL crypto library. Run the following command in your terminal:
* gcc Student_attendance.c -o student_attendance -lcrypto

## Running the Program
* ./student_attendance
