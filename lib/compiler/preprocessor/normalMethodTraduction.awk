NR == FNR {
  rep[$1] = $2
  next
} 

{
  for (key in rep)
  {
    pattern="[ 	][ 	]*"key"[ 	]*[(]"
    className=substr(rep[key], 0, match(rep[key], /_/)-1)
    replacement=rep[key]"(("className")"
    gsub(pattern, replacement)
  }
  print
}
