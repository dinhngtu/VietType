`vw39kw.txt` is derived from Hồ Ngọc Đức's `Viet39K.txt` Vietnamese word list, distributed under the GNU General Public License:
https://www.informatik.uni-leipzig.de/~duc/software/misc/wordlist.html

`ewdsw.txt` is derived from the SCOWL word list maintained by Kevin Atkinson <kevina@users.sourceforge.net>. See the included license file.

Creation script:

    cat /usr/share/dict/american-english-small /usr/share/dict/british-english-small | \
    tr '[:upper:]' '[:lower:]' | \
    sed '/[^a-z]/d' | \
    sort -u | \
    tr '\n' '\000' | \
    iconv -f utf8 -t utf16le > ewdsw.txt
