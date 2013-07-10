#!/bin/bash

path=$1;


for file in $(find "$path" -name '*.php' -or -name '*.html' -or -name "*.shtml" -or -name "*.phtml");
do
	infoFile=$(file -i "$file");
	temp="$file"".utf";
	if [[ "$infoFile" == *unknown-8bit* ]]; 
		then
			iconv -f windows-1252 -t utf-8 "$file" > "$temp";
			cp "$temp" "$file";
			rm "$temp";
	fi

	if [[ "$infoFile" == *windows-1252* ]];
		then 
		     iconv -f windows-1252 -t utf-8 "$file" > "$temp";
		     cp "$temp" "$file";
		     rm "$temp";
	fi

	if [[ "$infoFile" == *iso-8859-1* ]];
		then 
		     iconv -f iso-8859-1 -t utf-8 "$file" > "$temp";
	             cp "$temp" "$file";
		     rm "$temp";
	fi

	if [[ "$infoFile" == *iso-8859-2* ]];
		then 
		     iconv -f iso-8859-2 -t utf-8 "$file" > "$temp";
		     cp "$temp" "$file";
		     rm "$temp";
	fi

done
