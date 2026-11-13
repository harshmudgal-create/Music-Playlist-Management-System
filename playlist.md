<!-- markdownlint-disable -->

# Music Playlist Management System — Explained Simply

**Subject:** DSA Lab | **Language:** C | **File:** `MusicPlaylist.c`
**Main concept:** Doubly Linked List

This document explains what the project does and how the code works, in plain language. You do not need to read the code first.

---

## 1. What is this project?

A music playlist app that runs in the terminal. You can add songs (title, artist, duration), delete or edit them, move to the next or previous song, search for a song, show the playlist from the first song to the last (or the other way round), and see the total playing time.

The special part is **how the songs are stored**: in a **doubly linked list**, where every song knows both the song after it and the song before it. That is exactly what "next" and "previous" buttons need.

Playback is **simulated**: the program prints `Now playing: ...`. No real audio is played. Everything lives in memory while the program runs; when you close it, the playlist is gone.

### The four modules

| # | Module | What it does | Menu option |
|---|--------|--------------|-------------|
| 1 | Playlist Creation | Add a song at the beginning, end or any position | 1, 2, 3 |
| 2 | Song Deletion and Update | Delete a song, edit song details, clear the playlist | 4, 5, 6 |
| 3 | Playback Navigation | Play from start / end, next song, previous song | 7, 8, 9, 10 |
| 4 | Search and Display | Search by title / artist, display forward / backward, total duration | 11, 12, 13, 14, 15 |

Menu option 16 loads five sample songs, so you can demonstrate the project without typing everything.

---

## 2. How to run it

```bash
gcc MusicPlaylist.c -o musicplaylist
./musicplaylist
```

On Windows the program is `musicplaylist.exe`.

---

## 3. What is a doubly linked list? (the key idea)

A **singly linked list** is like a treasure hunt where each clue only tells you where the *next* clue is. You can never go back.

A **doubly linked list** has two clues in every node: where the **next** one is *and* where the **previous** one is. You can walk in both directions.

```
NULL ◄── [ Song 1 ] ◄──► [ Song 2 ] ◄──► [ Song 3 ] ──► NULL
            ▲ head                          ▲ tail
```

- **`head`** points to the first song and **`tail`** points to the last song.
- The first song's `prev` is `NULL`, and the last song's `next` is `NULL`.
- An **empty list** is when `head` and `tail` are both `NULL`.
- **`current`** points to the song that is playing right now (`NULL` means nothing is playing).

**Why use it here?**

| Need | How the doubly linked list helps |
|------|----------------------------------|
| Next song | `current = current->next` (one step) |
| Previous song | `current = current->prev` (one step). A singly linked list cannot do this |
| Add at the end / play from the end | The `tail` pointer gets there instantly |
| Delete a song | Link its two neighbours to each other; no need to search for the node before it |
| Unknown number of songs | The list grows one node at a time with `malloc` |

---

## 4. The node: what one song looks like

```c
struct Song {
    int id;                 // automatic number: 1, 2, 3...
    char title[31];         // up to 30 characters
    char artist[31];
    int minutes, seconds;   // duration
    struct Song *prev;      // pointer to the previous song
    struct Song *next;      // pointer to the next song
};
```

The last two lines make it a *doubly* linked list.

Global variables that describe the playlist:

```c
struct Song *head = NULL;      // first song
struct Song *tail = NULL;      // last song
struct Song *current = NULL;   // song playing now
int nextId = 1;                // next song ID to hand out
int count = 0;                 // number of songs
```

---

## 5. Walking through the code

### 5.1 Helper functions

| Function | What it does |
|----------|--------------|
| `readInt(msg)` | Prints a message and reads a number. If the user types letters, it asks again, so the program never gets stuck |
| `readText(msg, dest)` | Reads a title or artist with spaces. Rejects empty input and text longer than 30 characters |
| `readDuration()` | Keeps asking for minutes and seconds until they are valid (0-99 minutes, 0-59 seconds, not 0:00) |
| `isDuplicate()` | Checks whether the same title and artist are already in the playlist (ignores upper/lower case) |
| `findSong(id)` | Walks the list from `head` until it finds the ID or reaches `NULL` |
| `containsIgnoreCase()` | Checks whether a keyword appears inside a text, ignoring case. Used by search |
| `printHeader()` / `printRow()` | Print the table. The playing song gets a `>` mark |

### 5.2 Module 1: adding songs

`createSong()` reads the details, checks for a duplicate, and uses `malloc` to make the new node. Then one of three functions links it in.

**Add at beginning**

```
Before:  head ► [B] ◄► [C]
Add A:   head ► [A] ◄► [B] ◄► [C]
```

New node's `next` = old head; old head's `prev` = new node; `head` = new node. If the list was empty, `tail` also becomes the new node.

**Add at end** is the mirror image and uses `tail`, so no walking is needed.

**Add at a position (say position 3)**

