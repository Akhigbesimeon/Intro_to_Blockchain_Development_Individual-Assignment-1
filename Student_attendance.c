#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <openssl/sha.h>

#define MAX_STUDENTS 100
#define HASH_HEX_LEN 65
#define SIG_LEN 72

// Data structures

typedef struct {
    char student_id[20];
    char full_name[50];
    char course_code[10];
} Student;

typedef struct Block {
    int index;
    time_t timestamp;
    char student_id[20];
    char full_name[50];
    char course_code[10];
    char status[10];
    char previous_hash[HASH_HEX_LEN];
    unsigned char signature[SIG_LEN];
    char hash[HASH_HEX_LEN];
    struct Block* next;
} Block;

// Global variables
Student registry[MAX_STUDENTS];
int student_count = 0;
Block* blockchain_head = NULL;
Block* blockchain_tail = NULL;

// Helper functions
void compute_hash(const char* input, char* output) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256((unsigned char*)input, strlen(input), hash);
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sprintf(output + (i * 2), "%02x", hash[i]);
    }
    output[HASH_HEX_LEN - 1] = '\0';
}

// Hash generator for specific block
void calculate_block_hash(Block* b, char* out_hash) {
    char buffer[1024];
    snprintf(buffer, sizeof(buffer), "%d%ld%s%s%s%s%s",
             b->index, b->timestamp, b->student_id, b->full_name,
             b->course_code, b->status, b->previous_hash);
    compute_hash(buffer, out_hash);
}

void generate_mock_signature(Block* b) {
    char buffer[256];
    snprintf(buffer, sizeof(buffer), "SIG_%s_%ld", b->student_id, b->timestamp);
    unsigned char temp_hash[SHA256_DIGEST_LENGTH];
    SHA256((unsigned char*)buffer, strlen(buffer), temp_hash);

    memset(b->signature, 0, SIG_LEN);
    memcpy(b->signature, temp_hash, SHA256_DIGEST_LENGTH);
    memcpy(b->signature + SHA256_DIGEST_LENGTH, temp_hash, SHA256_DIGEST_LENGTH);
}

// Core requirements
void load_registry() {
    FILE* file = fopen("students.txt", "r");
    if (!file) {
        printf("ERROR: students.txt is missing.\n");
        exit(1);
    }

    char line[256];
    while (fgets(line, sizeof(line), file) && student_count < MAX_STUDENTS) {
        line[strcspn(line, "\r\n")] = 0;
        if (strlen(line) == 0) continue;

        char* id = strtok(line, ",");
        char* name = strtok(NULL, ",");
        char* course = strtok(NULL, ",");

        if (id && name && course) {
            strncpy(registry[student_count].student_id, id, 19);
            strncpy(registry[student_count].full_name, name, 49);
            strncpy(registry[student_count].course_code, course, 9);
            student_count++;
        }
    }
    fclose(file);

    if (student_count == 0) {
        printf("ERROR: students.txt is empty.\n");
        exit(1);
    }
    printf("Loaded %d students from registry.\n", student_count);
}

void create_genesis_block() {
    Block* genesis = (Block*)malloc(sizeof(Block));
    genesis->index = 0;
    genesis->timestamp = time(NULL);
    strcpy(genesis->student_id, "GENESIS");
    strcpy(genesis->full_name, "SYSTEM");
    strcpy(genesis->course_code, "NONE");
    strcpy(genesis->status, "SYSTEM");
    memset(genesis->previous_hash, '0', 64);
    genesis->previous_hash[64] = '\0';

    generate_mock_signature(genesis);
    calculate_block_hash(genesis, genesis->hash);

    genesis->next = NULL;
    blockchain_head = genesis;
    blockchain_tail = genesis;
    printf("Genesis block created.\n");
}

void mark_attendance() {
    char id[20], status[10];
    printf("\nEnter Student ID: ");
    scanf("%19s", id);

    // Validate ID
    int found_idx = -1;
    for (int i = 0; i < student_count; i++) {
        if (strcmp(registry[i].student_id, id) == 0) {
            found_idx = i;
            break;
        }
    }

    if (found_idx == -1) {
        printf("ERROR: Student ID not found in registry. Aborting.\n");
        return;
    }

    printf("Enter Status (PRESENT/ABSENT/LATE): ");
    scanf("%9s", status);

    Block* new_block = (Block*)malloc(sizeof(Block));
    new_block->index = blockchain_tail->index + 1;
    new_block->timestamp = time(NULL);
    strcpy(new_block->student_id, registry[found_idx].student_id);
    strcpy(new_block->full_name, registry[found_idx].full_name);
    strcpy(new_block->course_code, registry[found_idx].course_code);
    strcpy(new_block->status, status);
    strcpy(new_block->previous_hash, blockchain_tail->hash);

    generate_mock_signature(new_block);
    calculate_block_hash(new_block, new_block->hash);

    new_block->next = NULL;
    blockchain_tail->next = new_block;
    blockchain_tail = new_block;

    printf("Attendance marked securely for %s.\n", new_block->full_name);
}

void view_records() {
    Block* current = blockchain_head;
    printf("\n-- Student Attendance Records --\n");
    while (current != NULL) {
        printf("Block %d | Time: %ld\n", current->index, current->timestamp);
        printf("Student: %s (%s) | Course: %s | Status: %s\n",
               current->full_name, current->student_id, current->course_code, current->status);
        printf("Prev Hash: %s\n", current->previous_hash);
        printf("Curr Hash: %s\n", current->hash);
        printf("-------------------------------------\n");
        current = current->next;
    }
}

void validate_chain() {
    Block* current = blockchain_head;
    printf("\n Initiating Chain Validation...\n");

    while (current != NULL) {
        char recalculated_hash[HASH_HEX_LEN];
        calculate_block_hash(current, recalculated_hash);

        if (strcmp(current->hash, recalculated_hash) != 0) {
            printf("TAMPER DETECTED at Block %d: Hash mismatch!\n", current->index);
            return;
        }

        if (current->next != NULL) {
            if (strcmp(current->hash, current->next->previous_hash) != 0) {
                printf("TAMPER DETECTED at Block %d: Chain linkage broken!\n", current->index);
                return;
            }
        }
        current = current->next;
    }
    printf("Blockchain is valid. No tampering detected.\n");
}

void tamper_demonstration() {
    if (blockchain_head == NULL || blockchain_head->next == NULL) {
        printf("Need at least 1 attendance record to demonstrate tampering.\n");
        return;
    }

    Block* target = blockchain_head->next;
    printf("\n Tampering with Block %d...\n", target->index);
    strcpy(target->status, "PRESENT");
    printf("Changed status manually. Run Validation to detect.\n");
}

// Main function
int main() {
    load_registry();
    create_genesis_block();

    int choice;
    do {
        printf("\n-- Student Attendance System --\n");
        printf("1. Mark Attendance\n");
        printf("2. View Records\n");
        printf("3. Validate Chain\n");
        printf("4. Tamper with a Record\n");
        printf("5. Exit\n");
        printf("Select option: ");
        if (scanf("%d", &choice) != 1) {
            while(getchar() != '\n');
            continue;
        }

        switch (choice) {
            case 1: mark_attendance(); break;
            case 2: view_records(); break;
            case 3: validate_chain() ; break;
            case 4: tamper_demonstration(); break;
            case 5: printf("Exiting...\n"); break;
            default: printf("Invalid choice.\n");
        }
    } while (choice != 5);

    return 0;
}
