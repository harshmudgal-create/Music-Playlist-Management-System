/*
 * MUSIC PLAYLIST MANAGEMENT SYSTEM
 * --------------------------------
 * DSA Lab Project | Language: C | Data structure: Doubly Linked List
 *
 * Module 1 - Playlist Creation          : add a song at the beginning, end or any position
 * Module 2 - Song Deletion and Update   : delete a song, edit song details, clear the playlist
 * Module 3 - Playback Navigation        : play from start/end, next and previous song
 * Module 4 - Search and Display         : search by title/artist, display forward/backward,
 *                                         show total duration
 *
 * Playback is simulated (the program prints "Now playing"); no audio files are used.
 *
 * Build : gcc MusicPlaylist.c -o musicplaylist
 * Run   : ./musicplaylist        (Windows: musicplaylist.exe)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

#define LEN 31   /* 30 characters + '\0' */

struct Song {
    int id;
    char title[LEN];
    char artist[LEN];
    int minutes, seconds;   /* duration */
    struct Song *prev;      /* link to the previous song */
    struct Song *next;      /* link to the next song */
};

struct Song *head = NULL;      /* first song */
struct Song *tail = NULL;      /* last song */
struct Song *current = NULL;   /* song that is playing now (NULL = nothing playing) */
int nextId = 1;
int count = 0;

void freeAll(void);

/* ====================================================================
 *  INPUT HELPERS
 * ==================================================================== */

void quit(void) {          /* used when input ends (Ctrl+D / Ctrl+Z) */
    freeAll();
    exit(0);
}

void trim(char *s) {
    int start = 0, end = (int)strlen(s);

    while (s[start] && isspace((unsigned char)s[start]))
        start++;
    while (end > start && isspace((unsigned char)s[end - 1]))
        end--;
    memmove(s, s + start, end - start);
    s[end - start] = '\0';
}

int readInt(const char *msg) {
    char buf[64], *end;
    long x;
    int c;

    for (;;) {
        printf("%s", msg);
        if (fgets(buf, sizeof(buf), stdin) == NULL)
            quit();
        if (strchr(buf, '\n') == NULL)                       /* line was too long: discard the rest */
            while ((c = getchar()) != '\n' && c != EOF);

        x = strtol(buf, &end, 10);
        while (isspace((unsigned char)*end))
            end++;
        if (end == buf || *end != '\0' || x < INT_MIN || x > INT_MAX) {
            printf("Invalid input! ");
            continue;
        }
        return (int)x;
    }
}

void readText(const char *msg, char *dest) {
    char buf[256];
    size_t len;
    int c;

    for (;;) {
        printf("%s", msg);
        if (fgets(buf, sizeof(buf), stdin) == NULL)
            quit();
        len = strlen(buf);
        if (len > 0 && buf[len - 1] == '\n')
            buf[len - 1] = '\0';
        else
            while ((c = getchar()) != '\n' && c != EOF);

        trim(buf);
        len = strlen(buf);
        if (len == 0) {
            printf("Input cannot be empty!\n");
            continue;
        }
        if (len >= LEN) {
            printf("Too long! Maximum %d characters.\n", LEN - 1);
            continue;
        }
        strcpy(dest, buf);
        return;
    }
}

void readDuration(int *m, int *s) {
    for (;;) {
        *m = readInt("Enter Duration - Minutes: ");
        *s = readInt("Enter Duration - Seconds (0-59): ");
        if (*m >= 0 && *m <= 99 && *s >= 0 && *s <= 59 && (*m > 0 || *s > 0))
            return;
        printf("Invalid duration! Minutes must be 0-99, seconds 0-59, and the song cannot be 0:00.\n");
    }
}

/* ====================================================================
 *  GENERAL HELPERS
 * ==================================================================== */

int equalsIgnoreCase(const char *a, const char *b) {
    while (*a && *b && tolower((unsigned char)*a) == tolower((unsigned char)*b)) {
        a++;
        b++;
    }
    return *a == '\0' && *b == '\0';
}

int containsIgnoreCase(const char *text, const char *key) {
    char t[LEN], k[LEN];
    int i;

    for (i = 0; text[i]; i++) t[i] = (char)tolower((unsigned char)text[i]);
    t[i] = '\0';
    for (i = 0; key[i]; i++) k[i] = (char)tolower((unsigned char)key[i]);
    k[i] = '\0';

    return strstr(t, k) != NULL;
}

