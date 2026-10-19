# CS214-WordOccurenceCount
Project 2 of CS214 Systems Programming

By: Jeffrey Lu (jl3925) & Alexander Wong (amw408)

Test Plan:

1. Word rules: one file mixing case variants (“foo”, “Foo”), digits inside words (“a2z”), assorted punctuation, apostrophes at the start, middle, and end of words, valid hyphens (“foo-bar”, “a-b-c”), invalid hyphens (“foo--bar”, “foo-”, “-foo”, “foo-‘bar”), and a mix of spaces, tabs, and CRLF line endings, with no trailing newline. Verifies word splitting, case sensitivity, and that the last word is counted.

2. Spec example: the exact text from section 2.1. Verifies the output matches the spec character for character, as a baseline check.

3. Sorting and output format: one file with words repeated different numbers of times, including several ties that mix uppercase, lowercase, and apostrophe-initial words. Verifies descending count order, lexicographic tie-breaking, and the “word count” line format.

4. Buffer boundaries: one large file containing a word longer than the read buffer, words and a hyphen placed to straddle buffer boundaries, and enough distinct words to force the word map to grow. Verifies parsing across read() calls, unbounded word length, large inputs, and correct counts with many unique words.

5. Empty input: an empty file and a file of only digits, whitespace, and punctuation given together. Verifies that no output is produced and nothing crashes.

6. Multiple file arguments: several files sharing some words, including one starting with a period and one with no extension. Verifies counts are totaled across files and that explicit files are processed regardless of name.

7. Directory traversal: a directory tree containing nested subdirectories several levels deep, matching and non-matching file names, hidden files and hidden subdirectories, subdirectories whose names don’t end in the suffix, an empty subdirectory, and a file sharing its name with one in the working directory. Run with and without a trailing slash. Verifies recursion, suffix filtering with the default .txt, skipping hidden entries, and correct path construction.

8. Suffix option: the same directory tree from test 7, plus some .md files and a file ending in “md” without a dot, run with -s .md, -s.md, and -s md. Verifies both argument forms behave identically, no dot is inserted, and an explicitly named .txt file is still processed alongside the directory.

9. Robustness: a run including a nonexistent file and an unreadable file alongside valid ones, with all tests also run under AddressSanitizer or Valgrind. Verifies error handling without crashing, continued processing of remaining arguments, and memory correctness.