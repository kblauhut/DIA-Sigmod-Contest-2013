// Quick bench Links
https://quick-bench.com/q/D_EjKmBXy2aabR9doqv48NiW_PQ
https://quick-bench.com/q/tz5dYNrURuClKbzpYJXwm4i5u50

// Resources
https://www.mi.fu-berlin.de/wiki/pub/ABI/RnaSeqP4/myers-bitvector-verification.pdf

Give queries q1,q2,q3...
If the word w1 exists in q1 and q2 and q3
and the match type and distance are the same
we can run a new query qx with the same match type and distance
if the query is falsy we can exclude q1,q2 and q3 from our search

Lets handle exact matches only for now:

Idea 1:
Construct a tree of words

When a query is added:
For every word:
If word not in tree: Add a word node to the tree referencing the query
If word in tree: Add query id to word node

When a query is removed:
TODO

When a document is matched:
Take every leaf node of the tree where there are >x query ids
Run the matcher for the given word
If word doesnt match, add all query ids to a set of non matching queries
Exclude these when iterating over all queries

Now for handling all Cases:
If EDIT_DISTANCE <= 3 doesnt match for a document then:
EDIT_DISTANCE <= 2 wont match for that document
HAMMING_DISTANCE <= 3 wont match for that document
EXACT_MATCH wont match for that document

If HAMMING_DISTANCE <= 3 doesnt match for a document then:
EXACT_MATCH wont match for that document

If EXACT_MATCH doesnt match for a document then:

Now the issue is that we can't just always reduce the word query to EDIT_DISTANCE with the highest match distance
Since then we wont be effective in excluding queries.
