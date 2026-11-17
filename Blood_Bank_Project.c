

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define GROUPS 8
#define NAME_LEN 50
#define PHONE_LEN 20

typedef struct {
    int id;
    char name[NAME_LEN];
    int age;
    char bloodGroup[4];
    char phone[PHONE_LEN];
} Donor;

typedef struct {
    char group[4];
    int units;
} BloodStock;

Donor *donors = NULL;
int donorCount = 0;
int donorCapacity = 0;

BloodStock stock[GROUPS] = {
    {"A+", 0}, {"A-", 0},
    {"B+", 0}, {"B-", 0},
    {"AB+", 0}, {"AB-", 0},
    {"O+", 0}, {"O-", 0}
};

/* Read a complete line safely. */
void readLine(const char *prompt, char *buffer, int size) {
    int ch;
    size_t len;

    printf("%s", prompt);

    if (fgets(buffer, size, stdin) == NULL) {
        printf("\nInput ended. Exiting.\n");
        free(donors);
        exit(0);
    }

    len = strlen(buffer);

    if (len > 0 && buffer[len - 1] == '\n') {
        buffer[len - 1] = '\0';
    } else {
        while ((ch = getchar()) != '\n' && ch != EOF) {
            /* Discard excess input. */
        }
    }
}

/* Read an integer within a specified range. */
int readInt(const char *prompt, int min, int max) {
    char line[100];
    char extra;
    int value;

    while (1) {
        readLine(prompt, line, sizeof(line));

        if (sscanf(line, " %d %c", &value, &extra) == 1 &&
            value >= min && value <= max) {
            return value;
        }

        printf("Invalid input. Enter a number from %d to %d.\n",
               min, max);
    }
}

/* Convert blood group letters to uppercase. */
void normalizeGroup(char *group) {
    int i;

    for (i = 0; group[i] != '\0'; i++) {
        group[i] = (char)toupper((unsigned char)group[i]);
    }
}

/* Find the index of a valid blood group. */
int findGroup(const char *group) {
    int i;

    for (i = 0; i < GROUPS; i++) {
        if (strcmp(stock[i].group, group) == 0) {
            return i;
        }
    }

    return -1;
}

/* Read and validate a blood group. */
int readBloodGroup(char *group) {
    readLine(
        "Enter blood group (A+, A-, B+, B-, AB+, AB-, O+, O-): ",
        group, 10
    );

    normalizeGroup(group);
    return findGroup(group);
}

/* Increase the capacity of the donor array when required. */
int ensureCapacity(void) {
    Donor *temp;
    int newCapacity;

    if (donorCount < donorCapacity) {
        return 1;
    }

    newCapacity = (donorCapacity == 0) ? 2 : donorCapacity * 2;

    temp = realloc(donors, (size_t)newCapacity * sizeof(Donor));

    if (temp == NULL) {
        printf("Memory allocation failed. Donor not added.\n");
        return 0;
    }

    donors = temp;
    donorCapacity = newCapacity;

    return 1;
}

/* Register a new donor. */
void addDonor(void) {
    Donor d;
    char group[10];

    if (!ensureCapacity()) {
        return;
    }

    d.id = donorCount + 1;

    readLine("Enter donor name: ", d.name, sizeof(d.name));

    if (d.name[0] == '\0') {
        printf("Name cannot be empty.\n");
        return;
    }

    d.age = readInt("Enter donor age (18-65): ", 18, 65);

    while (1) {
        if (readBloodGroup(group) >= 0) {
            break;
        }

        printf("Invalid blood group. Please try again.\n");
    }

    strcpy(d.bloodGroup, group);

    readLine("Enter phone number: ", d.phone, sizeof(d.phone));

    donors[donorCount] = d;
    donorCount++;

    printf("\nDonor registered successfully!\n");
    printf("Donor ID: %d\n", d.id);
    printf("Name: %s\n", d.name);
    printf("Blood group: %s\n", d.bloodGroup);
}

/* Display all donor records. */
void displayDonors(void) {
    int i;

    if (donorCount == 0) {
        printf("\nNo donor records found.\n");
        return;
    }

    printf("\n%-6s %-22s %-6s %-10s %-18s\n",
           "ID", "Name", "Age", "Group", "Phone");

    printf("---------------------------------------------------------------\n");

    for (i = 0; i < donorCount; i++) {
        printf("%-6d %-22s %-6d %-10s %-18s\n",
               donors[i].id,
               donors[i].name,
               donors[i].age,
               donors[i].bloodGroup,
               donors[i].phone);
    }
}

/* Search donors by blood group. */
void searchDonors(void) {
    char group[10];
    int i, found = 0;

    if (readBloodGroup(group) < 0) {
        printf("Invalid blood group.\n");
        return;
    }

    printf("\nDonors with blood group %s:\n", group);

    for (i = 0; i < donorCount; i++) {
        if (strcmp(donors[i].bloodGroup, group) == 0) {
            printf("ID: %d | Name: %s | Age: %d | Phone: %s\n",
                   donors[i].id,
                   donors[i].name,
                   donors[i].age,
                   donors[i].phone);

            found = 1;
        }
    }

    if (!found) {
        printf("No donors found for this blood group.\n");
    }
}

