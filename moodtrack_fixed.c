#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* ---- Struct definition (this was missing and caused most of the errors) ---- */
struct MoodEntry {
    int day;
    int month;
    int year;
    time_t timestamp;   // used by weeklyreport()/viewNotes() to compute recency
    int mood;            // 1-5
    char note[200];
};

void printMenu() {
    printf("============ MOOD TRACKER ============\n");
    printf("1. Add today's mood entry\n");
    printf("2. View all entries\n");
    printf("3. Update an entry\n");
    printf("4. Weekly report\n");
    printf("5. Monthly report\n");
    printf("6. Overall mood percentage\n");
    printf("7. View all notes\n");
    printf("8. Exit\n");
    printf("=======================================\n");
    printf("Enter your choice: ");
}

void addentry() {
    struct MoodEntry entry;
    FILE *fp;
    time_t t;
    struct tm *current;

    time(&t);
    current = localtime(&t);

    entry.day = current->tm_mday;
    entry.month = current->tm_mon + 1;
    entry.year = current->tm_year + 1900;
    entry.timestamp = t;   // store the timestamp so weekly report can use it

    printf("Today's Date : %02d/%02d/%04d\n", entry.day, entry.month, entry.year);

    /* Each line needs its own quotes - a string literal can't span lines raw */
    printf("================ MOOD SCALE ================\n");
    printf("1. Very Sad\n");
    printf("2. Sad\n");
    printf("3. Neutral\n");
    printf("4. Happy\n");
    printf("5. Very Happy\n");
    printf("============================================\n");
    printf("Enter your mood (1-5): ");

    scanf("%d", &entry.mood);
    getchar();

    printf("Enter your note: ");
    fgets(entry.note, sizeof(entry.note), stdin);

    fp = fopen("moods.dat", "ab");
    if (fp == NULL) {
        printf("Unable to open file.\n");
        return;              // was "return 1;" - invalid in a void function
    }

    fwrite(&entry, sizeof(entry), 1, fp);
    fclose(fp);

    printf("\nMood saved successfully!\n");
}

void viewentry() {
    FILE *fp;
    struct MoodEntry entry;

    fp = fopen("moods.dat", "rb");
    if (fp == NULL) {
        printf("No mood entries found.\n");
        return;
    }

    printf("\n========== ALL MOOD ENTRIES ==========\n\n");

    while (fread(&entry, sizeof(entry), 1, fp) == 1) {
        printf("Date : %02d/%02d/%04d\n", entry.day, entry.month, entry.year);

        switch (entry.mood) {
            case 1: printf("Mood : Very Sad\n");   break;
            case 2: printf("Mood : Sad\n");        break;
            case 3: printf("Mood : Neutral\n");    break;
            case 4: printf("Mood : Happy\n");      break;
            case 5: printf("Mood : Very Happy\n"); break;
            default: printf("Mood : Unknown\n");
        }

        printf("Note : %s", entry.note);
        printf("--------------------------------------\n");
    }

    fclose(fp);
}