/* same title + same artist already in the list? (skip = a node to ignore, may be NULL) */
int isDuplicate(const char *title, const char *artist, struct Song *skip) {
    struct Song *s;

    for (s = head; s != NULL; s = s->next)
        if (s != skip && equalsIgnoreCase(s->title, title) && equalsIgnoreCase(s->artist, artist))
            return 1;
    return 0;
}

struct Song *findSong(int id) {
    struct Song *s = head;

    while (s != NULL && s->id != id)
        s = s->next;
    return s;
}

int positionOf(struct Song *target) {
    struct Song *s;
    int pos = 1;

    for (s = head; s != NULL; s = s->next, pos++)
        if (s == target)
            return pos;
    return 0;
}

void printHeader(void) {
    printf("\n  %-4s %-4s %-30s %-30s %s\n", "Pos", "ID", "Title", "Artist", "Duration");
    printf("--------------------------------------------------------------------------------\n");
}

void printRow(struct Song *s, int pos) {
    printf("%c %-4d %-4d %-30s %-30s %d:%02d\n", (s == current) ? '>' : ' ', pos, s->id,
           s->title, s->artist, s->minutes, s->seconds);
}

void freeAll(void) {
    struct Song *s;

    while (head != NULL) {
        s = head;
        head = head->next;
        free(s);
    }
    tail = NULL;
    current = NULL;
    count = 0;
}

/* ====================================================================
 *  MODULE 1: PLAYLIST CREATION
 * ==================================================================== */

/* reads the song details and allocates a node; returns NULL if it could not be created */
struct Song *createSong(void) {
    char title[LEN], artist[LEN];
    int m, s;
    struct Song *n;

    readText("Enter Title: ", title);
    readText("Enter Artist: ", artist);
    if (isDuplicate(title, artist, NULL)) {
        printf("This song is already in the playlist!\n");
        return NULL;
    }
    readDuration(&m, &s);

    n = (struct Song *)malloc(sizeof(struct Song));
    if (n == NULL) {
        printf("Memory allocation failed!\n");
        return NULL;
    }
    n->id = nextId++;
    strcpy(n->title, title);
    strcpy(n->artist, artist);
    n->minutes = m;
    n->seconds = s;
    n->prev = NULL;
    n->next = NULL;
    return n;
}

void insertAtBeginning(struct Song *n) {
    n->prev = NULL;
    n->next = head;
    if (head != NULL)
        head->prev = n;
    else
        tail = n;               /* list was empty: the new node is also the tail */
    head = n;
    count++;
}

void insertAtEnd(struct Song *n) {
    n->next = NULL;
    n->prev = tail;
    if (tail != NULL)
        tail->next = n;
    else
        head = n;               /* list was empty: the new node is also the head */
    tail = n;
    count++;
}

/* pos is between 1 and count + 1 */
void insertAtPosition(struct Song *n, int pos) {
    struct Song *p;
    int i;

    if (pos == 1) {
        insertAtBeginning(n);
        return;
    }
    if (pos == count + 1) {
        insertAtEnd(n);
        return;
    }
    p = head;                               /* find the node just before the new position */
    for (i = 1; i < pos - 1; i++)
        p = p->next;

    n->prev = p;
    n->next = p->next;
    p->next->prev = n;
    p->next = n;
    count++;
}

void addAtBeginning(void) {
    struct Song *n = createSong();

    if (n == NULL)
        return;
    insertAtBeginning(n);
    printf("Song added at the beginning! Song ID: %d\n", n->id);
}

void addAtEnd(void) {
    struct Song *n = createSong();

    if (n == NULL)
        return;
    insertAtEnd(n);
    printf("Song added at the end! Song ID: %d\n", n->id);
}

void addAtPosition(void) {
    char msg[64];
    int pos;
    struct Song *n;

    snprintf(msg, sizeof(msg), "Enter position (1 to %d): ", count + 1);
    do {
        pos = readInt(msg);
        if (pos < 1 || pos > count + 1)
            printf("Invalid position!\n");
    } while (pos < 1 || pos > count + 1);

    n = createSong();
    if (n == NULL)
        return;
    insertAtPosition(n, pos);
    printf("Song added at position %d! Song ID: %d\n", pos, n->id);
}

/* ====================================================================
 *  MODULE 2: SONG DELETION AND UPDATE
 * ==================================================================== */