/* Record donated blood units in inventory. */
void recordDonation(void) {
    char group[10];
    int index, units;

    index = readBloodGroup(group);

    if (index < 0) {
        printf("Invalid blood group.\n");
        return;
    }

    units = readInt("Enter units collected (1-1000): ", 1, 1000);

    if (stock[index].units > 1000000 - units) {
        printf("Inventory limit exceeded.\n");
        return;
    }

    stock[index].units += units;

    printf("Donation recorded successfully.\n");
    printf("%s stock is now %d units.\n",
           stock[index].group, stock[index].units);
}

/* Display the current blood inventory. */
void displayStock(void) {
    int i;

    printf("\n========== BLOOD INVENTORY ==========\n");
    printf("%-15s %s\n", "Blood Group", "Available Units");
    printf("-------------------------------------\n");

    for (i = 0; i < GROUPS; i++) {
        printf("%-15s %d\n",
               stock[i].group, stock[i].units);
    }
}
int isCompatible(const char *donor, const char *recipient) {
    int aboCompatible = 0;
    int rhCompatible = 0;

    /* ABO compatibility. */
    if (strcmp(donor, "O+") == 0 ||
        strcmp(donor, "O-") == 0) {
        aboCompatible = 1;
    } else if (strcmp(donor, "A+") == 0 ||
               strcmp(donor, "A-") == 0) {
        aboCompatible =
            (strncmp(recipient, "A", 1) == 0 ||
             strncmp(recipient, "AB", 2) == 0);
    } else if (strcmp(donor, "B+") == 0 ||
               strcmp(donor, "B-") == 0) {
        aboCompatible =
            (strncmp(recipient, "B", 1) == 0 ||
             strncmp(recipient, "AB", 2) == 0);
    } else if (strcmp(donor, "AB+") == 0 ||
               strcmp(donor, "AB-") == 0) {
        aboCompatible =
            (strncmp(recipient, "AB", 2) == 0);
    }

    /* Rh compatibility. */
    if (donor[strlen(donor) - 1] == '-') {
        rhCompatible = 1;
    } else if (recipient[strlen(recipient) - 1] == '+') {
        rhCompatible = 1;
    }

    return aboCompatible && rhCompatible;
}

/* Check compatibility between two blood groups. */
void checkCompatibility(void) {
    char donorGroup[10];
    char recipientGroup[10];

    printf("\nEnter donor blood group:\n");

    if (readBloodGroup(donorGroup) < 0) {
        printf("Invalid donor blood group.\n");
        return;
    }

    printf("Enter recipient blood group:\n");

    if (readBloodGroup(recipientGroup) < 0) {
        printf("Invalid recipient blood group.\n");
        return;
    }

    if (isCompatible(donorGroup, recipientGroup)) {
        printf("%s red blood cells are compatible with %s "
               "under standard ABO/Rh rules.\n",
               donorGroup, recipientGroup);
    } else {
        printf("%s red blood cells are NOT compatible with %s "
               "under standard ABO/Rh rules.\n",
               donorGroup, recipientGroup);
    }
}

/* Request blood and update the inventory. */
void requestBlood(void) {
    char group[10];
    int index, units;

    index = readBloodGroup(group);

    if (index < 0) {
        printf("Invalid blood group.\n");
        return;
    }

    units = readInt("Enter units required (1-1000): ", 1, 1000);

    if (stock[index].units >= units) {
        stock[index].units -= units;

        printf("Blood request recorded successfully.\n");
        printf("Remaining %s stock: %d units\n",
               stock[index].group, stock[index].units);
    } else {
        printf("Insufficient stock for %s.\n", stock[index].group);
        printf("Available units: %d\n", stock[index].units);
    }
}

/* Main menu. */
int main(void) {
    int choice;

    do {
        printf("\n========================================\n");
        printf("   BLOOD BANK MANAGEMENT SYSTEM\n");
        printf("========================================\n");
        printf("1. Add Donor\n");
        printf("2. Display All Donors\n");
        printf("3. Search Donors by Blood Group\n");
        printf("4. Record Blood Donation\n");
        printf("5. Display Blood Inventory\n");
        printf("6. Check Blood Compatibility\n");
        printf("7. Request Blood\n");
        printf("0. Exit\n");
        printf("========================================\n");

        choice = readInt("Enter your choice: ", 0, 7);

        switch (choice) {
            case 1:
                addDonor();
                break;

            case 2:
                displayDonors();
                break;

            case 3:
                searchDonors();
                break;

            case 4:
                recordDonation();
                break;

            case 5:
                displayStock();
                break;

            case 6:
                checkCompatibility();
                break;

            case 7:
                requestBlood();
                break;

            case 0:
                printf("Exiting Blood Bank Management System.\n");
                break;

            default:
                printf("Invalid choice.\n");
        }

    } while (choice != 0);

    free(donors);
    donors = NULL;

    printf("Memory released successfully.\n");

    return 0;
}
