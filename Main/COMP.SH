for i in $*
do
  diff -b $i.bak $i.c
done
