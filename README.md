
**goal:**
build a simple implementation of a signature indexed file, including applications to create such files, insert tuples into them, and search for tuples based on partial-match retrieval queries.


**#structure**
Relations
In our system, a relation R is represented by five physical files:

R.info containing global information such as

the number of attributes and size of each tuple
the number of data pages and number of tuples
the base type of signatures (simc or catc)
the sizes of the various kinds of signatures
the number of signatures and signature pages
etc. etc. etc.
The R.info file contains a copy of the RelnParams structure given in the reln.h file (see below).

R.data containing data pages, where each data page contains

a count of the number of tuples in the page
the tuples (as comma-separated character sequences)
Each data page has a capacity of c tuples. If there are n tuples then there will be b = ⌈n/c⌉ pages in the data file. All pages except the last are full. Tuples are never deleted.

R.tsig containing tuple signatures, where each page contains

a count of the number of signatures in the page
the signatures themselves (as bit strings)
Each tuple signature is formed by incorporating the codewords from each attribute in the tuple. How this is done differs between SIMC and CATC, but the overall result is a single m-bit long signature. If there are n tuples in the relation, there will be n tuple signatures, in bt pages. All tuple signature pages except the last are full.

R.psig containing page signatures, where each page contains

a count of the number of signatures in the page
the signatures themselves (as bit strings)
Page signatures are much larger than tuple signatures, and are formed by incorporating the codewords of all attribute values in all tuples in the page. How this is done differs between SIMC and CATC, but the result is a single mp-bit long signature There is one page signature for each page in the data file.

R.bsig containing bit-sliced signatures, where each page contains

a count of the number of signatures in the page
the bit-slices themselves (as bit strings)
Bit-slices give an alternate 90o-rotated view of page signatures. If there are b data pages, then each bit-slice is b-bits long. If page signatures are pm bits long, then there are pm bit-slices.


****command:****

1.create RelName SigType #tuples #attrs 1/pF
The following example of using create makes a relation called abc where each tuple has 4 attributes and the indexing has a false match probability of 1/100. The relation can hold up to 10000 tuples (it can actually hold more, but only the first 10000 will be indexed via the bit-sliced signatures).

$ ./create  abc  simc  10000  4  100

2.insert RelName
Reads tuples, one per line, from standard input and inserts them into the relation specified on the command line. Tuples all take the form val1,val2,...,valn. The values can be any sequence of alpha-numeric characters and '-'. The characters ',' (field separator) and '?' (query wildcard) are treated specially.

Since all tuples need to be the same length, it is simplest to use gendata to generate them, and pipe the generated tuples into the insert command

3.select RelName QueryString IndexType
Takes a "query tuple" on the command line, and finds all tuples in the data pages of the relation RelName that match the query. IndexType has a value of either t, p or b, indicating whether it should used the tuple, page, or bit-sliced signatures. Queries take the form val1,val2,...,valn, where some of the vali can be '?' (without the quotes). Some examples, and their interpretation are given below. You can find more examples in the lecture slides and course notes.
?,?,?    # matches any tuple in the relation
10,?,?   # matches any tuple with 10 as the value of attribute 1
?,abc,?  # matches any tuple with abc as the value of attribute 2
10,abc,? # matches any tuple with 10 and abc as the values of attributes 1 and 2

There are also a number of auxiliary commands to assist with building and examining relations:

4.gendata #tuples #attributes [startID] [seed]
Generates a specified number of n-attribute tuples in the appropriate format to insert into a created relation. All tuples are the same format and look like

UniqID,RandomString,a3-Num,a4-Num,...,an-Num
For example, the following 4-attribute tuples could be generated by a call like   gendata 1000 4

7654321,aTwentyCharLongStrng,a3-013,a4-001
3456789,aTwentyChrLongString,a3-042,a4-128
Of course, the above call to gendata will generate 1000 tuples like these.

A tuple is represented by a sequence of comma-separated fields. The first field is a unique 7-digit number; the second field is a random 20-char string (most likely unique in a given database); the remaining fields have a field identifier followed by a non-unique 3-digit number. The size of each tuple is

7+1 + 20+1 + (n-2)*(6+1)-1  = 28 + 7*(n-2) bytes
The -1 is because the last attribute doesn't have a trailing comma, and (n-2)*(6+1) assumes that it does.

Note that tuples are limited to at most 9 attributes, which means that the maximum tuple size is a modest 77 bytes. (If you wish, you can work with larger tuples by tweaking the gendata and create commands and the newRelation() function, but this not required for the assignment).

5.stats RelName
Prints information about the sizes of various aspects of the relation. Note that some aspects are static (e.g. the size of tuples) and some aspects are dynamic (e.g. the number of tuples). An example of using the stats command is given below.

You can use it to help with debugging, by making sure that the files have been correctly built after the create command, and that the files have been correctly updated after some tuples have been inserted.

6.dump RelName
Writes all tuples from the relation RelName, one per line, to standard output. This is like an inverse of the insert command. Tuples are dumped in a form that could be used by insert to rebuild a database.

You can use it to help with debugging, by making sure that the tuples are inserted correctly into the data file.

tricks:
1. for the code word of the page signature, we need to set ceil(x/2c) bits to 1 where x is the length and c is the number of tuples in the page. So that when the page is full, we'll have roughly half the bits in each code word set to 1.
