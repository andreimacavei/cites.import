#!/bin/sh

for f in $(find /home/andrei/Desktop/cites.production.arhive3/cites.production/ -name '*.php' -or -name '*.html' -or -name "*.shtml" -or -name ".phtml")
do
	rez=$(file -bi "$f")
	a="$f".".utf8"
	if echo $rez | grep 'unknown'
		then
  		iconv -t utf-8 -f windows-1252 "$f" > "$a"
		cp "$a" "$f"
		rm "$a"	
	fi

	if echo $rez | grep 'iso-8859-1'
		then
		iconv -t utf-8 -f iso-8859-1 "$f" > "$a"
		cp "$a" "$f"
		rm "$a"
	fi

	if echo $rez | grep 'windows-1252'
		then
		iconv -t utf-8 -f windows-1252 "$f" > "$a"
		cp "$a" "$f"
		rm "$a"
	fi
done

