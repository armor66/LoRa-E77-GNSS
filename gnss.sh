#!/bin/bash

if [ -z "$1" ]; then
	read -e -p "Enter filename, use tab for completion: " file
else
	file="$1"
fi

if [ -z "$file" ]; then
	sed -i "s/^.*TREKPOINTS_TOTAL.*$/#define TREKPOINTS_TOTAL (1)/" gnss.h
	sed -i "s/^.*latitude_array.*$/static const float latitude_array[]={0}/" gnss.h
	sed -i "s/^.*longitude_array.*$/static const float longitude_array[]={0}/" gnss.h
	echo "No gnss file supplied"
	echo "No trackpoints to follow"
	exit 0
fi

#[ -f "$file" ] || exit 1

#сохранить содержимое в "" после lat= и lon=
sed -n 's/.*lat="\([^"]*\)".*/\1/gp' $file > lat.h
sed -n 's/.*lon="\([^"]*\)".*/\1/gp' $file > lon.h

#убрать точки
#sed -i 's/[.]//g' lat.h
#sed -i 's/[.]//g' lon.h

#to remove lines use the n~m (n skip m) address notation
#sed '1~3d' filename
#which deletes every third line, starting at the first

#remove the even numbered lines from the file.
sed -i '0~2d' lat.h
sed -i '0~2d' lon.h
sed -i '0~2d' lat.h
sed -i '0~2d' lon.h
sed -i '0~2d' lat.h
sed -i '0~2d' lon.h
sed -i '0~2d' lat.h
sed -i '0~2d' lon.h
#sed -i '1~3d' lat.h
#sed -i '1~3d' lon.h
sed -n '$=' lat.h > num.h

#deg_to_rad = 0.017453292519943
#awk -i inplace '{$1=$1/1000000000} {printf "%.7f\n", $1}' lat.h
awk -i inplace '{$1=$1*0.017453292519943} {printf "%.15f\n", $1}' lat.h
#awk -i inplace '{$1=$1/1000000000} {printf "%.7f\n", $1}' lon.h
awk -i inplace '{$1=$1*0.017453292519943} {printf "%.15f\n", $1}' lon.h

#awk -v c=1000 '{ print $0/c }' lat.h > lat1.h
#awk -v c=1000 '{ print $0/c }' lon.h > lon1.h

#заменить перевод строк на ", "
sed -ni 'H;${g;s/\n/,/gp}' lat.h
sed -ni 'H;${g;s/\n/,/gp}' lon.h

#вставить названия и заключить координаты в фигурные скобки:
sed -i 's/^/\#define TREKPOINTS_TOTAL \(/;s/,//;s/$/\)\n/' num.h
sed -i 's/^/static const float latitude_array []=\{/;s/,//;s/$/\};\n/' lat.h
sed -i 's/^/static const float longitude_array[]=\{/;s/,//;s/$/\};\n/' lon.h

#заменить содежимое {} в gnss.h:
#static const int32_t latitude_array []={}
#static const int32_t longitude_array[]={}

sed -i "s/^.*TREKPOINTS_TOTAL.*$/$(cat num.h)/" gnss.h
sed -i "s/^.*latitude_array.*$/$(cat lat.h)/" gnss.h
sed -i "s/^.*longitude_array.*$/$(cat lon.h)/" gnss.h

rm lat.h lon.h num.h
