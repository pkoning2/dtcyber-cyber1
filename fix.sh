#!/bin/bash
cr=${0%/*}
cj=$cr/j
test -e $cj/fix && rm -f $cj/fix
for f in $@; do
  f2=${f##*/}
  echo -e "fix.\nuser,plato,plato.\nattach,foo=$f2/m=w,na.\nif(.not.file(foo,as))define,foo=$f2.\ncopycf,input,foo.\n~" >> $cj/fix
  cat $f >> $cj/fix
  echo "}" >> $cj/fix
done
