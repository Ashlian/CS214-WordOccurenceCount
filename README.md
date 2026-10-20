# CS214-WordOccurenceCount
Project 2 of CS214 Systems Programming

By: Jeffrey Lu (jl3925) & Alexander Wong (amw408)

Run Instructions:

Design Notes: In order to 

Test Plan:

1. Word rules: one file mixing case variants (“foo”, “Foo”), digits inside words (“a2z”), assorted punctuation, apostrophes at the start, middle, and end of words, valid hyphens (“foo-bar”, “a-b-c”), invalid hyphens (“foo--bar”, “foo-”, “-foo”, “foo-‘bar”), and a mix of spaces, tabs, and CRLF line endings, with no trailing newline. Verifies word splitting, case sensitivity, and that the last word is counted.

foo 2
'oo 1
'y 1
FOO 1
Foo 1
a 1
a-b-c 1
bar 1
colon 1
dogs' 1
don't 1
double 1
end 1
foo-bar 1
hello 1
it''s 1
last 1
semi 1
separated 1
spaces 1
start 1
tab 1
wait 1
world 1
x 1
z 1

3. Sorting and output format: one file with words repeated different numbers of times, including several ties that mix uppercase, lowercase, and apostrophe-initial words. Verifies descending count order, lexicographic tie-breaking, and the “word count” line format.

the 4
Cat 3
cat 3
'dog 2
Dog 2
dog 2
zebra 2
'zebra 1
Apple 1
an 1
and 1
apple 1

4. Buffer boundaries: one large file containing a word longer than the read buffer, words and a hyphen placed to straddle buffer boundaries, and enough distinct words to force the word map to grow. Verifies parsing across read() calls, unbounded word length, large inputs, and correct counts with many unique words.

Anothersuperlongwordthatshouldbeallreadasoneword 1
Hithisisasuperlongwordthatissupposedtobelargerthanthebuffersizetotestdynamicbuffersize 1
dadjakdadad 1
jumbleofcharactersjmdoinwasdnwausdwasdwdwansdwghkadda 1

5. Empty input: an empty file and a file of only digits, whitespace, and punctuation given together. Verifies that no output is produced and nothing crashes.

Nothing

6. Multiple file arguments: several files sharing some words, including one starting with a period and one with no extension. Verifies counts are totaled across files and that explicit files are processed regardless of name.

file 8
that 6
the 6
to 5
is 4
multi 4
I 3
a 3
enough 3
just 3
other 3
test 3
this 3
Hi 2
am 2
be 2
of 2
part 2
so 2
with 2
writing 2
However 1
That's 1
This 1
again 1
also 1
anything 1
can 1
come 1
differently 1
does 1
filler 1
in 1
isn't 1
it 1
little 1
long 1
make 1
maybe 1
mean 1
meaningful 1
mind 1
need 1
not 1
order 1
overlap 1
overlaps 1
probably 1
random 1
same 1
should 1
supposed 1
than 1
things 1
unique 1
why 1
words 1
write 1

7. Directory traversal: a directory tree containing nested subdirectories several levels deep, matching and non-matching file names, hidden files and hidden subdirectories, subdirectories whose names don’t end in the suffix, an empty subdirectory, and a file sharing its name with one in the working directory. Run with and without a trailing slash. Verifies recursion, suffix filtering with the default .txt, skipping hidden entries, and correct path construction.

the 19
directory 14
file 14
in 10
deepest 7
is 7
This 5
be 4
if 4
to 4
a 3
all 3
and 3
that 3
through 3
even 2
md 2
read 2
should 2
traveled 2
travesal 2
txt 2
weird 2
I 1
Should 1
Weird 1
access 1
actually 1
after 1
are 1
as 1
at 1
bait 1
because 1
comes 1
correctly 1
counted 1
deep 1
directories 1
don't 1
else 1
end 1
files 1
for 1
from 1
goes 1
here 1
inside 1
its 1
know 1
needs 1
no 1
of 1
parent 1
recursive 1
scans 1
see 1
since 1
still 1
subdirectories 1
subdirectory 1
suffix 1
tests 1
this 1
tho 1
traversal 1
tree 1
weirdness 1
what 1
will 1
works 1
write 1


8. Suffix option: the same directory tree from test 7, plus some .md files and a file ending in “md” without a dot, run with -s .md, -s.md, and -s md. Verifies both argument forms behave identically, no dot is inserted, and an explicitly named .txt file is still processed alongside the directory.

md 6
the 6
file 4
This 3
is 3
not 3
should 3
an 2
and 2
be 2
command 2
default 2
in 2
read 2
suffix 2
that 2
to 2
txt 2
'typo 1
'words 1
More 1
actual 1
at 1
but 1
directory 1
filler 1
its 1
ne 1
now 1
only 1
pad 1
parent 1
similarly 1
this 1
with 1


9. Robustness: a run including a nonexistent file and an unreadable file alongside valid ones, with all tests also run under AddressSanitizer or Valgrind. Verifies error handling without crashing, continued processing of remaining arguments, and memory correctness.

File perms denied

chmod 000 test/robust/locked.txt;
chmod 000 test/robust/subdir/noread.txt;
chmod 000 test/robust/subdir/closed  