#!/bin/bash

path=$1;
find "$path" -name "*" | grep htm > "rezultat.txt"
find "$path" -name "*" | grep php >> "rezultat.txt"

while read line
do
	infoFile=$(file -i "$line");
	temp="$line"".utf";
	if [[ $infoFile == *unknown-8bit* ]]; 
		then
			iconv -f windows-1252 -t utf-8 "$line" > "$temp";
			cp "$temp" "$line";
			rm "$temp";
	fi

	if [[ $infoFile == *windows-1252* ]];
		then 
		     iconv -f windows-1252 -t utf-8 "$line" > "$temp";
		     cp "$temp" "$line";
		     rm "$temp";
	fi

	if [[ $infoFile == *iso-8859-1* ]];
		then 
		     iconv -f iso-8859-1 -t utf-8 "$line" > "$temp";
        	     cp "$temp" "$line";
		     rm "$temp";
	fi

	if [[ $infoFile == *iso-8859-2* ]];
		then 
		     iconv -f iso-8859-2 -t utf-8 "$line" > "$temp";
		     cp "$temp" "$line";
		     rm "$temp";
	fi		
done < "rezultat.txt"
rm "rezultat.txt" 
