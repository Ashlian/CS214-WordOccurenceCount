# CS214-WordOccurenceCount
Project 2 of CS214 Systems Programming

By: Jeffrey Lu (jl3925) & Alexander Wong (amw408)

# Run Instructions:
From the project root, run `make` to build to make the program. The executables are placed in the `build` directory. To remove all build outputs, run `make clean`

The program is run directly from the project root through `./build/words [-s suffix] file_or_directory`
- To read files, the program reads through the args and lists the words in descending order.
- To read directories, the program recursively traverses through all subdirectories and reads all target files and lists the words in descending order.
- The optional `-s` flag can be added with a file suffix added after to read only files of that type.

# Design Notes: 
`words` processes its input in two stages: first it collects every word from the input files, then it counts and sorts them.

1. **Gathering input:** The program reads the optional `-s` suffix and then goes through each argument. Files named on the command line are always read. Directories are searched recursively, skipping hidden entries and reading only files whose names end with the chosen suffix.
2. **Extracting words:** Each file is read in fixed-size chunks and scanned character by character, applying the spec's rules for letters, apostrophes, and hyphens. Words are built in a buffer that grows as needed, so there is no limit on word length, and words that span two chunks are handled correctly. Every completed word, from every file, is written on its own line to a single temporary file.
3. **Counting and sorting:** Once all input has been read, the temporary file is passed through a pipeline of standard Unix tools: `sort` groups identical words together, `uniq -c` counts each group, a second `sort` orders the results by count (highest first) and then alphabetically, and `awk` prints each line as the word followed by its count.
4. **Cleanup:** The temporary file is deleted before the program exits. If any file or directory can't be read, the program reports the error and stops.

# Test Strategy:

Here are the following tests we used to ensure correctness of the program:

**Test 1: Word rules**
- **How it's run:** `words` is given a single file mixing case variants, digits inside words, assorted punctuation, apostrophes at the start, middle, and end of words, valid and invalid hyphen placements, and a mix of spaces, tabs, and CRLF line endings, with no trailing newline.
- **Expected behavior:** Prints each word found under the rules: case variants are counted separately, digits and punctuation split words, apostrophes are kept anywhere in a word, hyphens are kept only between two letters, and all whitespace separates words. Also, the final word is counted even without a trailing newline.
- **Why this test:** Word splitting is the core of the program, and the hyphen and apostrophe rules have the most edge cases. Combining them in one file checks that the rules work together, not just individually.

**Test 2: Sorting and output format**
- **How it's run:** `words` is given a single file with words repeated different numbers of times, in an order unrelated to their final ranking, including several ties that mix uppercase, lowercase, and apostrophe-initial words.
- **Expected behavior:** Prints words in descending order of count, with ties broken in order: apostrophe-initial words, then uppercase, then lowercase. Each line is the word, one space, and its count.
- **Why this test:** Correct output for this requires actual sorting, and ensures that ties at several counts follow the tie-breaking rule correctly

**Test 3: Buffer boundaries**
- **How it's run:** `words` is given a single large file containing a word longer than the read buffer, words and a hyphen placed to straddle buffer boundaries, and many distinct words.
- **Expected behavior:** Counts every word correctly, including the oversized word and the words split across two `read()` calls.
- **Why this test:** Files are read in fixed-size chunks, so a word or hyphen split across chunks is easy to mishandle. It also checks that there's no maximum word length and that large inputs with many unique words are counted correctly.

**Test 4: Empty input**
- **How it's run:** `words` is given two files together: one completely empty, and one containing only digits, whitespace, and punctuation.
- **Expected behavior:** Prints nothing and exits normally.
- **Why this test:** Checks that files with no words don't cause crashes or produce stray output.

**Test 5: Multiple file arguments**
- **How it's run:** `words` is given several files that share some words, including one whose name begins with a period and one with no extension.
- **Expected behavior:** Prints combined counts across all files, including the hidden and extensionless ones.
- **Why this test:** Counts must be totaled across files rather than kept per file, and files named on the command line must be processed regardless of their names.

**Test 6: Directory traversal**
- **How it's run:** `words` is given a directory tree containing nested subdirectories several levels deep, matching and non-matching file names, hidden files and hidden subdirectories, subdirectories whose names don't end in the suffix, an empty subdirectory, and a file sharing its name with one in the working directory. It is run once with the directory name 
- **Expected behavior:** Processes only regular `.txt` files, recursing into all non-hidden subdirectories. Hidden entries are skipped, the empty subdirectory causes no errors, and the file inside the tree is read instead of the one with the same name in the working directory. 
- **Why this test:** Covers recursion, default suffix filtering, the hidden-entry exception, and correct path construction

**Test 7: Suffix option**
- **How it's run:** `words` is given the same directory tree, with the `-s .md` flag being put into the command to read only md files. It is run three times, with `-s .md`, `-s.md`, and `-s md`.
- **Expected behavior:** All three runs give the same output, only reading the words from the md files in the directory tree.
- **Why this test:** Checks both forms of the `-s` argument, and that suffix filtering applies only to directory traversal.

**Test 8: Mix of invalid and valid files**
- **How it's run:** `words` is given a nonexistent file and an unreadable file (with permissions removed using `chmod 000`, run as a non-root user) alongside valid files. All tests are also run under AddressSanitizer
- **Expected behavior:** Reports an error to standard error for the missing and unreadable files without crashing, and cleans up its temporary file. No memory errors or leaks are reported in any test.
- **Why this test:** This checks that failures are handled in a controlled, documented way. Running under a memory checker catches leaks and invalid accesses, especially in error paths.