void updateEntry() {
    FILE *fp, *temp;
    struct MoodEntry entry;

    int day, month, year;
    int choice;
    int found = 0;

    printf("\n========== UPDATE ENTRY ==========\n");
    printf("Enter Date (DD MM YYYY): ");
    scanf("%d %d %d", &day, &month, &year);
    getchar();

    fp = fopen("moods.dat", "rb");
    if (fp == NULL) {
        printf("\nNo mood entries found.\n");
        return;
    }

    temp = fopen("temp.dat", "wb");

    while (fread(&entry, sizeof(entry), 1, fp) == 1) {
        if (entry.day == day && entry.month == month && entry.year == year) {
            found = 1;

            printf("\n========== ENTRY FOUND ==========\n");
            printf("Date : %02d/%02d/%04d\n", entry.day, entry.month, entry.year);

            switch (entry.mood) {
                case 1: printf("Current Mood : Very Sad\n");   break;
                case 2: printf("Current Mood : Sad\n");        break;
                case 3: printf("Current Mood : Neutral\n");    break;
                case 4: printf("Current Mood : Happy\n");      break;
                case 5: printf("Current Mood : Very Happy\n"); break;
            }

            printf("Current Note : %s\n", entry.note);

            printf("\nWhat would you like to do?\n");
            printf("1. Update Mood\n");
            printf("2. Update Note\n");
            printf("3. Update Mood & Note\n");
            printf("4. Delete Entry\n");
            printf("5. Cancel\n");
            printf("\nEnter Choice : ");
            scanf("%d", &choice);
            getchar();

            switch (choice) {
                case 1:
                    printf("\nEnter New Mood (1-5): ");
                    scanf("%d", &entry.mood);
                    getchar();
                    printf("\nMood Updated Successfully!\n");
                    break;

                case 2:
                    printf("\nEnter New Note:\n");
                    fgets(entry.note, sizeof(entry.note), stdin);
                    printf("\nNote Updated Successfully!\n");
                    break;

                case 3:
                    printf("\nEnter New Mood (1-5): ");
                    scanf("%d", &entry.mood);
                    getchar();
                    printf("\nEnter New Note:\n");
                    fgets(entry.note, sizeof(entry.note), stdin);
                    printf("\nEntry Updated Successfully!\n");
                    break;

                case 4:
                    printf("\nEntry Deleted Successfully!\n");
                    continue;   // skip writing this entry to temp.dat = delete it

                case 5:
                    printf("\nUpdate Cancelled.\n");
                    break;

                default:
                    printf("\nInvalid Choice.\n");
            }
        }

        fwrite(&entry, sizeof(entry), 1, temp);
    }

    fclose(fp);
    fclose(temp);

    remove("moods.dat");
    rename("temp.dat", "moods.dat");

    if (!found) {
        printf("\nNo Entry Found For That Date.\n");
    }
}

void weeklyreport() {
    FILE *fp;
    struct MoodEntry entry;

    int verySad = 0, sad = 0, neutral = 0, happy = 0, veryHappy = 0;
    int totalEntries = 0, totalMood = 0;
    float average;

    time_t currentTime;
    time(&currentTime);

    fp = fopen("moods.dat", "rb");
    if (fp == NULL) {
        printf("No mood entries found.\n");
        return;
    }

    printf("\n========== WEEKLY REPORT ==========\n\n");

    while (fread(&entry, sizeof(entry), 1, fp) == 1) {
        double days = difftime(currentTime, entry.timestamp) / (60 * 60 * 24);

        if (days >= 0 && days <= 7) {
            totalEntries++;
            totalMood += entry.mood;

            switch (entry.mood) {
                case 1: verySad++;   break;
                case 2: sad++;       break;
                case 3: neutral++;   break;
                case 4: happy++;     break;
                case 5: veryHappy++; break;
            }
        }
    }

    fclose(fp);

    if (totalEntries == 0) {
        printf("No entries found in the last 7 days.\n");
        return;
    }

    average = (float)totalMood / totalEntries;

    printf("Total Entries : %d\n\n", totalEntries);
    printf("Very Sad   : %d\n", verySad);
    printf("Sad        : %d\n", sad);
    printf("Neutral    : %d\n", neutral);
    printf("Happy      : %d\n", happy);
    printf("Very Happy : %d\n\n", veryHappy);
    printf("Average Mood : %.2f / 5\n", average);
    printf("\n===================================\n");
}

