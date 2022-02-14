copy c:\util\grep.exe e:\
copy d:\unix\usr\bin\ps*.ovl e:\
for var in *.c *.h *.asm *.inc
do
if e:\grep -s -e 'oma2000' -e 'OMA2000' $var; then
echo ${var%.*}
sed -e 's/oma2000/oma4000/' -e 's/OMA2000/OMA4000/' $var >${var%.*}.'$$$'
del $var
rename ${var%.*}.\$\$\$ $var
fi
done