void deleteSong(void) {
    struct Song *s;

    if (head == NULL) {
        printf("Playlist is empty!\n");
        return;
    }
    s = findSong(readInt("Enter Song ID to delete: "));
    if (s == NULL) {
        printf("Song not found!\n");
        return;
    }

    /* relink the neighbours around s */
    if (s->prev != NULL)
        s->prev->next = s->next;
    else
        head = s->next;         /* deleting the first song */
    if (s->next != NULL)
        s->next->prev = s->prev;
    else
        tail = s->prev;         /* deleting the last song */

    if (current == s) {
        current = NULL;
        printf("The song that was playing has been removed. Playback stopped.\n");
    }
    free(s);
    count--;
    printf("Song deleted!\n");
}

void editSong(void) {
    struct Song *s;
    char text[LEN];
    int ch, m, sec;

    if (head == NULL) {
        printf("Playlist is empty!\n");
        return;
    }
    s = findSong(readInt("Enter Song ID to edit: "));
    if (s == NULL) {
        printf("Song not found!\n");
        return;
    }

    printf("1. Title\n2. Artist\n3. Duration\n");
    ch = readInt("What do you want to edit? ");

    if (ch == 1) {
        readText("Enter new Title: ", text);
        if (isDuplicate(text, s->artist, s)) {
            printf("This song is already in the playlist!\n");
            return;
        }
        strcpy(s->title, text);
    } else if (ch == 2) {
        readText("Enter new Artist: ", text);
        if (isDuplicate(s->title, text, s)) {
            printf("This song is already in the playlist!\n");
            return;
        }
        strcpy(s->artist, text);
    } else if (ch == 3) {
        readDuration(&m, &sec);
        s->minutes = m;
        s->seconds = sec;
    } else {
        printf("Invalid choice!\n");
        return;
    }
    printf("Song updated!\n");
}

void clearPlaylist(void) {
    if (head == NULL) {
        printf("Playlist is already empty!\n");
        return;
    }
    if (readInt("Clear the whole playlist? (1 = Yes, 0 = No): ") != 1) {
        printf("Cancelled.\n");
        return;
    }
    freeAll();
    nextId = 1;
    printf("Playlist cleared!\n");
}

/* ====================================================================
 *  MODULE 3: PLAYBACK NAVIGATION
 * ==================================================================== */

void showNowPlaying(void) {
    printf("\n>> Now playing: [%d] %s - %s (%d:%02d)   Track %d of %d\n", current->id,
           current->title, current->artist, current->minutes, current->seconds,
           positionOf(current), count);
}

void playFromStart(void) {
    if (head == NULL) {
        printf("Playlist is empty!\n");
        return;
    }
    current = head;
    showNowPlaying();
}

void playFromEnd(void) {
    if (head == NULL) {
        printf("Playlist is empty!\n");
        return;
    }
    current = tail;
    showNowPlaying();
}

void playNext(void) {
    if (head == NULL) {
        printf("Playlist is empty!\n");
        return;
    }
    if (current == NULL) {
        printf("No song is playing. Starting from the first song.\n");
        current = head;
    } else if (current->next == NULL) {
        printf("End of playlist! This is the last song.\n");
        return;
    } else {
        current = current->next;
    }
    showNowPlaying();
}

void playPrevious(void) {
    if (head == NULL) {
        printf("Playlist is empty!\n");
        return;
    }
    if (current == NULL) {
        printf("No song is playing. Starting from the last song.\n");
        current = tail;
    } else if (current->prev == NULL) {
        printf("Start of playlist! This is the first song.\n");
        return;
    } else {
        current = current->prev;
    }
    showNowPlaying();
}

/* ====================================================================
 *  MODULE 4: SEARCH AND DISPLAY
 * ==================================================================== */

void searchSongs(int byTitle) {
    char key[LEN];
    struct Song *s;
    int found = 0, pos = 1;

    if (head == NULL) {
        printf("Playlist is empty!\n");
        return;
    }
    readText(byTitle ? "Enter title keyword: " : "Enter artist keyword: ", key);

    for (s = head; s != NULL; s = s->next, pos++) {
        if (containsIgnoreCase(byTitle ? s->title : s->artist, key)) {
            if (!found)
                printHeader();
            printRow(s, pos);
            found = 1;
        }
    }
    if (!found)
        printf("No song matches the keyword!\n");
}

void displayForward(void) {
    struct Song *s;
    int pos = 1;

    if (head == NULL) {
        printf("Playlist is empty!\n");
        return;
    }
    printf("\n--- Playlist: first to last ---");
    printHeader();
    for (s = head; s != NULL; s = s->next)
        printRow(s, pos++);
    printf("Total songs: %d   (> = now playing)\n", count);
}

