# Music Playlist Management System

> DSA Lab Project · C

A menu-driven console application in C that stores songs in a **doubly linked list**. The user can add songs at the beginning, end or any position, delete and edit them, move to the next or previous song, search, display the playlist forward and backward, and see the total duration.

**Main DSA concept:** Doubly Linked List

For a beginner-friendly walkthrough of the code, see [playlist.md](playlist.md).

> Playback is **simulated**: the program prints "Now playing" for the selected song. No audio files are used.

---

## Modules

| # | Module | What it does | Menu options |
|---|--------|--------------|--------------|
| 1 | Playlist Creation | Add a song (title, artist, duration) at the beginning, end or any position | 1, 2, 3 |
| 2 | Song Deletion and Update | Delete a song, edit its title / artist / duration, clear the playlist | 4, 5, 6 |
| 3 | Playback Navigation | Play from start or end, play the next and previous song using the `next` / `prev` pointers | 7, 8, 9, 10 |
| 4 | Search and Display | Search by title or artist, display forward and backward, show total duration | 11, 12, 13, 14, 15 |

Menu option 16 loads five sample songs so the project can be demonstrated quickly.

## Data structure

Each song is one node of the list:

```c
struct Song {
    int id;                 // auto-generated, starts at 1
    char title[31];
    char artist[31];
    int minutes, seconds;   // duration
    struct Song *prev;      // link to the previous song
    struct Song *next;      // link to the next song
};
```

Four global variables describe the playlist:

| Variable | Meaning |
|----------|---------|
| `head` | first song (`NULL` when the playlist is empty) |
| `tail` | last song |
| `current` | song that is playing now (`NULL` = nothing playing) |
| `count` | number of songs |

```
        NULL <-- [1|Kesariya] <--> [2|Shape of You] <--> [3|Believer] --> NULL
                    ^ head                                   ^ tail
```

**Why a doubly linked list?** Playback needs to move both ways. With `next` and `prev` pointers, "next song" and "previous song" take one step each. A `tail` pointer makes adding at the end and playing from the end instant, and a song can be unlinked without searching for the node before it.

## How each operation works

**Add at beginning.** The new node's `next` points to the old `head`, the old head's `prev` points back to the new node, and `head` moves to the new node. If the list was empty, the new node is also `tail`.

**Add at end.** The new node's `prev` points to `tail`, the old tail's `next` points to the new node, and `tail` moves. No walking through the list is needed.

**Add at position.** The code walks to the node just before the chosen position (`p`) and then connects four links in a safe order:

```
n->prev = p;   n->next = p->next;   p->next->prev = n;   p->next = n;
```

Positions 1 and `count + 1` reuse the beginning and end cases.

**Delete.** The neighbours are linked to each other (`s->prev->next = s->next` and `s->next->prev = s->prev`), then the node is released with `free`. If the first or last song is deleted, `head` or `tail` is moved instead. If the deleted song was playing, playback stops.

**Edit.** `findSong(id)` walks the list to the song, then the chosen field is changed. A title/artist change that would create a duplicate is rejected.

**Playback.** `current` always points to the playing song. Next is `current = current->next` and previous is `current = current->prev`. At the last or first song the program says so and stays there.

**Search.** Walks the list and prints every song whose title (or artist) contains the keyword, ignoring case.

**Display.** Forward display starts at `head` and follows `next`. Backward display starts at `tail` and follows `prev`. The playing song is marked with `>`.

**Total duration.** Adds up all durations in seconds and prints them as minutes and seconds (or hours, minutes and seconds).

## Complexity (n = number of songs)

| Operation | Time | Note |
|-----------|------|------|
| Add at beginning / end | O(1) | `head` and `tail` pointers |
| Add at position | O(n) | walk to the position |
| Delete / edit | O(n) | O(n) to find by ID, O(1) to unlink |
| Clear playlist | O(n) | frees every node |
| Play from start / end | O(1) | uses `head` / `tail` |
| Next / previous | O(1) | one pointer step (the "Track x of n" label costs O(n)) |
| Search | O(n x m) | m = length of the title / artist |
| Display forward / backward | O(n) | |
| Total duration | O(n) | |

Space used is O(n): one node per song.

## Input validation