```
Before:  [A] ◄► [B] ◄► [D]          p points to B (the node just before position 3)
Step 1:  n->prev = p;               (C looks back at B)
Step 2:  n->next = p->next;         (C looks forward at D)
Step 3:  p->next->prev = n;         (D looks back at C)
Step 4:  p->next = n;               (B looks forward at C)
After:   [A] ◄► [B] ◄► [C] ◄► [D]
```

The order matters. If you did step 4 first, you would lose the link to D. Position 1 and position `count + 1` are handled by the beginning and end cases.

### 5.3 Module 2: delete, edit, clear

**`deleteSong()`** is the classic doubly linked list operation. To remove `s`, the song before it and the song after it are linked to each other:

```
Delete B:
Before:  [A] ◄► [B] ◄► [C]
Step 1:  s->prev->next = s->next;   (A now points forward to C)
Step 2:  s->next->prev = s->prev;   (C now points back to A)
After:   [A] ◄► [C]                  then free(B)
```

Special cases:
- Deleting the **first** song: there is no `s->prev`, so `head` moves to `s->next`.
- Deleting the **last** song: there is no `s->next`, so `tail` moves to `s->prev`.
- If the deleted song was the one **playing**, `current` is set to `NULL` and playback stops. Otherwise `current` would point to freed memory.

**`editSong()`** finds the song by ID and lets you change the title, the artist or the duration. A change that would make a duplicate is rejected.

**`clearPlaylist()`** asks for confirmation, then frees every node and resets `head`, `tail`, `current`, `count` and the ID counter.

### 5.4 Module 3: playback navigation

`current` is the "play head". The functions only move it and print a message.

| Function | What it does | Edge case |
|----------|--------------|-----------|
| `playFromStart()` | `current = head` | "Playlist is empty!" |
| `playFromEnd()` | `current = tail` | "Playlist is empty!" |
| `playNext()` | `current = current->next` | At the last song: "End of playlist!" and it stays there. If nothing is playing, it starts from the first song |
| `playPrevious()` | `current = current->prev` | At the first song: "Start of playlist!" and it stays there. If nothing is playing, it starts from the last song |

After each move, `showNowPlaying()` prints the song and its track number (for example `Track 2 of 5`).

### 5.5 Module 4: search and display

**`searchSongs()`** walks the list and prints every song whose title or artist contains the keyword. Case does not matter ("TUM" finds "Tum Hi Ho").

**`displayForward()`** starts at `head` and follows `next` pointers. **`displayBackward()`** starts at `tail` and follows `prev` pointers. This second one is only possible because the list is doubly linked.

**`showTotalDuration()`** adds all durations in seconds, then prints them as minutes and seconds (or hours, minutes and seconds).

### 5.6 `main()`

A `do { ... } while (choice != 0);` loop shows the menu repeatedly, and a `switch` calls the right function. When the user exits, `freeAll()` walks the list and frees every node, so no memory is leaked.

---

## 6. Example run

```
Enter choice: 16
5 sample song(s) added.
Enter choice: 7

>> Now playing: [1] Kesariya - Arijit Singh (4:28)   Track 1 of 5
Enter choice: 9

>> Now playing: [2] Shape of You - Ed Sheeran (3:53)   Track 2 of 5
Enter choice: 10

>> Now playing: [1] Kesariya - Arijit Singh (4:28)   Track 1 of 5
Enter choice: 10
Start of playlist! This is the first song.
```

---

## 7. Speed of each operation (time complexity)

Let **n** = number of songs. "O(1)" means the work does not grow with the playlist size; "O(n)" means we may have to look at every node.

| Operation | Time | Why |
|-----------|------|-----|
| Add at beginning / end | O(1) | `head` and `tail` are known |
| Add at a position | O(n) | Must walk to the position |
| Delete / edit | O(n) | Walk to find the ID; unlinking itself is O(1) |
| Next / previous | O(1) | One pointer step |
| Play from start / end | O(1) | Jump to `head` / `tail` |
| Search | O(n x m) | Check every title / artist (m = text length) |
| Display / total duration | O(n) | Visit every node once |

Memory used: O(n), one node per song.

---

## 8. Key terms

| Term | Meaning |
|------|---------|
| Node | One item of the list (here, one song) |
| Pointer | A variable that stores the memory address of something else |
| `head` / `tail` | Pointers to the first / last node |
| `prev` / `next` | Pointers to the previous / next node |
| `NULL` | "Points to nothing"; marks the end of the list |
| `malloc` / `free` | Ask the system for memory / give it back |
| Traversal | Walking through the list node by node |
| Doubly linked | Each node knows both its next and previous node |
| Dangling pointer | A pointer to memory that was already freed (we avoid this by clearing `current` on delete) |

---

## 9. Limitations

- Songs are not saved; they disappear when the program closes.
- Titles and artists are limited to 30 characters.
- No real audio is played, and there is no timer that moves to the next song automatically.
- Search is a simple scan of the whole list.

**Possible improvements:** save to a file; real audio playback; shuffle and repeat; a circular list so "next" after the last song wraps around; sorting by title, artist or duration.
