NR == FNR {
  rep[$1] = $2
  next
} 

{
  for (key in rep)
  {
    pattern="[ 	][ 	]*"key"[ 	]*[(]"
    replacement=" "rep[key]
    gsub(pattern, replacement)
  }
  print
}