- Non-numeric input is rejected and asked again instead of crashing.
- Titles and artists cannot be empty and are limited to 30 characters (spaces allowed).
- Duration must be 0-99 minutes and 0-59 seconds, and not 0:00.
- Position must be between 1 and `count + 1`.
- The same title and artist cannot be added twice (case-insensitive), including through edit.
- Delete and edit report "Song not found!" for an unknown ID, and every operation reports "Playlist is empty!" when there is nothing to work on.
- Clearing the playlist asks for confirmation.
- All nodes are freed when the program exits.

## Build and run

```
gcc MusicPlaylist.c -o musicplaylist
./musicplaylist          # on Windows: musicplaylist.exe
```

## Menu

```
===== MUSIC PLAYLIST MANAGEMENT SYSTEM =====
--- Module 1: Playlist Creation ---
 1. Add song at beginning
 2. Add song at end
 3. Add song at a position
--- Module 2: Song Deletion and Update ---
 4. Delete song
 5. Edit song details
 6. Clear playlist
--- Module 3: Playback Navigation ---
 7. Play from start
 8. Play from end
 9. Next song
10. Previous song
--- Module 4: Search and Display ---
11. Search song by title
12. Search song by artist
13. Display playlist (forward)
14. Display playlist (backward)
15. Show total duration
--- Extra ---
16. Load sample songs
 0. Exit
```

## Sample output

```
Enter choice: 16
5 sample song(s) added.
Enter choice: 13

--- Playlist: first to last ---
  Pos  ID   Title                          Artist                         Duration
--------------------------------------------------------------------------------
  1    1    Kesariya                       Arijit Singh                   4:28
  2    2    Shape of You                   Ed Sheeran                     3:53
  3    3    Blinding Lights                The Weeknd                     3:20
  4    4    Tum Hi Ho                      Arijit Singh                   4:22
  5    5    Believer                       Imagine Dragons                3:24
Total songs: 5   (> = now playing)
Enter choice: 7

>> Now playing: [1] Kesariya - Arijit Singh (4:28)   Track 1 of 5
Enter choice: 9

>> Now playing: [2] Shape of You - Ed Sheeran (3:53)   Track 2 of 5
Enter choice: 9

>> Now playing: [3] Blinding Lights - The Weeknd (3:20)   Track 3 of 5
Enter choice: 10

>> Now playing: [2] Shape of You - Ed Sheeran (3:53)   Track 2 of 5
Enter choice: 11
Enter title keyword: tum

  Pos  ID   Title                          Artist                         Duration
--------------------------------------------------------------------------------
  4    4    Tum Hi Ho                      Arijit Singh                   4:22
Enter choice: 3
Enter position (1 to 6): 2
Enter Title: Zinda
Enter Artist: Farhan Akhtar
Enter Duration - Minutes: 4
Enter Duration - Seconds (0-59): 10
Song added at position 2! Song ID: 6
Enter choice: 14

--- Playlist: last to first ---
  Pos  ID   Title                          Artist                         Duration
--------------------------------------------------------------------------------
  6    5    Believer                       Imagine Dragons                3:24
  5    4    Tum Hi Ho                      Arijit Singh                   4:22
  4    3    Blinding Lights                The Weeknd                     3:20
> 3    2    Shape of You                   Ed Sheeran                     3:53
  2    6    Zinda                          Farhan Akhtar                  4:10
  1    1    Kesariya                       Arijit Singh                   4:28
Total songs: 6   (> = now playing)
Enter choice: 4
Enter Song ID to delete: 1
Song deleted!
Enter choice: 15
Total songs   : 5
Total duration: 19m 09s
Enter choice: 0
Exiting program...
```

## Limitations

- Songs are kept in memory only and are lost when the program exits.
- Title and artist length is fixed at 30 characters.
- Playback is simulated: there is no real audio and no timer that moves to the next song automatically.
- Search is by keyword in the title or artist only, using a linear scan.

## Possible extensions

- Save and load the playlist from a file.
- Store a file path in each node and play real audio (for example with the Windows MCI API, or a library such as miniaudio or SDL_mixer). The linked list only decides the order; a separate audio library does the playing.
- Shuffle and repeat modes, or a circular doubly linked list so "next" after the last song wraps to the first.
- Sort the playlist by title, artist or duration.

## Author

Harsh Mudgal · [GitHub](https://github.com/harshmudgal-create)
