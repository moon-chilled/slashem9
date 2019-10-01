CFLAGS="-I../include -Iinclude "
mkdir dat include src
cc ../util/makedefs.c ../util/panic.c ../src/alloc.c ../src/objects.c ../src/monst.c  $CFLAGS -o makedefs
./makedefs