void displayBackward(void) {
    struct Song *s;
    int pos = count;

    if (head == NULL) {
        printf("Playlist is empty!\n");
        return;
    }
    printf("\n--- Playlist: last to first ---");
    printHeader();
    for (s = tail; s != NULL; s = s->prev)
        printRow(s, pos--);
    printf("Total songs: %d   (> = now playing)\n", count);
}

void showTotalDuration(void) {
    struct Song *s;
    long total = 0;

    for (s = head; s != NULL; s = s->next)
        total += (long)s->minutes * 60 + s->seconds;

    printf("Total songs   : %d\n", count);
    if (total >= 3600)
        printf("Total duration: %ldh %02ldm %02lds\n", total / 3600, (total % 3600) / 60, total % 60);
    else
        printf("Total duration: %ldm %02lds\n", total / 60, total % 60);
}

/* ====================================================================
 *  EXTRA: SAMPLE DATA (handy for a quick demo)
 * ==================================================================== */

void loadSampleSongs(void) {
    static const struct {
        const char *title, *artist;
        int m, s;
    } sample[] = {
        {"Kesariya", "Arijit Singh", 4, 28},
        {"Shape of You", "Ed Sheeran", 3, 53},
        {"Blinding Lights", "The Weeknd", 3, 20},
        {"Tum Hi Ho", "Arijit Singh", 4, 22},
        {"Believer", "Imagine Dragons", 3, 24}
    };
    int i, added = 0, total = (int)(sizeof(sample) / sizeof(sample[0]));
    struct Song *n;

    for (i = 0; i < total; i++) {
        if (isDuplicate(sample[i].title, sample[i].artist, NULL))
            continue;
        n = (struct Song *)malloc(sizeof(struct Song));
        if (n == NULL) {
            printf("Memory allocation failed!\n");
            return;
        }
        n->id = nextId++;
        strcpy(n->title, sample[i].title);
        strcpy(n->artist, sample[i].artist);
        n->minutes = sample[i].m;
        n->seconds = sample[i].s;
        insertAtEnd(n);
        added++;
    }
    printf("%d sample song(s) added.\n", added);
}

/* ====================================================================
 *  MAIN MENU
 * ==================================================================== */

void printMenu(void) {
    printf("\n===== MUSIC PLAYLIST MANAGEMENT SYSTEM =====\n");
    printf("--- Module 1: Playlist Creation ---\n");
    printf(" 1. Add song at beginning\n");
    printf(" 2. Add song at end\n");
    printf(" 3. Add song at a position\n");
    printf("--- Module 2: Song Deletion and Update ---\n");
    printf(" 4. Delete song\n");
    printf(" 5. Edit song details\n");
    printf(" 6. Clear playlist\n");
    printf("--- Module 3: Playback Navigation ---\n");
    printf(" 7. Play from start\n");
    printf(" 8. Play from end\n");
    printf(" 9. Next song\n");
    printf("10. Previous song\n");
    printf("--- Module 4: Search and Display ---\n");
    printf("11. Search song by title\n");
    printf("12. Search song by artist\n");
    printf("13. Display playlist (forward)\n");
    printf("14. Display playlist (backward)\n");
    printf("15. Show total duration\n");
    printf("--- Extra ---\n");
    printf("16. Load sample songs\n");
    printf(" 0. Exit\n");
}

int main(void) {
    int choice;

    do {
        printMenu();
        choice = readInt("Enter choice: ");

        switch (choice) {
            case 1:  addAtBeginning(); break;
            case 2:  addAtEnd(); break;
            case 3:  addAtPosition(); break;
            case 4:  deleteSong(); break;
            case 5:  editSong(); break;
            case 6:  clearPlaylist(); break;
            case 7:  playFromStart(); break;
            case 8:  playFromEnd(); break;
            case 9:  playNext(); break;
            case 10: playPrevious(); break;
            case 11: searchSongs(1); break;
            case 12: searchSongs(0); break;
            case 13: displayForward(); break;
            case 14: displayBackward(); break;
            case 15: showTotalDuration(); break;
            case 16: loadSampleSongs(); break;
            case 0:  printf("Exiting program...\n"); break;
            default: printf("Invalid choice!\n");
        }
    } while (choice != 0);

    freeAll();      /* free every node before exiting */
    return 0;
}
