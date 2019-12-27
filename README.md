# OS Assignment
1. Read-Write lock in AVL tree (iterative &amp; recursive)
    g++ avlIterative.cpp -o avli -std=c++11
    ./avli

    g++ avlRecursive.cpp -o avlr -std=c++11
    ./avlr

2. Read-Write lock with fair odering (random selection from queue)
    g++ readWrite.cpp -o rw -std=c++11
    - prints list of reader and writers in operation 

3. File/Directory finder
    g++ find.cpp -o find -std=c++11
    ./find <file_name/directory_name>

    - prints all the files/directories of the specified name present anywhere under the current directory and specifies if its a file or a directory
