{
  if ($5 > $4) {
    len = split($2, splitname, " ")
    name = splitname[len]
    printf("sed -Ee 's/EMAIL/%s/g' ", $1)
    printf("-Ee 's/FULLNAME/%s/g' ", $2)
    printf("-Ee 's/TITLE/%s/g' ", $3)
    printf("-Ee '/[^L]NAME/ s/NAME/%s/g' ", name)
    printf("-Ee 's/AMOUNT/%d/g' ", $5)
    printf("-Ee 's,DATE,%s,g' template.txt > Emails/%s\n", date, $1)
  }
}