void monthlyreport() {
    FILE *fp;
    struct MoodEntry entry;

    int currentMonth, currentYear;
    int verySad = 0, sad = 0, neutral = 0, happy = 0, veryHappy = 0;
    int totalEntries = 0, totalMood = 0;
    float average;

    time_t t;
    struct tm *current;

    time(&t);
    current = localtime(&t);

    currentMonth = current->tm_mon + 1;
    currentYear = current->tm_year + 1900;

    fp = fopen("moods.dat", "rb");
    if (fp == NULL) {
        printf("\nNo mood entries found.\n");
        return;
    }

    while (fread(&entry, sizeof(entry), 1, fp) == 1) {
        if (entry.month == currentMonth && entry.year == currentYear) {
            totalEntries++;
            totalMood += entry.mood;

            switch (entry.mood) {
                case 1: verySad++;   break;
                case 2: sad++;       break;
                case 3: neutral++;   break;
                case 4: happy++;     break;
                case 5: veryHappy++; break;
            }
        }
    }

    fclose(fp);

    if (totalEntries == 0) {
        printf("\nNo entries for this month.\n");
        return;
    }

    average = (float)totalMood / totalEntries;

    printf("\n========== MONTHLY REPORT ==========\n");
    printf("Month : %02d/%04d\n\n", currentMonth, currentYear);
    printf("Total Entries : %d\n\n", totalEntries);
    printf("Very Sad   : %d\n", verySad);
    printf("Sad        : %d\n", sad);
    printf("Neutral    : %d\n", neutral);
    printf("Happy      : %d\n", happy);
    printf("Very Happy : %d\n\n", veryHappy);
    printf("Average Mood : %.2f / 5\n", average);
    printf("====================================\n");
}

void moodpercentage() {
    FILE *fp;
    struct MoodEntry entry;

    int verySad = 0, sad = 0, neutral = 0, happy = 0, veryHappy = 0;
    int totalEntries = 0;

    fp = fopen("moods.dat", "rb");
    if (fp == NULL) {
        printf("\nNo mood entries found.\n");
        return;
    }

    while (fread(&entry, sizeof(entry), 1, fp) == 1) {
        totalEntries++;

        switch (entry.mood) {
            case 1: verySad++;   break;
            case 2: sad++;       break;
            case 3: neutral++;   break;
            case 4: happy++;     break;
            case 5: veryHappy++; break;
        }
    }

    fclose(fp);

    if (totalEntries == 0) {
        printf("No entries available.\n");
        return;
    }

    printf("\n========== OVERALL MOOD REPORT ==========\n\n");
    printf("Total Entries : %d\n\n", totalEntries);
    printf("Very Sad   : %.2f%%\n", (verySad * 100.0) / totalEntries);
    printf("Sad        : %.2f%%\n", (sad * 100.0) / totalEntries);
    printf("Neutral    : %.2f%%\n", (neutral * 100.0) / totalEntries);
    printf("Happy      : %.2f%%\n", (happy * 100.0) / totalEntries);
    printf("Very Happy : %.2f%%\n", (veryHappy * 100.0) / totalEntries);
    printf("\n=========================================\n");
}

void viewNotes() {
    FILE *fp;
    struct MoodEntry entry;

    fp = fopen("moods.dat", "rb");
    if (fp == NULL) {
        printf("\nNo notes found.\n");
        return;
    }

    printf("\n========== ALL NOTES ==========\n\n");

    while (fread(&entry, sizeof(entry), 1, fp) == 1) {
        struct tm *date = localtime(&entry.timestamp);

        printf("Date : %02d/%02d/%04d\n",
               date->tm_mday, date->tm_mon + 1, date->tm_year + 1900);
        printf("Note : %s\n", entry.note);
        printf("--------------------------------------\n");
    }

    fclose(fp);
}

int main() {
    int num;
    printMenu();
    scanf("%d", &num);

    switch (num) {
        case 1: addentry();      break;
        case 2: viewentry();     break;
        case 3: updateEntry();   break;   // fixed: was updateentry()
        case 4: weeklyreport();  break;
        case 5: monthlyreport(); break;
        case 6: moodpercentage();break;
        case 7: viewNotes();     break;   // fixed: was viewnotes()
        case 8: exit(0);         break;   // fixed: exit() needs an argument
        default:
            printf("Invalid choice.\n");
    }

    return 0;
}
