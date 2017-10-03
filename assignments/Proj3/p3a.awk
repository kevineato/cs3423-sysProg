{
  for (i = NF; i > 0; i--) {
    if (i == 1)
      print $i
    else
      printf("%s ", $i)
  }
}