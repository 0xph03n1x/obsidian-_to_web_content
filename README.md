# obsidian-_to_web_content

TL;DR could've been a bash one-liner, but that wouldln't be fun.

Alternative with bash, which copies any screenshot found inside of a markdown file to the current directory (modify as needed) and change the file name (modify as needed) to anything you want, appending a number to it:

```bash
cat *.md | grep -E "\!\[\[Pasted\ image\ [0-9]{14}.png\]\]" | awk -F "\[" '{print $3}' | rev | cut -c3- | rev | while read i; do find / -name $i -exec cp -t . {} + 2>/dev/null; done;ls -v | grep "png" | cat -n | while read n f; do mv -n "$f" "file_name$n.png"; done
```

TO DO:

1. Add functionality to replace the markdown link to the image in the source file
2. Add check for existing file, if exists -> skip copy
3. Less arguments?
