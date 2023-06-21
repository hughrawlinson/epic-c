# Epic-C

I decided as an exercise to reimplement Matt Grey's epic viewer in C, to learn
some C. That program is in this repo.

Aside: Here it is in one line of bash

```bash
curl https://epic.gsfc.nasa.gov/api/natural | jq -r .[].image | sed -E 's/epic_1b
_(.{4})(.{2})(.{2}).+/https:\/\/epic.gsfc.nasa.gov\/archive\/natural\/\1\/\2\/\3\/png\/\0.png/' | xargs feh -x --slideshow-delay 1
```
